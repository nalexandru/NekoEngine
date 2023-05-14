#include "Win32Platform.h"

#include <System/Thread.h>
#include <System/Memory.h>

// Thread naming info:
// https://docs.microsoft.com/en-us/visualstudio/debugger/how-to-set-a-thread-name-in-native-code?view=vs-2015&redirectedfrom=MSDN
const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

HRESULT (WINAPI *k32_SetThreadDescription)(HANDLE, PCWSTR) = NULL;

uint32_t
Sys_TlsAlloc(void)
{
	return TlsAlloc();
}

void *
Sys_TlsGet(uint32_t key)
{
	return TlsGetValue(key);
}

void
Sys_TlsSet(uint32_t key, void *data)
{
	TlsSetValue(key, data);
}

void
Sys_TlsFree(uint32_t key)
{
	TlsFree(key);
}

void
Sys_Yield(void)
{
	NtYieldExecution();
}

bool
Sys_InitThread(NeThread *t, const char *name, void (*proc)(void *), void *args)
{
	THREADNAME_INFO info = { 0x1000, name, 0, 0 };
	HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)proc, args, 0, &info.dwThreadID);
	if (thread == INVALID_HANDLE_VALUE)
		return false;

	if (k32_SetThreadDescription)
		k32_SetThreadDescription(thread, NeWin32_UTF8toUCS2(name));

#ifdef MSC_VER
	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (const ULONG_PTR *)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
	}
#endif

	*t = thread;

	return true;
}

void
Sys_SetThreadAffinity(NeThread t, int cpu)
{
	SetThreadAffinityMask((HANDLE)t, (DWORD_PTR)1 << ((DWORD_PTR)cpu));
}

void
Sys_DetachThread(NeThread t)
{
	CloseHandle((HANDLE)t);
}

void
Sys_JoinThread(NeThread t)
{
	LARGE_INTEGER li = { .QuadPart = -10000LL * INFINITE };
	NtWaitForSingleObject((HANDLE)t, FALSE, &li);
}

void
Sys_JoinThreads(NeThread *threads, int count)
{
	LARGE_INTEGER li = { .QuadPart = -10000LL * INFINITE };
	NtWaitForMultipleObjects(count, (PHANDLE)threads, WaitAllObject, FALSE, &li);
}

NeThread
Sys_CurrentThread(void)
{
	return (NeThread)(uint64_t)GetCurrentThreadId();
}

void Sys_TermThread(NeThread t)
{
	TerminateThread((HANDLE)t, 0);
}

// Mutex

bool
Sys_InitMutex(NeMutex *mtx)
{
	CRITICAL_SECTION *cs = HeapAlloc(GetProcessHeap(), 0, sizeof(*cs));
	if (!cs)
		return false;

	InitializeCriticalSection(cs);

	*mtx = cs;

	return true;
}

bool
Sys_LockMutex(NeMutex mtx)
{
	EnterCriticalSection((LPCRITICAL_SECTION)mtx);
	return true;
}

bool
Sys_UnlockMutex(NeMutex mtx)
{
	LeaveCriticalSection((LPCRITICAL_SECTION)mtx);
	return true;
}

void
Sys_TermMutex(NeMutex mtx)
{
	DeleteCriticalSection((LPCRITICAL_SECTION)mtx);
	HeapFree(GetProcessHeap(), 0, mtx);
}

// Futex

bool
Sys_InitFutex(NeFutex *ftx)
{
#ifdef NE_NT5_SUPPORT
	if (!Win32_InitializeSRWLock)
		return Sys_InitMutex((NeMutex *)ftx);
#endif

	SRWLOCK *srw = HeapAlloc(GetProcessHeap(), 0, sizeof(*srw));
	if (!srw)
		return false;

	Win32_InitializeSRWLock(srw);

	*ftx = srw;

	return true;
}

bool
Sys_LockFutex(NeFutex ftx)
{
#ifdef NE_NT5_SUPPORT
	if (!Win32_AcquireSRWLockExclusive)
		return Sys_LockMutex((NeMutex)ftx);
#endif

	Win32_AcquireSRWLockExclusive((PSRWLOCK)ftx);
	return true;
}

bool
Sys_UnlockFutex(NeFutex ftx)
{
#ifdef NE_NT5_SUPPORT
	if (!Win32_ReleaseSRWLockExclusive)
		return Sys_UnlockMutex((NeMutex)ftx);
#endif

	Win32_ReleaseSRWLockExclusive((PSRWLOCK)ftx);
	return true;
}

void
Sys_TermFutex(NeFutex ftx)
{
#ifdef NE_NT5_SUPPORT
	if (!Win32_InitializeSRWLock) {
		Sys_TermMutex((NeMutex)ftx);
		return;
	}
#endif

	HeapFree(GetProcessHeap(), 0, ftx);
}

// Condition variable

bool
Sys_InitConditionVariable(NeConditionVariable *cv)
{
#ifdef NE_NT5_SUPPORT
	if (!Win32_InitializeConditionVariable) {
		struct Win32c_ConditionVariable *v = HeapAlloc(GetProcessHeap(), 0, sizeof(*v));

		Win32c_InitializeConditionVariable((PCONDITION_VARIABLE)v);

		*cv = v;
		return true;
	}
#endif

	CONDITION_VARIABLE *v = HeapAlloc(GetProcessHeap(), 0, sizeof(*v));
	if (!v)
		return false;

	Win32_InitializeConditionVariable(v);

	*cv = v;
	return true;
}

