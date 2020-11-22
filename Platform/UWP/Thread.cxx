#include <Windows.h>

#include <System/Thread.h>
#include <System/Memory.h>

bool
Sys_InitThread(Thread *t, const wchar_t *name, void (*proc)(void *), void *args)
{
	DWORD id;
	HANDLE thread;
	
	thread = CreateThread(NULL, 0, (DWORD(*)(LPVOID))proc, args, 0, &id);
	if (thread == INVALID_HANDLE_VALUE)
		return false;

	SetThreadDescription(thread, name);

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
	//
}

// Mutex

bool
Sys_InitMutex(Mutex *mtx)
{
	CRITICAL_SECTION *cs = (CRITICAL_SECTION *)HeapAlloc(GetProcessHeap(), 0, sizeof(*cs));
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
	SRWLOCK *srw = (SRWLOCK *)HeapAlloc(GetProcessHeap(), 0, sizeof(*srw));
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
	CONDITION_VARIABLE *v = (CONDITION_VARIABLE *)HeapAlloc(GetProcessHeap(), 0, sizeof(*v));
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
	return InterlockedAdd((volatile LONG *)i, v);
}

int32_t
Sys_AtomicSub(volatile int32_t *i, int32_t v)
{
	return InterlockedAdd((volatile LONG *)i, -v);
}

int32_t
Sys_AtomicCompareAndSwap(volatile int32_t *i, int32_t e, int32_t c)
{
	return InterlockedCompareExchange((volatile LONG *)i, e, c);
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
