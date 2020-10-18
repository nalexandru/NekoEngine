#include <Windows.h>
#include <VersionHelpers.h>

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

HRESULT (*k32_SetThreadDescription)(HANDLE, PCWSTR);

bool
Sys_InitThread(Thread *t, const wchar_t *name, void (*proc)(void *), void *args)
{
	DWORD id;
	HANDLE thread;
	char *aname;
	
	thread = CreateThread(NULL, 0, (DWORD(*)(LPVOID))proc, args, 0, &id);
	if (thread == INVALID_HANDLE_VALUE)
		return false;

	if (k32_SetThreadDescription)
		k32_SetThreadDescription(thread, name);

	aname = Sys_Alloc(sizeof(char), wcslen(name) + 1, MH_Transient);
	wcstombs(aname, name, wcslen(name));

	THREADNAME_INFO info = { 0x1000, aname, id, 0 };
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
Sys_LockMutex(Mutex *mtx)
{
	EnterCriticalSection((LPCRITICAL_SECTION)mtx);
	return true;
}

bool
Sys_UnlockMutex(Mutex *mtx)
{
	LeaveCriticalSection((LPCRITICAL_SECTION)mtx);
	return true;
}

void
Sys_TermMutex(Mutex *mtx)
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
Sys_LockFutex(Futex *ftx)
{
	AcquireSRWLockExclusive((PSRWLOCK)ftx);
	return true;
}

bool
Sys_UnlockFutex(Futex *ftx)
{
	ReleaseSRWLockExclusive((PSRWLOCK)ftx);
	return true;
}

void
Sys_TermFutex(Futex *ftx)
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

// Atomic operations

int32_t
Sys_AtomicAdd(volatile int32_t *i, int32_t v)
{
	return InterlockedAdd(i, v);
}

int32_t
Sys_AtomicSub(volatile int32_t *i, int32_t v)
{
	return InterlockedAdd(i, -v);
}

int32_t
Sys_AtomicCompareAndSwap(volatile int32_t *i, int32_t e, int32_t c)
{
	return InterlockedCompareExchange(i, e, c);
}

int32_t
Sys_AtomicIncrement(volatile int32_t *i)
{
	return (int32_t)InterlockedIncrement((volatile LONG *)i);
}

int32_t
Sys_AtomicDecrement(volatile int32_t *i)
{
	return (int32_t)InterlockedDecrement((volatile LONG *)i);
}

int64_t
Sys_AtomicAdd64(volatile int64_t *i, int64_t v)
{
	return InterlockedAdd64(i, v);
}

int64_t
Sys_AtomicSub64(volatile int64_t *i, int64_t v)
{
	return InterlockedAdd64(i, -v);
}

int64_t
Sys_AtomicCompareAndSwap64(volatile int64_t *i, int64_t e, int64_t c)
{
	return InterlockedCompareExchange64(i, e, c);
}

int64_t
Sys_AtomicIncrement64(volatile int64_t *i)
{
	return (int64_t)InterlockedIncrement64((volatile LONG64 *)i);
}

int64_t
Sys_AtomicDecrement64(volatile int64_t *i)
{
	return (int64_t)InterlockedDecrement64((volatile LONG64 *)i);
}

