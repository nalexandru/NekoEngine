#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdatomic.h>

#include <sched.h>
#include <pthread.h>

#include <System/Thread.h>
#include <System/Memory.h>

#include "UNIXPlatform.h"

bool
Sys_InitThread(Thread *t, const wchar_t *name, void (*proc)(void *), void *args)
{
	(void)name;
	
	if (!pthread_create((pthread_t *)t, NULL, (void *(*)(void *))proc, args))
		return false;
	
	#if defined(__linux__)	
	//	pthread_setname_np(t, name);
	#else
	#	warning "Thread naming not implemented for this platform"
	#endif

	return true;
}

void
Sys_SetThreadAffinity(Thread t, int cpu)
{
#if defined(__linux__)
	cpu_set_t cpu_set;
	CPU_ZERO(&cpu_set);
	CPU_SET(cpu, &cpu_set);
	pthread_setaffinity_np((pthread_t)t, sizeof(cpu_set_t), &cpu_set);
#else
#	warning "Thread affinity not implemented for this platform"
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
	pthread_mutex_t *m = Sys_Alloc(sizeof(*m), 1, MH_System);
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
	Sys_Free(ftx);
}

bool
Sys_InitConditionVariable(ConditionVariable *cv)
{
	pthread_cond_t *c = Sys_Alloc(sizeof(*c), 1, MH_System);
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
	Sys_Free(cv);
}
