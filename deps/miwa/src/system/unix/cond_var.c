/* Miwa Portable Runtime
 *
 * cond_var.c
 * Author: Alexandru Naiman
 *
 * UNIX Condition Variable
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

#include <stdlib.h>
#include <pthread.h>

#include <system/cond_var.h>

struct SYS_MUTEX
{
	pthread_mutex_t mtx;
};

struct SYS_COND_VAR
{
	pthread_cond_t var;
};

sys_cond_var *
sys_cond_var_create(void)
{
	sys_cond_var *cond_var = (sys_cond_var *)calloc(1, sizeof(sys_cond_var));
	if (!cond_var)
		return NULL;

	pthread_cond_init(&cond_var->var, NULL);

	return cond_var;
}

bool
sys_cond_var_wait(sys_cond_var *cond_var,
	sys_mutex *mutex,
	uint32_t timeout)
{
	if (!cond_var || !mutex)
		return false;
	
	if (timeout) {
		struct timespec ts;
		ts.tv_sec = timeout / 1000;
		ts.tv_nsec = (timeout % 1000) * 1000000;

		if (pthread_cond_timedwait(&cond_var->var, &mutex->mtx, &ts)
				!= 0)
			return false;

		return true;
	}
	
	pthread_cond_wait(&cond_var->var, &mutex->mtx);
	return true;
}

void
sys_cond_var_signal(sys_cond_var *cond_var)
{
	if (!cond_var)
		return;
	
	pthread_cond_signal(&cond_var->var);
}

void
sys_cond_var_broadcast(sys_cond_var *cond_var)
{
	if (!cond_var)
		return;
	
	pthread_cond_broadcast(&cond_var->var);
}

void
sys_cond_var_destroy(sys_cond_var *cond_var)
{
	if (!cond_var)
		return;
	
	pthread_cond_destroy(&cond_var->var);
	free(cond_var);
}

