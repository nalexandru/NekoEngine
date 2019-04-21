/* Miwa Portable Runtime
 *
 * thread.h
 * Author: Alexandru Naiman
 *
 * Thread UNIX Implementation
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2010-2019, Alexandru Naiman
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#define _GNU_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <sched.h>

#include <system/thread.h>
#include <system/platform.h>

#ifdef __ANDROID__
	#include <signal.h>
#endif

#if defined(SYS_PLATFORM_APPLE)
	#include <libkern/OSAtomic.h>
	#include <mach/thread_policy.h>
	#include <mach/task_info.h>
	#include <mach/thread_policy.h>
	#include <mach/thread_act.h>
#elif defined(SYS_PLATFORM_SUNOS)
	#include <atomic.h>
	#include <sys/processor.h>
	#include <sys/procset.h>
	#include <thread.h>
#endif

struct POSIX_TA
{
	void (*thread_proc)(void *);
	void *args;
};

struct SYS_THREAD
{
	pthread_t handle;
	bool running;
	bool suspended;
	bool detached;
	pthread_cond_t resume_cond;
	pthread_mutex_t suspend_mutex;
	struct POSIX_TA args;
};

static void *
_posix_thread_proc(void *thread_ptr)
{
	if (!thread_ptr)
		return 0;

	sys_thread *thread = (sys_thread *)thread_ptr;

	thread->args.thread_proc(thread->args.args);

	thread->running = false;

	return 0;
}

sys_thread *
sys_thread_create(void (*thread_proc)(void *))
{
	sys_thread *th = (sys_thread *)calloc(1, sizeof(*th));
	if (!th)
		return NULL;

	th->running = false;
	th->args.thread_proc = thread_proc;

	pthread_cond_init(&th->resume_cond, NULL);
	pthread_mutex_init(&th->suspend_mutex, NULL);

	return th;
}

void
sys_thread_set_proc(sys_thread *thread,
	void(*thread_proc)(void *))
{
	if (!thread)
		return;

	thread->args.thread_proc = thread_proc;
}

bool
sys_thread_is_running(sys_thread *thread)
{
	if (!thread)
		return false;

	return thread->running;
}

bool
sys_thread_start(sys_thread *thread,
	void *args)
{
	if (!thread)
		return false;

	if (thread->running)
		return true;

	thread->args.args = args;

	if (pthread_create(&thread->handle, 0, _posix_thread_proc, thread) != 0)
		return false;

	thread->running = true;
	return true;
}

bool
sys_thread_join(sys_thread *thread)
{
	if (!thread)
		return false;

	return pthread_join(thread->handle, NULL) == 0;
}

bool
sys_thread_timed_join(sys_thread *thread,
	int32_t timeout)
{
	if (!thread)
		return false;

	// FIXME
	return pthread_join(thread->handle, NULL) == 0;
}

bool
sys_thread_detach(sys_thread *thread)
{
	int ret = 0;

	if (!thread)
		return false;

	ret = pthread_detach(thread->handle);

	if (!ret)
		thread->detached = true;

	return ret;
}

bool
sys_thread_abort(sys_thread *thread)
{
	if (!thread)
		return false;

	if (!thread->detached)
		pthread_detach(thread->handle);

#ifndef __ANDROID__
	if (pthread_cancel(thread->handle) != 0)
		return false;
#else
	if (pthread_kill(thread->handle, SIGUSR2) != 0)
		return false;
#endif

	pthread_join(thread->handle, NULL);

	thread->running = false;

	return true;
}

bool
sys_thread_set_affinity(sys_thread *thread,
	int cpu)
{
#if defined(SYS_PLATFORM_APPLE)
	thread_affinity_policy_data_t pd = { cpu };
	return !thread_policy_set(&thread->handle, THREAD_AFFINITY_POLICY,
					  (thread_policy_t)&pd, THREAD_AFFINITY_POLICY_COUNT);
#elif defined(SYS_PLATFORM_SUNOS)
/*	procset_t ps;
	uint_t nid = 1;
	id_t id = cpu;
	uint32_t flags = PA_TYPE_CPU | PA_AFF_STRONG;

	setprocset(&ps, POP_AND, P_PID, P_MYID, P_LWPID, thr_self());

	return !processor_affinity(&ps, &nid, &id, &flags); */
	return -1;
