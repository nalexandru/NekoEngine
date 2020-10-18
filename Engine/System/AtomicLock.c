#include <System/System.h>
#include <System/Thread.h>
#include <System/AtomicLock.h>

void
Sys_InitAtomicLock(struct AtomicLock *lock)
{
	lock->read = lock->write = 0;
}

void
Sys_AtomicLockRead(struct AtomicLock *lock)
{
	while (lock->write != 0)
		Sys_Yield();

	Sys_AtomicIncrement(&lock->read);
}

void
Sys_AtomicUnlockRead(struct AtomicLock *lock)
{
	Sys_AtomicDecrement(&lock->read);
}

void
Sys_AtomicLockWrite(struct AtomicLock *lock)
{
	while (Sys_AtomicCompareAndSwap(&lock->write, 1, 0))
		;

	while (lock->read != 0)
		Sys_Yield();
}

void
Sys_AtomicUnlockWrite(struct AtomicLock *lock)
{
	Sys_AtomicDecrement(&lock->write);
}

