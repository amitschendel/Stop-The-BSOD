# Solution
So this driver is very simple, we just allocate a struct and write data to one of the members.<br>
However, inside the ```WriteSecretData``` function we acquire a spin lock before writing any data.<br>
This is a common thing, in kernel code, you want to acquire locks on shared data structures before modifying them to avoid race conditions.<br>
One such lock is a spin lock.

# Spin Lock - Inside
A spin lock is a synchronization mechanism used in multi-threaded and multi-processor systems to protect critical sections of code from concurrent access by multiple threads or processors. Unlike traditional locks or mutexes, which may put a thread to sleep when it attempts to acquire a locked resource, a spin lock simply "spins" in a tight loop until the lock becomes available. This means that the thread holding the lock releases it relatively quickly, as spin locks are designed for use in scenarios where the critical section is expected to be held for a very short duration. Spin locks are lightweight and can be more efficient than other synchronization primitives in situations where contention is rare and the critical section is short.

Here's a simple assembly code snippet demonstrating the use of a spin lock on a hypothetical Windows-like system. Note that this is a simplified example for illustrative purposes:
```asm
section .data
    ; Define the spin lock variable as 0 (unlocked)
    spinLock db 0

section .text
global acquireSpinLock
global releaseSpinLock

acquireSpinLock:
    mov eax, 1           ; Set EAX to 1 (lock value)
    xchg [spinLock], al  ; Atomically exchange spinLock with AL (EAX's low byte)
    test al, al          ; Test if AL is zero (lock acquired)
    jnz acquireSpinLock  ; If not, keep spinning
    ret

releaseSpinLock:
    mov byte [spinLock], 0  ; Release the lock by setting spinLock to 0
    ret
```
In this hypothetical assembly code:

1. The spinLock variable is represented as a byte (8 bits), where 0 indicates an unlocked state, and 1 indicates a locked state.
2. The acquireSpinLock function repeatedly tries to acquire the spin lock using an atomic exchange operation (xchg) until the lock is acquired (i.e., spinLock becomes 0). It does this by setting EAX to 1 (lock value) and attempting to exchange it with the spinLock variable. If unsuccessful (i.e., spinLock was already 1), it keeps spinning until the lock becomes available.
3. The releaseSpinLock function simply sets spinLock back to 0 to release the lock.

In Windows, when you acquire a spin lock, the function you call is ```KeAcquireSpinLockRaiseToDpc``` which raises the IRQL to DPC=2.<br>
This means that if you try to access a page of memory that is in the page file and this is possible because you have allocated the pool as a ```PagedPool``` then you will get a BSOD.

# Stopping the BSOD

If we return to our driver code, this is exactly the bug, we allocate the struct as a ```PagedPool```- 
```cpp
auto data = reinterpret_cast<SuperSecretNumber*>(ExAllocatePoolWithTag(PagedPool, sizeof(SuperSecretNumber), DRIVER_TAG));
```

Therefore, a BSOD is likely to occur here, especially in systems with low memory capacity.

The fix is to allocate the struct in a ```NonPagedPool``` or use a different locking mechanism.<br>
[Here](https://www.osr.com/nt-insider/2015-issue3/the-state-of-synchronization/) is a reference for a great article about it.