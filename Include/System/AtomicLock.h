#ifndef _SYS_ATOMIC_LOCK_H_
#define _SYS_ATOMIC_LOCK_H_

#include <stdatomic.h>

#include <Engine/Types.h>

struct AtomicLock
{
	ALIGN(16) _Atomic int32_t read, write;
};

void Sys_InitAtomicLock(struct AtomicLock *lock);

void Sys_AtomicLockRead(struct AtomicLock *lock);
void Sys_AtomicUnlockRead(struct AtomicLock *lock);

void Sys_AtomicLockWrite(struct AtomicLock *lock);
void Sys_AtomicUnlockWrite(struct AtomicLock *lock);

#endif /* _SYS_ATOMIC_LOCK_H_ */
