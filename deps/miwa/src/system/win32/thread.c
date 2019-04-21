/* Miwa Portable Runtime
 *
 * thread.c
 * Author: Alexandru Naiman
 *
 * Win32 Thread
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (c) 2018-2019, Alexandru Naiman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <windows.h>

#include <system/thread.h>

#include "compat_win32.h"

typedef struct WIN32_TA
{
	void (*thread_proc)(void *);
	void *args;
} win32_ta;

struct SYS_THREAD
{
	HANDLE handle;
	DWORD id;
	bool running;
	win32_ta args;
};

static DWORD __stdcall
_win32_thread_proc(void *thread_ptr)
{
	sys_thread *thread = NULL;

	if (!thread_ptr)
		return -1;
	
	thread = thread_ptr;
	thread->args.thread_proc(thread->args.args);
	thread->running = false;

	return 0;
}

sys_thread *
sys_thread_create(void (*thread_proc)(void *))
{
	sys_thread *th = calloc(1, sizeof(*th));
	if (!th)
		return NULL;

	th->running = false;
	th->args.thread_proc = thread_proc;

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

	thread->handle = CreateThread(NULL, 0, _win32_thread_proc,
		thread, 0, &thread->id);

	if (thread->handle == INVALID_HANDLE_VALUE)
		return false;

	thread->running = true;
	return true;
}

bool
sys_thread_join(sys_thread *thread)
{
	if (!thread)
		return false;
	
	return WaitForSingleObject(thread->handle, INFINITE) == WAIT_OBJECT_0;
}

bool
sys_thread_timed_join(sys_thread *thread,
	int32_t timeout)
{
	if (!thread)
		return false;
	
	return WaitForSingleObject(thread->handle, timeout) == WAIT_OBJECT_0;
}

bool
sys_thread_detach(sys_thread *thread)
{
	if (!thread)
		return false;
	
	return CloseHandle(thread->handle) != 0;
}

bool
sys_thread_abort(sys_thread *thread)
{
	if (!thread)
		return false;
	
	if (!TerminateThread(thread->handle, 0))
		return false;

	thread->running = false;

	return true;
}

bool
sys_thread_set_affinity(sys_thread *thread,
	int cpu)
{
	return !SetThreadAffinityMask(thread->handle, (DWORD)cpu);
}

uint32_t
sys_tls_alloc_key(void)
{
	return TlsAlloc();
}

void *
sys_tls_get_value(uint32_t key)
{
	return TlsGetValue(key);
}

void
sys_tls_set_value(uint32_t key,
	void *value)
{
	TlsSetValue(key, value);
}

void
sys_tls_free_key(uint32_t key)
{
	TlsFree(key);
}

void
sys_thread_destroy(sys_thread *thread)
{
	if (thread->running)
		sys_thread_abort(thread);

	free(thread);
}

void
sys_yield(void)
{
	win32_SwitchToThread();
}

sys_ilock_int
sys_ilock_inc(sys_ilock_int *v)
{
	return InterlockedIncrementAcquire(v);
}

sys_ilock_int
sys_ilock_dec(sys_ilock_int *v)
{
	return InterlockedDecrementRelease(v);
}

sys_ilock_int
sys_ilock_add(sys_ilock_int *v,
	sys_ilock_int i)
{
	return InterlockedExchangeAdd(v, i) + i;
}

sys_ilock_int
sys_ilock_sub(sys_ilock_int *v,
	sys_ilock_int i)
{
	return InterlockedExchangeAdd(v, -i) - i;
}

sys_ilock_int
sys_ilock_swap(sys_ilock_int *v,
	sys_ilock_int i)
{
	return InterlockedExchange(v, i);
}

sys_ilock_int
sys_ilock_compare(sys_ilock_int *v,
	sys_ilock_int c,
	sys_ilock_int i)
{
	return InterlockedCompareExchange(v, i, c);
}
