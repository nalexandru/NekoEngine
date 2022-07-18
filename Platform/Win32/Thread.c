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
Sys_InitThread(NeThread *t, const char *name, void (*proc)(void *), void *args)
{
	HANDLE thread;
	THREADNAME_INFO info = { 0x1000, NULL, 0, 0 };
	
	thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)proc, args, 0, &info.dwThreadID);
	if (thread == INVALID_HANDLE_VALUE)
		return false;

	if (k32_SetThreadDescription)
		k32_SetThreadDescription(thread, NeWin32_UTF8toUCS2(name));

	info.szName = name;

#ifdef MSC_VER
	__try {
		RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), (const ULONG_PTR *)&info);
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

void Sys_JoinThread(NeThread t)
{
	WaitForSingleObject((HANDLE)t, INFINITE);
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
	SRWLOCK *srw = HeapAlloc(GetProcessHeap(), 0, sizeof(*srw));
	if (!srw)
		return false;

	InitializeSRWLock(srw);

	*ftx = srw;

	return true;
}

bool
Sys_LockFutex(NeFutex ftx)
{
	AcquireSRWLockExclusive((PSRWLOCK)ftx);
	return true;
}

bool
Sys_UnlockFutex(NeFutex ftx)
{
	ReleaseSRWLockExclusive((PSRWLOCK)ftx);
	return true;
}

void
Sys_TermFutex(NeFutex ftx)
{
	HeapFree(GetProcessHeap(), 0, ftx);
}

// Condition variable

bool
Sys_InitConditionVariable(NeConditionVariable *cv)
{
	CONDITION_VARIABLE *v = HeapAlloc(GetProcessHeap(), 0, sizeof(*v));
	if (!v)
		return false;

	InitializeConditionVariable(v);

	*cv = v;

	return true;
}

void
Sys_Signal(NeConditionVariable cv)
{
	WakeConditionVariable((PCONDITION_VARIABLE)cv);
}

void
Sys_Broadcast(NeConditionVariable cv)
{
	WakeAllConditionVariable((PCONDITION_VARIABLE)cv);
}

bool
Sys_WaitMutex(NeConditionVariable cv, NeMutex mtx)
{
	return SleepConditionVariableCS((PCONDITION_VARIABLE)cv, (PCRITICAL_SECTION)mtx, INFINITE);
}

bool
Sys_WaitFutex(NeConditionVariable cv, NeFutex ftx)
{
	return SleepConditionVariableSRW((PCONDITION_VARIABLE)cv, (PSRWLOCK)ftx, INFINITE, 0);
}

void
Sys_TermConditionVariable(NeConditionVariable cv)
{
	HeapFree(GetProcessHeap(), 0, cv);
}
