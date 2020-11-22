#include <stdlib.h>

#include <pthread.h>

#include <System/Thread.h>

#include "MacXPlatform.h"

#include <mach/thread_act.h>
#include <mach/thread_policy.h>

//#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
//#	include <stdatomic.h>
//#	define USE_STDC
//#elif MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_3
#include <libkern/OSAtomic.h>
#define USE_OSATOMIC

uint32_t
Sys_TlsAlloc(void)
{
	pthread_key_t key;
	pthread_key_create(&key, NULL);
	return (uint32_t)key;
}

void *
Sys_TlsGet(uint32_t key)
{
	return pthread_getspecific((pthread_key_t)key);
}

void
Sys_TlsSet(uint32_t key, void *data)
{
	pthread_setspecific((pthread_key_t)key, data);
}

void
Sys_TlsFree(uint32_t key)
{
	pthread_key_delete((pthread_key_t)key);
}

void
Sys_Yield(void)
{
	sched_yield();
}

bool
Sys_InitThread(Thread *t, const wchar_t *name, void (*proc)(void *), void *args)
{
	(void)name;
	
	if (!pthread_create((pthread_t *)t, NULL, (void *(*)(void *))proc, args))
		return false;
	
	return true;
}

void
Sys_SetThreadAffinity(Thread t, int cpu)
{
	thread_affinity_policy_data_t pd = { cpu };
	thread_policy_set(&t, THREAD_AFFINITY_POLICY, (thread_policy_t)&pd, THREAD_AFFINITY_POLICY_COUNT);
}

void
Sys_JoinThread(Thread t)
{
	pthread_join((pthread_t)t, NULL);
}

void
Sys_TermThread(Thread t)
{
	pthread_cancel((pthread_t)t);
}

bool
Sys_InitMutex(Mutex *mtx)
{
	return Sys_InitFutex((Futex *)mtx);
}

bool
Sys_LockMutex(Mutex mtx)
{
	return Sys_LockFutex((Futex)mtx);
}

bool
Sys_UnlockMutex(Mutex mtx)
{
	return Sys_UnlockFutex((Futex)mtx);
}

void
Sys_TermMutex(Mutex mtx)
{
	Sys_TermFutex((Futex)mtx);
}

bool
Sys_InitFutex(Futex *ftx)
{
	pthread_mutex_t *m = malloc(sizeof(*m));
	if (!m)
		return false;
	
	*ftx = m;
	
	return !pthread_mutex_init(m, NULL);
}

bool
Sys_LockFutex(Futex ftx)
{
	return !pthread_mutex_lock((pthread_mutex_t *)ftx);
}

bool
Sys_UnlockFutex(Futex ftx)
{
	return !pthread_mutex_unlock((pthread_mutex_t *)ftx);
}

void
Sys_TermFutex(Futex ftx)
{
	pthread_mutex_destroy((pthread_mutex_t *)ftx);
	free(ftx);
}

bool
Sys_InitConditionVariable(ConditionVariable *cv)
{
	pthread_cond_t *c = malloc(sizeof(*c));
	if (!c)
		return false;
	
	*cv = c;
	
	return !pthread_cond_init(c, NULL);
}

void
Sys_Signal(ConditionVariable cv)
{
	pthread_cond_signal((pthread_cond_t *)cv);
}

void
Sys_Broadcast(ConditionVariable cv)
{
	pthread_cond_broadcast((pthread_cond_t *)cv);
}

bool
Sys_WaitMutex(ConditionVariable cv, Mutex mtx)
{
	return Sys_WaitFutex(cv, (Futex)mtx);
}

bool
Sys_WaitFutex(ConditionVariable cv, Futex ftx)
{
	return !pthread_cond_wait((pthread_cond_t *)cv, (pthread_mutex_t *)ftx);
}

void
Sys_TermConditionVariable(ConditionVariable cv)
{
	pthread_cond_destroy((pthread_cond_t *)cv);
	free(cv);
}

int32_t
Sys_AtomicAdd(volatile int32_t *i, int32_t v)
{
	return OSAtomicAdd32Barrier(v, (int32_t *)i);
	//return atomic_fetch_add(i, v);
}

int32_t
Sys_AtomicSub(volatile int32_t *i, int32_t v)
{
	return OSAtomicAdd32Barrier(-v, (int32_t *)i);
	//return atomic_fetch_add(i, -v);
}

int32_t
Sys_AtomicCompareAndSwap(volatile int32_t *i, int32_t e, int32_t c)
{
	return OSAtomicCompareAndSwap32Barrier(c, e, (int32_t *)i);
	//return atomic_compare_exchange_strong(i, &e, c);
}

int32_t
Sys_AtomicIncrement(volatile int32_t *i)
{
	return OSAtomicIncrement32Barrier((int32_t *)i);
	//return atomic_fetch_add(i, 1);
}

int32_t
Sys_AtomicDecrement(volatile int32_t *i)
{
	return OSAtomicDecrement32Barrier((int32_t *)i);
//	return atomic_fetch_add(i, -1);
}

int64_t
Sys_AtomicAdd64(volatile int64_t *i, int64_t v)
{
	return OSAtomicAdd64Barrier(v, (int64_t *)i);
//	return atomic_fetch_add(i, v);
}

int64_t
Sys_AtomicSub64(volatile int64_t *i, int64_t v)
{
	return OSAtomicAdd64Barrier(-v, (int64_t *)i);
//	return atomic_fetch_add(i, -v);
}

int64_t
Sys_AtomicCompareAndSwap64(volatile int64_t *i, int64_t e, int64_t c)
{
	return OSAtomicCompareAndSwap64Barrier(c, e, (int64_t *)i);
//	return atomic_compare_exchange_strong(i, &c, e);
}

int64_t
Sys_AtomicIncrement64(volatile int64_t *i)
{
	return OSAtomicIncrement64Barrier((int64_t *)i);
//	return atomic_fetch_add(i, 1);
}

int64_t
Sys_AtomicDecrement64(volatile int64_t *i)
{
	return OSAtomicDecrement64Barrier((int64_t *)i);
//	return atomic_fetch_add(i, -1);
}
