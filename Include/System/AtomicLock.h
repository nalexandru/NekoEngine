#ifndef _NE_SYSTEM_ATOMIC_LOCK_H_
#define _NE_SYSTEM_ATOMIC_LOCK_H_

#include <stdatomic.h>

#include <Engine/Types.h>

struct NeAtomicLock
{
	NE_ALIGN(16) volatile _Atomic int32_t read, write;
};

void Sys_InitAtomicLock(struct NeAtomicLock *lock);

void Sys_AtomicLockRead(volatile struct NeAtomicLock *lock);
void Sys_AtomicUnlockRead(volatile struct NeAtomicLock *lock);

void Sys_AtomicLockWrite(volatile struct NeAtomicLock *lock);
void Sys_AtomicUnlockWrite(volatile struct NeAtomicLock *lock);

#endif /* _NE_SYSTEM_ATOMIC_LOCK_H_ */
