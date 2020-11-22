#include <stdlib.h>

#include <pthread.h>

#include <System/Thread.h>

#include "MacXPlatform.h"

#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
#	include <stdatomic.h>
#	define USE_STDC
#elif MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_3
#	include <libkern/OSAtomic.h>
#	define USE_OSATOMIC
#else
#	define USE_GCC
#	warning "Mac OS X 10.4 or later is required for atomic operations"
#endif

#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
#	include <mach/thread_policy.h>
#endif


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
#if defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
	thread_affinity_policy_data_t pd = { cpu };
	thread_policy_set(&t, THREAD_AFFINITY_POLICY, (thread_policy_t)&pd, THREAD_AFFINITY_POLICY_COUNT);
#endif
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
#if defined(USE_OSATOMIC)
	return OSAtomicAdd32Barrier(v, (int32_t *)i);
#elif defined(USE_STDC)
	return atomic_fetch_add(i, v);
#elif defined(USE_GCC)
	return __sync_add_and_fetch(i, v);
#endif
}

int32_t
Sys_AtomicSub(volatile int32_t *i, int32_t v)
{
#if defined(USE_OSATOMIC)
	return OSAtomicAdd32Barrier(-v, (int32_t *)i);
#elif defined(USE_STDC)
	return atomic_fetch_add(i, -v);
#elif defined(USE_GCC)
	return __sync_add_and_fetch(i, -v);
#endif
}

int32_t
Sys_AtomicCompareAndSwap(volatile int32_t *i, int32_t e, int32_t c)
{
#if defined(USE_OSATOMIC)
	return OSAtomicCompareAndSwap32Barrier(c, e, (int32_t *)i);
#elif defined(USE_STDC)
	return atomic_compare_exchange_strong(i, &e, c);
#elif defined(USE_GCC)
	return __sync_val_compare_and_swap(i, c, e);
#endif
}

int32_t
Sys_AtomicIncrement(volatile int32_t *i)
{
#if defined(USE_OSATOMIC)
	return OSAtomicIncrement32Barrier((int32_t *)i);
#elif defined(USE_STDC)
	return atomic_fetch_add(i, 1);
#elif defined(USE_GCC)
	return __sync_add_and_fetch(i, 1);
#endif
}

int32_t
Sys_AtomicDecrement(volatile int32_t *i)
{
#if defined(USE_OSATOMIC)
	return OSAtomicDecrement32Barrier((int32_t *)i);
#elif defined(USE_STDC)
	return atomic_fetch_add(i, -1);
#elif defined(USE_GCC)
	return __sync_add_and_fetch(i, -1);
#endif
}

int64_t
Sys_AtomicAdd64(volatile int64_t *i, int64_t v)
{
#if defined(USE_OSATOMIC)
#	if defined(_ARCH_PPC) && !defined(_ARCH_PPC64)
		return OSAtomicAdd32Barrier((int32_t)v, (int32_t *)i);
#	else
		return OSAtomicAdd64Barrier(v, (int64_t *)i);
#	endif
#elif defined(USE_STDC)
	return atomic_fetch_add(i, v);
#elif defined(USE_GCC)
	return __sync_add_and_fetch(i, v);
#endif
}

int64_t
Sys_AtomicSub64(volatile int64_t *i, int64_t v)
{
#if defined(USE_OSATOMIC)
#	if defined(_ARCH_PPC) && !defined(_ARCH_PPC64)
		return OSAtomicAdd32Barrier((int32_t)-v, (int32_t *)i);
#	else
		return OSAtomicAdd64Barrier(-v, (int64_t *)i);
#	endif
#elif defined(USE_STDC)
	return atomic_fetch_add(i, -v);
#elif defined(USE_GCC)
	return __sync_add_and_fetch(i, -v);
#endif
}

int64_t
Sys_AtomicCompareAndSwap64(volatile int64_t *i, int64_t e, int64_t c)
{
#if defined(USE_OSATOMIC)
#	if defined(_ARCH_PPC) && !defined(_ARCH_PPC64)
	return OSAtomicCompareAndSwap32Barrier((int32_t)c, (int32_t)e, (int32_t *)i);
#	else
	return OSAtomicCompareAndSwap64Barrier(c, e, (int64_t *)i);
#	endif
#elif defined(USE_STDC)
	return atomic_compare_exchange_strong(i, &c, e);
#elif defined(USE_GCC)
	return __sync_val_compare_and_swap(i, c, e);
#endif
}

int64_t
Sys_AtomicIncrement64(volatile int64_t *i)
{
#if defined(USE_OSATOMIC)
#	if defined(_ARCH_PPC) && !defined(_ARCH_PPC64)
	return OSAtomicIncrement32Barrier((int32_t *)i);
#	else
	return OSAtomicIncrement64Barrier((int64_t *)i);
#	endif
#elif defined(USE_STDC)
	return atomic_fetch_add(i, 1);
#elif defined(USE_GCC)
	return __sync_add_and_fetch(i, 1);
#endif
}

int64_t
Sys_AtomicDecrement64(volatile int64_t *i)
{
#if defined(USE_OSATOMIC)
#	if defined(_ARCH_PPC) && !defined(_ARCH_PPC64)
	return OSAtomicDecrement32Barrier((int32_t *)i);
#	else
	return OSAtomicDecrement64Barrier((int64_t *)i);
#	endif
#elif defined(USE_STDC)
	return atomic_fetch_add(i, -1);
#elif defined(USE_GCC)
	return __sync_add_and_fetch(i, -1);
#endif
}
