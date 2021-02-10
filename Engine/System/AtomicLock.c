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

	atomic_fetch_add(&lock->read, 1);
}

void
Sys_AtomicUnlockRead(struct AtomicLock *lock)
{
	atomic_fetch_sub(&lock->read, 1);
}

void
Sys_AtomicLockWrite(struct AtomicLock *lock)
{
	int expected = 0;
	while (atomic_compare_exchange_strong(&lock->write, &expected, 1))
		;
	
	while (lock->read != 0)
		Sys_Yield();
}

void
Sys_AtomicUnlockWrite(struct AtomicLock *lock)
{
	atomic_fetch_sub(&lock->write, 1);
}