#else
	cpu_set_t cpu_set;
	CPU_ZERO(&cpu_set);
	CPU_SET(cpu, &cpu_set);
	return !pthread_setaffinity_np(thread->handle,
					sizeof(cpu_set_t), &cpu_set);
#endif
}

uint32_t
sys_tls_alloc_key(void)
{
	pthread_key_t key;
	pthread_key_create(&key, NULL);
	return (uint32_t)key;
}

void *
sys_tls_get_value(uint32_t key)
{
	return pthread_getspecific((pthread_key_t)key);
}

void
sys_tls_set_value(uint32_t key,
	void *value)
{
	pthread_setspecific((pthread_key_t)key, value);
}

void
sys_tls_free_key(uint32_t key)
{
	pthread_key_delete((pthread_key_t)key);
}

void
sys_thread_destroy(sys_thread *thread)
{
	if (!thread)
		return;

	if (thread->running)
		sys_thread_abort(thread);

	pthread_cond_destroy(&thread->resume_cond);
	pthread_mutex_destroy(&thread->suspend_mutex);

	free(thread);
}

void
sys_yield(void)
{
	sched_yield();
}

sys_ilock_int
sys_ilock_inc(sys_ilock_int *v)
{
#if defined(SYS_PLATFORM_APPLE)
	return OSAtomicIncrement32Barrier(v);
#elif defined(SYS_PLATFORM_SUNOS)
	return atomic_inc_32_nv(v);
#else
	return __sync_add_and_fetch(v, 1);
#endif
}

sys_ilock_int
sys_ilock_dec(sys_ilock_int *v)
{
#if defined(SYS_PLATFORM_APPLE)
	return OSAtomicDecrement32Barrier(v);
#elif defined(SYS_PLATFORM_SUNOS)
	return atomic_dec_32_nv(v);
#else
	return __sync_sub_and_fetch(v, 1);
#endif
}

sys_ilock_int
sys_ilock_add(sys_ilock_int *v,
	sys_ilock_int i)
{
#if defined(SYS_PLATFORM_APPLE)
	return OSAtomicAdd32Barrier(i, v);
#elif defined(SYS_PLATFORM_SUNOS)
	return atomic_add_32_nv(v, i);
#else
	return __sync_add_and_fetch(v, i);
#endif
}

sys_ilock_int
sys_ilock_sub(sys_ilock_int *v,
	sys_ilock_int i)
{
#if defined(SYS_PLATFORM_APPLE)
	return OSAtomicAdd32Barrier(-i, v);
#elif defined(SYS_PLATFORM_SUNOS)
	return atomic_add_32_nv(v, -i);
#else
	return __sync_sub_and_fetch(v, i);
#endif
}

sys_ilock_int
sys_ilock_swap(sys_ilock_int *v,
	sys_ilock_int i)
{
#if defined(SYS_PLATFORM_APPLE)
	return OSAtomicCompareAndSwap32Barrier(*v, i, v);
#elif defined(SYS_PLATFORM_SUNOS)
	return atomic_swap_32(v, i);
#else
	return __sync_val_compare_and_swap(v, *v, i);
#endif
}

sys_ilock_int
sys_ilock_compare(sys_ilock_int *v,
	sys_ilock_int c,
	sys_ilock_int i)
{
#if defined(SYS_PLATFORM_APPLE)
	return OSAtomicCompareAndSwap32Barrier(c, i, v);
#elif defined(SYS_PLATFORM_SUNOS)
	return atomic_cas_32(v, c, i);
#else
	return __sync_val_compare_and_swap(v, c, i);
#endif
}

