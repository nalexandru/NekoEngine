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
	SwitchToThread();
}

bool
Sys_InitThread(Thread *t, const wchar_t *name, void (*proc)(void *), void *args)
{
	HANDLE thread;
	THREADNAME_INFO info = { 0x1000, NULL, 0, 0 };
	
	thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)proc, args, 0, &info.dwThreadID);
	if (thread == INVALID_HANDLE_VALUE)
		return false;

	if (k32_SetThreadDescription)
		k32_SetThreadDescription(thread, name);

	info.szName = Sys_Alloc(sizeof(char), wcslen(name) + 1, MH_Transient);
	wcstombs((char *)info.szName, name, wcslen(name));
	
	__try {
		RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), (const ULONG_PTR *)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
	}

	*t = thread;

	return true;
}

void
Sys_SetThreadAffinity(Thread t, int cpu)
{
	SetThreadAffinityMask((HANDLE)t, (DWORD_PTR)1 << ((DWORD_PTR)cpu));
}

void Sys_JoinThread(Thread t)
{
	WaitForSingleObject((HANDLE)t, INFINITE);
}

void Sys_TermThread(Thread t)
{
	TerminateThread((HANDLE)t, 0);
}

// Mutex

bool
Sys_InitMutex(Mutex *mtx)
{
	CRITICAL_SECTION *cs = HeapAlloc(GetProcessHeap(), 0, sizeof(*cs));
	if (!cs)
		return false;

	InitializeCriticalSection(cs);

	*mtx = cs;

	return true;
}

bool
Sys_LockMutex(Mutex mtx)
{
	EnterCriticalSection((LPCRITICAL_SECTION)mtx);
	return true;
}

bool
Sys_UnlockMutex(Mutex mtx)
{
	LeaveCriticalSection((LPCRITICAL_SECTION)mtx);
	return true;
}

void
Sys_TermMutex(Mutex mtx)
{
	DeleteCriticalSection((LPCRITICAL_SECTION)mtx);
	HeapFree(GetProcessHeap(), 0, mtx);
}

// Futex

bool
Sys_InitFutex(Futex *ftx)
{
	SRWLOCK *srw = HeapAlloc(GetProcessHeap(), 0, sizeof(*srw));
	if (!srw)
		return false;

	InitializeSRWLock(srw);

	*ftx = srw;

	return true;
}

bool
Sys_LockFutex(Futex ftx)
{
	AcquireSRWLockExclusive((PSRWLOCK)ftx);
	return true;
}

bool
Sys_UnlockFutex(Futex ftx)
{
	ReleaseSRWLockExclusive((PSRWLOCK)ftx);
	return true;
}

void
Sys_TermFutex(Futex ftx)
{
	HeapFree(GetProcessHeap(), 0, ftx);
}

// Condition variable

bool
Sys_InitConditionVariable(ConditionVariable *cv)
{
	CONDITION_VARIABLE *v = HeapAlloc(GetProcessHeap(), 0, sizeof(*v));
	if (!v)
		return false;

	InitializeConditionVariable(v);

	*cv = v;

	return true;
}

void
Sys_Signal(ConditionVariable cv)
{
	WakeConditionVariable((PCONDITION_VARIABLE)cv);
}

void
Sys_Broadcast(ConditionVariable cv)
{
	WakeAllConditionVariable((PCONDITION_VARIABLE)cv);
}

bool
Sys_WaitMutex(ConditionVariable cv, Mutex mtx)
{
	return SleepConditionVariableCS((PCONDITION_VARIABLE)cv, (PCRITICAL_SECTION)mtx, INFINITE);
}

bool
Sys_WaitFutex(ConditionVariable cv, Futex ftx)
{
	return SleepConditionVariableSRW((PCONDITION_VARIABLE)cv, (PSRWLOCK)ftx, INFINITE, 0);
}

void
Sys_TermConditionVariable(ConditionVariable cv)
{
	HeapFree(GetProcessHeap(), 0, cv);
}
