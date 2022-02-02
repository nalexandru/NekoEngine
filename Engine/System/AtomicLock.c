#include <System/System.h>
#include <System/Thread.h>
#include <System/AtomicLock.h>

void
Sys_InitAtomicLock(struct NeAtomicLock *lock)
{
	lock->read = lock->write = 0;
}

void
Sys_AtomicLockRead(volatile struct NeAtomicLock *lock)
{
	while (lock->write != 0)
		Sys_Yield();

	atomic_fetch_add_explicit(&lock->read, 1, memory_order_acquire);
}

void
Sys_AtomicUnlockRead(volatile struct NeAtomicLock *lock)
{
	atomic_fetch_sub_explicit(&lock->read, 1, memory_order_release);
}

void
Sys_AtomicLockWrite(volatile struct NeAtomicLock *lock)
{
	int expected = 0;
	while (atomic_compare_exchange_strong_explicit(&lock->write, &expected, 1,
			memory_order_acquire, memory_order_acquire))
		;
	
	while (lock->read != 0)
		Sys_Yield();
}

void
Sys_AtomicUnlockWrite(volatile struct NeAtomicLock *lock)
{
	atomic_store_explicit(&lock->write, 0, memory_order_release);
}
