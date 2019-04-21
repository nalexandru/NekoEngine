/* Miwa Portable Runtime
 *
 * thread.h
 * Author: Alexandru Naiman
 *
 * Thread
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

#ifndef _MIWA_SYSTEM_THREAD_H_
#define _MIWA_SYSTEM_THREAD_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SYS_THREAD sys_thread;
typedef int32_t sys_ilock_int;

sys_thread *sys_thread_create(void(*thread_proc)(void *));

void sys_thread_set_proc(sys_thread *thread, void(*thread_proc)(void *));

bool sys_thread_is_running(sys_thread *thread);

bool sys_thread_start(sys_thread *thread, void *args);
bool sys_thread_join(sys_thread *thread);
bool sys_thread_timed_join(sys_thread *thread, int32_t timeout);
bool sys_thread_detach(sys_thread *thread);
bool sys_thread_abort(sys_thread *thread);

bool sys_thread_set_affinity(sys_thread *thread, int cpu);

uint32_t sys_tls_alloc_key(void);
void *sys_tls_get_value(uint32_t key);
void sys_tls_set_value(uint32_t key, void *value);
void sys_tls_free_key(uint32_t key);

void sys_thread_destroy(sys_thread *thread);

void sys_yield(void);

sys_ilock_int sys_ilock_inc(sys_ilock_int *v);
sys_ilock_int sys_ilock_dec(sys_ilock_int *v);
sys_ilock_int sys_ilock_add(sys_ilock_int *v, sys_ilock_int i);
sys_ilock_int sys_ilock_sub(sys_ilock_int *v, sys_ilock_int i);
sys_ilock_int sys_ilock_swap(sys_ilock_int *v, sys_ilock_int i);
sys_ilock_int sys_ilock_compare(sys_ilock_int *v, sys_ilock_int c, sys_ilock_int i);

#ifdef __cplusplus
}
#endif

#endif /* _MIWA_SYSTEM_THREAD_H_ */

