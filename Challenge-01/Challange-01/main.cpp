#include <ntifs.h>
#include "Common.h"


void StbUnload(PDRIVER_OBJECT DriverObject);
NTSTATUS StbCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS StbDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);


extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{

	UNREFERENCED_PARAMETER(RegistryPath);
	DriverObject->DriverUnload = StbUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = StbCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = StbCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = StbDeviceControl;

	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\StopTheBsod");
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\StopTheBsod");

	PDEVICE_OBJECT DeviceObject;
	NTSTATUS status = IoCreateDevice(
		DriverObject,
		0,
		&devName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&DeviceObject
	);
	if (!NT_SUCCESS(status)) {
		DbgPrint("Failed to create device object (0x%08X)\n", status);
		return status;
	}

	status = IoCreateSymbolicLink(&symLink, &devName);
	if (!NT_SUCCESS(status)) {
		DbgPrint("Failed to create symbolic link (0x%08X)\n", status);
		IoDeleteDevice(DeviceObject);
		return status;
	}

	return STATUS_SUCCESS;
}

NTSTATUS StbDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	auto status = STATUS_SUCCESS;

	switch (stack->Parameters.DeviceIoControl.IoControlCode) {
	case IOCTL_PRIORITY_BOOSTER_SET_PRIORITY: {
		auto data = reinterpret_cast<ThreadData*>(stack->Parameters.DeviceIoControl.Type3InputBuffer);
		if (data == nullptr) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}
		if (data->Priority < 1 || data->Priority > 31) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		KAPC_STATE apcState;
		KeStackAttachProcess(PsInitialSystemProcess, &apcState);

		PETHREAD Thread;
		status = PsLookupThreadByThreadId(ULongToHandle(data->ThreadId), &Thread);
		if (!NT_SUCCESS(status))
			break;
		KeSetPriorityThread(reinterpret_cast<PKTHREAD>(Thread), data->Priority);
		ObDereferenceObject(Thread);
		DbgPrint("Thread Priority change for %d to %d succeeded!\n", data->ThreadId, data->Priority);
		KeUnstackDetachProcess(&apcState);
		break;
	}
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;

}

NTSTATUS StbCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

void StbUnload(PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\StopTheBsod");
	// Delete symbolic link
	IoDeleteSymbolicLink(&symLink);
	// Delete device object
	IoDeleteDevice(DriverObject->DeviceObject);
}
