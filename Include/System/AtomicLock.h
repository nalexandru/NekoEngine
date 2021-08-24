#ifndef _NE_SYSTEM_ATOMIC_LOCK_H_
#define _NE_SYSTEM_ATOMIC_LOCK_H_

#include <stdatomic.h>

#include <Engine/Types.h>

struct AtomicLock
{
	ALIGN(16) volatile _Atomic int32_t read, write;
};

void Sys_InitAtomicLock(struct AtomicLock *lock);

void Sys_AtomicLockRead(volatile struct AtomicLock *lock);
void Sys_AtomicUnlockRead(volatile struct AtomicLock *lock);

void Sys_AtomicLockWrite(volatile struct AtomicLock *lock);
void Sys_AtomicUnlockWrite(volatile struct AtomicLock *lock);

#endif /* _NE_SYSTEM_ATOMIC_LOCK_H_ */
