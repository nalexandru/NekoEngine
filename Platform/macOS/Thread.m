#include <stdlib.h>
#include <pthread.h>

#include <System/Thread.h>
#include <System/Memory.h>
#include <Runtime/Runtime.h>

#include <mach/thread_act.h>
#include <mach/thread_policy.h>

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
Sys_InitThread(NeThread *t, const char *name, void (*proc)(void *), void *args)
{
	pthread_attr_t attr;

	(void)name;

	pthread_attr_init(&attr);
	pthread_attr_set_qos_class_np(&attr, QOS_CLASS_USER_INTERACTIVE, 0);

	if (!pthread_create((pthread_t *)t, &attr, (void *(*)(void *))proc, args))
		return false;

	pthread_attr_destroy(&attr);

	return true;
}

void
Sys_SetThreadAffinity(NeThread t, int cpu)
{
	thread_affinity_policy_data_t pd = { cpu };
	mach_port_t machThread = pthread_mach_thread_np((pthread_t)t);
	thread_policy_set(machThread, THREAD_AFFINITY_POLICY, (thread_policy_t)&pd, THREAD_AFFINITY_POLICY_COUNT);
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
