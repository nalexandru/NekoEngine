#ifndef NE_SYSTEM_ATOMIC_LOCK_H
#define NE_SYSTEM_ATOMIC_LOCK_H

#include <Engine/Types.h>

#include NE_ATOMIC_HDR

struct NeAtomicLock
{
	NE_ALIGN(16) NE_ATOMIC_INT read, write;
};

#ifdef __cplusplus
extern "C" {
#endif

void Sys_InitAtomicLock(struct NeAtomicLock *lock);

void Sys_AtomicLockRead(volatile struct NeAtomicLock *lock);
void Sys_AtomicUnlockRead(volatile struct NeAtomicLock *lock);

void Sys_AtomicLockWrite(volatile struct NeAtomicLock *lock);
void Sys_AtomicUnlockWrite(volatile struct NeAtomicLock *lock);

#ifdef __cplusplus
}
#endif

#endif /* NE_SYSTEM_ATOMIC_LOCK_H */

/* NekoEngine
 *
 * AtomicLock.h
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
