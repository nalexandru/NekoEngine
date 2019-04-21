/* Miwa Portable Runtime
 *
 * cond_var.c
 * Author: Alexandru Naiman
 *
 * Win32 Condition Variable
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdlib.h>

#include <system/cond_var.h>

struct SYS_MUTEX
{
	CRITICAL_SECTION cs;
};

#ifdef HAVE_WIN32_COND_VAR

struct SYS_COND_VAR
{
	CONDITION_VARIABLE var;
};

sys_cond_var *
sys_cond_var_create(void)
{
	sys_cond_var *cond_var = calloc(1, sizeof(*cond_var));
	if (!cond_var)
		return NULL;

	InitializeConditionVariable(&cond_var->var);

	return cond_var;
}

bool
sys_cond_var_wait(sys_cond_var *cond_var,
	sys_mutex *mutex,
	uint32_t timeout)
{
	if (!cond_var || !mutex)
		return false;
	
	return SleepConditionVariableCS(&cond_var->var,
		&mutex->cs, !timeout ? INFINITE : timeout) == TRUE;
}

void
sys_cond_var_signal(sys_cond_var *cond_var)
{
	if (!cond_var)
		return;
	
	WakeConditionVariable(&cond_var->var);
}

void
sys_cond_var_broadcast(sys_cond_var *cond_var)
{
	if (!cond_var)
		return;
	
	WakeAllConditionVariable(&cond_var->var);
}

void
sys_cond_var_destroy(sys_cond_var *cond_var)
{
	if (!cond_var)
		return;
	
	free(cond_var);
}

#else

/*
 * Condition variable implementation taken from GLFW 2.7.6
 * https://github.com/openglsuperbible/sb6code/blob/master/extern/glfw-2.7.6/lib/win32/win32_thread.c
 */

#define EVT_SIGNAL		0
#define EVT_BROADCAST	1

struct SYS_COND_VAR
{
	HANDLE evt[2];
	uint32_t wait;
	CRITICAL_SECTION wait_lock;
};

sys_cond_var *
sys_cond_var_create(void)
{
	sys_cond_var *cond_var = calloc(1, sizeof(*cond_var));
	if (!cond_var)
		return NULL;

	cond_var->evt[EVT_SIGNAL] = CreateEvent(NULL, FALSE, FALSE, NULL);
	cond_var->evt[EVT_BROADCAST] = CreateEvent(NULL, TRUE, FALSE, NULL);

	InitializeCriticalSection(&cond_var->wait_lock);

	return cond_var;
}

bool
sys_cond_var_wait(sys_cond_var *cond_var,
	sys_mutex *mutex,
	uint32_t timeout)
{
	int res, last_wait;
	bool ret = true;
	
	if (!cond_var || !mutex)
		return false;

	EnterCriticalSection(&cond_var->wait_lock);
	++cond_var->wait;
	LeaveCriticalSection(&cond_var->wait_lock);

	LeaveCriticalSection(&mutex->cs);

	res = WaitForMultipleObjects(2, cond_var->evt, FALSE, timeout);
	ret = (res != WAIT_TIMEOUT && res != WAIT_FAILED);

	EnterCriticalSection(&cond_var->wait_lock);
	--cond_var->wait;
	last_wait = (res == WAIT_OBJECT_0 + EVT_BROADCAST) && (cond_var->wait == 0);
	LeaveCriticalSection(&cond_var->wait_lock);

	if (last_wait)
		ResetEvent(cond_var->evt[EVT_BROADCAST]);

	EnterCriticalSection(&mutex->cs);

	return ret;
}

void
sys_cond_var_signal(sys_cond_var *cond_var)
{
	int set;
	
	if (!cond_var)
		return;

	EnterCriticalSection(&cond_var->wait_lock);
	set = cond_var->wait > 0;
	LeaveCriticalSection(&cond_var->wait_lock);

	if (set)
		SetEvent(cond_var->evt[EVT_SIGNAL]);
}

void
sys_cond_var_broadcast(sys_cond_var *cond_var)
{
	int set;
	
	if (!cond_var)
		return;

	EnterCriticalSection(&cond_var->wait_lock);
	set = cond_var->wait > 0;
	LeaveCriticalSection(&cond_var->wait_lock);

	if (set)
		SetEvent(cond_var->evt[EVT_BROADCAST]);
}

void
sys_cond_var_destroy(sys_cond_var *cond_var)
{
	if (!cond_var)
		return;
	
	CloseHandle(cond_var->evt[EVT_SIGNAL]);
	CloseHandle(cond_var->evt[EVT_BROADCAST]);

	DeleteCriticalSection(&cond_var->wait_lock);

	free(cond_var);
}

#endif