void
Sys_Signal(NeConditionVariable cv)
{
	Win32_WakeConditionVariable((PCONDITION_VARIABLE)cv);
}

void
Sys_Broadcast(NeConditionVariable cv)
{
	Win32_WakeAllConditionVariable((PCONDITION_VARIABLE)cv);
}

bool
Sys_WaitMutex(NeConditionVariable cv, NeMutex mtx)
{
	return Win32_SleepConditionVariableCS((PCONDITION_VARIABLE)cv, (PCRITICAL_SECTION)mtx, INFINITE);
}

bool
Sys_WaitFutex(NeConditionVariable cv, NeFutex ftx)
{
	return Win32_SleepConditionVariableSRW((PCONDITION_VARIABLE)cv, (PSRWLOCK)ftx, INFINITE, 0);
}

void
Sys_TermConditionVariable(NeConditionVariable cv)
{
#ifdef NE_NT5_SUPPORT
	if (!Win32_InitializeConditionVariable)
		Win32c_DeleteConditionVariable((PCONDITION_VARIABLE)cv);
#endif

	HeapFree(GetProcessHeap(), 0, cv);
}

#ifdef NE_NT5_SUPPORT

#define WIN32_CV_EVT_SIGNAL		0
#define WIN32_CV_EVT_BCAST		1

static BOOL WINAPI Win32c_SleepCS(PCONDITION_VARIABLE cv, PCRITICAL_SECTION cs, DWORD ms);

void WINAPI
Win32c_InitializeConditionVariable(PCONDITION_VARIABLE cv)
{
	struct Win32c_ConditionVariable *v = (struct Win32c_ConditionVariable *)cv;
	if (!cv)
		return;

	v->evt[WIN32_CV_EVT_SIGNAL] = CreateEvent(NULL, FALSE, FALSE, NULL);
	v->evt[WIN32_CV_EVT_BCAST] = CreateEvent(NULL, TRUE, FALSE, NULL);

	InitializeCriticalSection(&v->lock);
}

BOOL WINAPI
Win32c_SleepConditionVariableCS(PCONDITION_VARIABLE cv, PCRITICAL_SECTION cs, DWORD ms)
{
	return Win32c_SleepCS(cv, cs, ms);
}

BOOL WINAPI
Win32c_SleepConditionVariableSRW(PCONDITION_VARIABLE cv, PSRWLOCK srw, DWORD ms, ULONG flags)
{
	return Win32c_SleepCS(cv, (PCRITICAL_SECTION)srw, ms);
}

void WINAPI
Win32c_WakeConditionVariable(PCONDITION_VARIABLE cv)
{
	struct Win32c_ConditionVariable *v = (struct Win32c_ConditionVariable *)cv;
	if (!cv)
		return;

	EnterCriticalSection(&v->lock);
	int set = v->wait > 0;
	LeaveCriticalSection(&v->lock);

	if (set)
		SetEvent(v->evt[WIN32_CV_EVT_SIGNAL]);
}

void WINAPI
Win32c_WakeAllConditionVariable(PCONDITION_VARIABLE cv)
{
	struct Win32c_ConditionVariable *v = (struct Win32c_ConditionVariable *)cv;
	if (!cv)
		return;

	EnterCriticalSection(&v->lock);
	int set = v->wait > 0;
	LeaveCriticalSection(&v->lock);

	if (set)
		SetEvent(v->evt[WIN32_CV_EVT_BCAST]);
}

void WINAPI
Win32c_DeleteConditionVariable(PCONDITION_VARIABLE cv)
{
	struct Win32c_ConditionVariable *v = (struct Win32c_ConditionVariable *)cv;
	if (!cv)
		return;

	CloseHandle(v->evt[WIN32_CV_EVT_SIGNAL]);
	CloseHandle(v->evt[WIN32_CV_EVT_BCAST]);

	DeleteCriticalSection(&v->lock);
}

BOOL WINAPI
Win32c_SleepCS(PCONDITION_VARIABLE cv, PCRITICAL_SECTION cs, DWORD ms)
{
	struct Win32c_ConditionVariable *v = (struct Win32c_ConditionVariable *)cv;
	if (!cv || !cs)
		return false;

	EnterCriticalSection(&v->lock);
	++v->wait;
	LeaveCriticalSection(&v->lock);

	LeaveCriticalSection(cs);

	const DWORD rc = WaitForMultipleObjects(2, v->evt, FALSE, ms);
	const bool success = (rc != WAIT_TIMEOUT && rc != WAIT_FAILED);

	EnterCriticalSection(&v->lock);
	--v->wait;
	const bool lastWait = (rc == WAIT_OBJECT_0 + WIN32_CV_EVT_BCAST) && (v->wait == 0);
	LeaveCriticalSection(&v->lock);

	if (lastWait)
		ResetEvent(v->evt[WIN32_CV_EVT_BCAST]);

	EnterCriticalSection(cs);

	return success;
}

#endif

/* NekoEngine
 *
 * Thread.c
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
