#include <assert.h>
#include <stdlib.h>

#include <sched.h>

#include <System/AtomicLock.h>

#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
#include <stdatomic.h>

struct AtomicLock
{
	atomic_int read, write;
};

struct AtomicLock *
Sys_InitAtomicLock(void)
{
	struct AtomicLock *al = malloc(sizeof(*al));
	assert(al);

	atomic_init(&al->read, 0);
	atomic_init(&al->write, 0);

	return al;
}

void
Sys_AtomicLockRead(struct AtomicLock *lock)
{
	while (lock->write != 0)
		sched_yield();

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
		sched_yield();
}

void
Sys_AtomicUnlockWrite(struct AtomicLock *lock)
{
	atomic_fetch_sub(&lock->write, 1);
}

void
Sys_TermAtomicLock(struct AtomicLock *lock)
{
	free(lock);
}
#elif
struct AtomicLock
{
	int32_t read, write;
};

struct AtomicLock *
Sys_InitAtomicLock(void)
{
	struct AtomicLock *al = malloc(sizeof(*al), 16);
	assert(al);

	al->read = 0;
	al->write = 0;

	return al;
}

void
Sys_AtomicLockRead(struct AtomicLock *lock)
{
	while (lock->write != 0)
		sched_yield();

	__sync_add_and_fetch(&lock->read, 1);
}

void
Sys_AtomicUnlockRead(struct AtomicLock *lock)
{
	__sync_sub_and_fetch(&lock->read, 1);
}

void
Sys_AtomicLockWrite(struct AtomicLock *lock)
{
	while (__sync_val_compare_and_swap(&lock->write, 0, 1))
		;

	while (lock->read != 0)
		sched_yield();
}

void
Sys_AtomicUnlockWrite(struct AtomicLock *lock)
{
	__sync_sub_and_fetch(&lock->write, 1);
}

void
Sys_TermAtomicLock(struct AtomicLock *lock)
{
	free(lock);
}
#endif

