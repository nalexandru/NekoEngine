#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdatomic.h>

#include <sched.h>
#include <pthread.h>

#if defined(__FreeBSD__)
#	include <sys/param.h>
#	include <sys/cpuset.h>
#	include <pthread_np.h>
#elif defined(__OpenBSD__)
#	include <pthread_np.h>
#endif

#include <System/System.h>
#include <System/Thread.h>
#include <System/Memory.h>
#include <Runtime/Runtime.h>

#include "UNIXPlatform.h"

bool
Sys_InitThread(NeThread *t, const char *name, void (*proc)(void *), void *args)
{
	(void)name;
	
	if (!pthread_create((pthread_t *)t, NULL, (void *(*)(void *))proc, args))
		return false;
		
	#if defined(__linux__) || defined(__NetBSD__)
		pthread_setname_np(*((pthread_t *)t), name);
	#elif defined(__FreeBSD__) || defined(__OpenBSD__)
		pthread_set_name_np(*((pthread_t *)t), name);
	#else
	#	warning "Thread naming not implemented for this platform"
	#endif

	return true;
}

void
Sys_SetThreadAffinity(NeThread t, int cpu)
{
#if defined(__linux__)
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(cpu, &set);
	pthread_setaffinity_np((pthread_t)t, sizeof(set), &set);
#elif defined(__FreeBSD__)
	cpuset_t set;
	CPU_ZERO(&set);
	CPU_SET(cpu, &set);
	pthread_setaffinity_np((pthread_t)t, sizeof(set), &set);
	//cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, -1, sizeof(set), &set);
#elif defined(__OpenBSD__)
	// cpu affinity is not exposed in userland on OpenBSD
#else
#	warning "Thread affinity not implemented for this platform"
#endif
}

void
Sys_JoinThread(NeThread t)
{
	pthread_join((pthread_t)t, NULL);
}

void
Sys_TermThread(NeThread t)
{
	pthread_cancel((pthread_t)t);
}

bool
Sys_InitMutex(NeMutex *mtx)
{
	return Sys_InitFutex((NeFutex *)mtx);
}

bool
Sys_LockMutex(NeMutex mtx)
{
	return Sys_LockFutex((NeFutex)mtx);
}

bool
Sys_UnlockMutex(NeMutex mtx)
{
	return Sys_UnlockFutex((NeFutex)mtx);
}

void
Sys_TermMutex(NeMutex mtx)
{
	Sys_TermFutex((NeFutex)mtx);
}

bool
Sys_InitFutex(NeFutex *ftx)
{
	pthread_mutex_t *m = Sys_Alloc(sizeof(*m), 1, MH_System);
	if (!m)
		return false;
	
	*ftx = m;
	
	return !pthread_mutex_init(m, NULL);
}

bool
Sys_LockFutex(NeFutex ftx)
{
	return !pthread_mutex_lock((pthread_mutex_t *)ftx);
}

bool
Sys_UnlockFutex(NeFutex ftx)
{
	return !pthread_mutex_unlock((pthread_mutex_t *)ftx);
}

void
Sys_TermFutex(NeFutex ftx)
{
	pthread_mutex_destroy((pthread_mutex_t *)ftx);
	Sys_Free(ftx);
}

bool
Sys_InitConditionVariable(NeConditionVariable *cv)
{
	pthread_cond_t *c = Sys_Alloc(sizeof(*c), 1, MH_System);
	if (!c)
		return false;
	
	*cv = c;
	
	return !pthread_cond_init(c, NULL);
}

void
Sys_Signal(NeConditionVariable cv)
{
	pthread_cond_signal((pthread_cond_t *)cv);
}

void
Sys_Broadcast(NeConditionVariable cv)
{
	pthread_cond_broadcast((pthread_cond_t *)cv);
}

bool
Sys_WaitMutex(NeConditionVariable cv, NeMutex mtx)
{
	return Sys_WaitFutex(cv, (NeFutex)mtx);
}

bool
Sys_WaitFutex(NeConditionVariable cv, NeFutex ftx)
{
	return !pthread_cond_wait((pthread_cond_t *)cv, (pthread_mutex_t *)ftx);
}

void
Sys_TermConditionVariable(NeConditionVariable cv)
{
	pthread_cond_destroy((pthread_cond_t *)cv);
	Sys_Free(cv);
}
