# Solution
Let's examine what we have in this challenge 👀<br>
We have all the usual stuff of a driver and the really intersting part is the ```StbDeviceControl``` function.<br>
In this function we handle the IOCTL that contains the data from which the driver should take the values in order to change the priority of the thread.
The expected data is the following struct as you can see in the code:
```cpp
struct ThreadData {
    ULONG ThreadId;
	int Priority;
}
```

```ThreadId``` is the thread id of the system process that we want to change and the ```Priority``` is the priority value, pretty much self explantory.<br>
So, why do we get a BSOD?<br>
well, let's look on the following code:
```cpp
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
```
We start by changing our context from the requesting process context to the system process context by calling ```KeStackAttachProcess```.
Than, we lookup the thread id with the value we got from the client/user in order to get the ETHREAD object.
Once we have the ETHREAD we can change the priority by calling ```KeSetPriorityThread``` with the wanted priority.
Lastly, we deref the ETHREAD and deattch from the system process.

As you probably noticed, the faulting line is:<br>
```cpp
status = PsLookupThreadByThreadId(ULongToHandle(data->ThreadId), &Thread);
```
If we pass a thread id that doesn't exists, the code shouldn't crash so why does it crash?
Well, the answer lies in the memory address of ```data->ThreadId```.
If you look at the IOCTL we are sending:
```cpp
#define IOCTL_PRIORITY_BOOSTER_SET_PRIORITY CTL_CODE(0x8000, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)
```
You can see we are using ```METHOD_NEITHER``` which means that the data being passed resides in the calling process usermode virtual memory address space and not being copied to the kernel address space.
So when we call ```KeStackAttachProcess``` to switch context to the system process we are changing ```cr3``` to point to the system process ```PML4```.
This leads to a page fault because the MMU is trying to access a virtual address that only exists in the calling process address space.
And therefor we get a bsod when trying to deref the data struct.

So the fix to this challenge is to replace the method we use to be ```METHOD_BUFFERED``` instead of ```METHOD_NEITHER``` and access the data with ```Irp->AssociatedIrp.SystemBuffer```.
