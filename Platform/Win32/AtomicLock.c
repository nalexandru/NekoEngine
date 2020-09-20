#include <assert.h>
#include <Windows.h>
#include <System/AtomicLock.h>

struct AtomicLock
{
	volatile SHORT read, write;
};

struct AtomicLock *
Sys_InitAtomicLock(void)
{
	struct AtomicLock *al = _aligned_malloc(sizeof(*al), 16);
	assert(al);

	al->read = 0;
	al->write = 0;

	return al;
}

void
Sys_AtomicLockRead(struct AtomicLock *lock)
{
	while (lock->write != 0)
		SwitchToThread();

	InterlockedIncrement16(&lock->read);
}

void
Sys_AtomicUnlockRead(struct AtomicLock *lock)
{
	InterlockedDecrement16(&lock->read);
}

void
Sys_AtomicLockWrite(struct AtomicLock *lock)
{
	while (InterlockedCompareExchange16(&lock->write, 1, 0))
		;

	while (lock->read != 0)
		SwitchToThread();
}

void
Sys_AtomicUnlockWrite(struct AtomicLock *lock)
{
	InterlockedDecrement16(&lock->write);
}

void
Sys_TermAtomicLock(struct AtomicLock *lock)
{
	_aligned_free(lock);
}
