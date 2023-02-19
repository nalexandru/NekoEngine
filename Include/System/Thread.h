#ifndef _NE_SYSTEM_THREAD_H_
#define _NE_SYSTEM_THREAD_H_

#include <Engine/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#	define THREAD_LOCAL	__declspec(thread)
#elif defined(__GNUC__)
#	define THREAD_LOCAL	__thread
#endif

uint32_t Sys_TlsAlloc(void);
void *Sys_TlsGet(uint32_t key);
void Sys_TlsSet(uint32_t key, void *data);
void Sys_TlsFree(uint32_t key);

void Sys_Yield(void);

bool Sys_InitThread(NeThread *t, const char *name, void (*proc)(void *), void *args);
void Sys_SetThreadAffinity(NeThread t, int cpu);
void Sys_JoinThread(NeThread t);
void Sys_JoinThreads(NeThread *threads, int count);
NeThread Sys_CurrentThread(void);
void Sys_TermThread(NeThread t);

bool Sys_InitMutex(NeMutex *mtx);
bool Sys_LockMutex(NeMutex mtx);
bool Sys_UnlockMutex(NeMutex mtx);
void Sys_TermMutex(NeMutex mtx);

bool Sys_InitFutex(NeFutex *ftx);
bool Sys_LockFutex(NeFutex ftx);
bool Sys_UnlockFutex(NeFutex ftx);
void Sys_TermFutex(NeFutex ftx);

bool Sys_InitConditionVariable(NeConditionVariable *cv);
void Sys_Signal(NeConditionVariable cv);
void Sys_Broadcast(NeConditionVariable cv);
bool Sys_WaitMutex(NeConditionVariable cv, NeMutex mtx);
bool Sys_WaitFutex(NeConditionVariable cv, NeFutex ftx);
void Sys_TermConditionVariable(NeConditionVariable cv);

#ifdef __cplusplus
}
#endif

#endif /* _NE_SYSTEM_THREAD_H_ */

/* NekoEngine
 *
 * Thread.h
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
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
