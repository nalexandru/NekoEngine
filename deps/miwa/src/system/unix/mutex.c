/* Miwa Portable Runtime
 *
 * mutex.c
 * Author: Alexandru Naiman
 *
 * UNIX Mutex
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

#include <system/mutex.h>

struct SYS_MUTEX
{
	pthread_mutex_t mtx;
};

sys_mutex *
sys_mutex_create(void)
{
	sys_mutex *mtx = (sys_mutex *)calloc(1, sizeof(*mtx));
	if (!mtx)
		return NULL;

	pthread_mutex_init(&mtx->mtx, 0);

	return mtx;
}

bool
sys_mutex_lock(sys_mutex *mutex)
{
	if (!mutex)
		return false;
	
	return pthread_mutex_lock(&mutex->mtx) == 0;
}

bool
sys_mutex_try_lock(sys_mutex *mutex)
{
	if (!mutex)
		return false;
	
	return pthread_mutex_trylock(&mutex->mtx) == 0;
}

bool
sys_mutex_unlock(sys_mutex *mutex)
{
	if (!mutex)
		return false;
	
	return pthread_mutex_unlock(&mutex->mtx) == 0;
}

void
sys_mutex_destroy(sys_mutex *mutex)
{
	if (!mutex)
		return;
	
	pthread_mutex_destroy(&mutex->mtx);
	free(mutex);
}

