/* Miwa Portable Runtime
 *
 * threadpool.h
 * Author: Alexandru Naiman
 *
 * Thread pool
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

#ifndef _MIWA_SYSTEM_THREADPOOL_H_
#define _MIWA_SYSTEM_THREADPOOL_H_

#include <system/mutex.h>
#include <system/thread.h>
#include <system/cond_var.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYS_TP_MAIN_THREAD	(uint32_t)-1;

typedef struct SYS_THREADPOOL sys_threadpool;

sys_threadpool *sys_threadpool_create(uint32_t num_workers);

bool sys_threadpool_is_idle(sys_threadpool *pool);
uint32_t sys_threadpool_worker_count(sys_threadpool *pool);

uint32_t sys_threadpool_worker_id(sys_threadpool *pool);

int sys_threadpool_add_task(sys_threadpool *pool, void (*task_proc)(void *), void *args, uint64_t *id);

bool sys_threadpool_wait_task(sys_threadpool *pool, uint64_t id, uint32_t timeout);
bool sys_threadpool_wait_all(sys_threadpool *pool, uint32_t timeout);

void sys_threadpool_cancel_task(sys_threadpool *pool, uint64_t id);
void sys_threadpool_cancel_all(sys_threadpool *pool);

void sys_threadpool_destroy(sys_threadpool *pool);

#ifdef __cplusplus
}
#endif

#endif /* _MIWA_SYSTEM_THREADPOOL_H_ */

