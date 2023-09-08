#include <ntifs.h>

#define DRIVER_TAG 'STB'

void StbUnload(PDRIVER_OBJECT DriverObject);
NTSTATUS StbCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);
typedef struct SuperSecretNumber {
	int number;
} SuperSecretData;
NTSTATUS WriteSecretData(SuperSecretNumber* data);

extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	DriverObject->DriverUnload = StbUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = StbCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = StbCreateClose;
	
	auto data = reinterpret_cast<SuperSecretNumber*>(ExAllocatePoolWithTag(PagedPool, sizeof(SuperSecretNumber), DRIVER_TAG));

	if (data == nullptr) {
		return STATUS_INSUFFICIENT_RESOURCES;
	}

    return WriteSecretData(data);
}

NTSTATUS WriteSecretData(SuperSecretNumber* data) {
	KSPIN_LOCK spinLock;
	KIRQL oldIrql;
	
	KeAcquireSpinLock(&spinLock, &oldIrql);
	
	data->number = 10;

	KeReleaseSpinLock(&spinLock, oldIrql);

	DbgPrint("The secret number is: %d\n", data->number);

	return STATUS_SUCCESS;
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
	UNREFERENCED_PARAMETER(DriverObject);
}
