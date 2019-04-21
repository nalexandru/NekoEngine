/* Miwa Portable Runtime
 *
 * threadpool.c
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

#include <string.h>
#include <stdlib.h>

#include <system/log.h>
#include <system/system.h>
#include <system/threadpool.h>
#include <runtime/runtime.h>

#define THREADPOOL_MODULE	"ThreadPool"

typedef struct SYS_TASK
{
	uint64_t id;
	void (*proc)(void *);
	void *args;
	sys_thread *worker;
} sys_task;

typedef struct SYS_WORKER
{
	sys_threadpool *pool;
	bool idle;
	uint32_t id;
	sys_thread *thread;
} sys_worker;

struct SYS_THREADPOOL
{
	rt_array workers;
	rt_array active_tasks;
	rt_queue task_queue;
	sys_mutex *task_mutex;
	sys_cond_var *task_notify;
	uint32_t num_workers;
	uint64_t next_task_id;
	uint32_t worker_key;
	bool idle;
	bool shutdown;
};

void
_threadpool_worker_proc(void *arg)
{
	sys_worker *worker = (sys_worker *)arg;
	sys_task task, *t = NULL;
	
	if (!worker)
		return;
	
	sys_tls_set_value(worker->pool->worker_key, worker);

	while (!worker->pool->shutdown) {
		sys_mutex_lock(worker->pool->task_mutex);

		while (!worker->pool->task_queue.count && !worker->pool->shutdown)
			sys_cond_var_wait(worker->pool->task_notify, worker->pool->task_mutex, 0);

		if (worker->pool->shutdown)
			break;

		t = rt_queue_pop(&worker->pool->task_queue);
		if (!t) {
			sys_mutex_unlock(worker->pool->task_mutex);
			continue;
		}
		memcpy(&task, t, sizeof(task));

		sys_mutex_unlock(worker->pool->task_mutex);

		worker->idle = false;
		task.worker = worker->thread;

		task.proc(task.args);

		worker->idle = true;
	}
}

sys_threadpool *
sys_threadpool_create(uint32_t num_workers)
{
	sys_threadpool *pool = (sys_threadpool *)calloc(1, sizeof(*pool));
	sys_worker *worker = NULL;
	size_t i;

	if (!pool)
		return NULL;

	pool->task_mutex = sys_mutex_create();
	pool->task_notify = sys_cond_var_create();
	
	pool->num_workers = num_workers;
	pool->shutdown = false;

	if (pool->num_workers <= 0) {
		if ((pool->num_workers = sys_cpu_count()) <= 0) {
			log_entry(THREADPOOL_MODULE, LOG_WARNING, "Failed to determine processor count. Limiting server to one thread. To avoid this, specify the number of threads");
			pool->num_workers = 1;
		}
	}

	if (rt_array_init(&pool->workers, pool->num_workers, sizeof(sys_worker)) != SYS_OK)
		goto free_task;

	if (rt_array_init(&pool->active_tasks, pool->num_workers, sizeof(sys_task *)) != SYS_OK)
		goto free_workers;	

	if (rt_queue_init(&pool->task_queue, (size_t)pool->num_workers * 2, sizeof(sys_task)) != SYS_OK)
		goto free_active_tasks;

	pool->worker_key = sys_tls_alloc_key();

	rt_array_fill(&pool->workers);
	for (i = 0; i < pool->num_workers; ++i) {
		worker = rt_array_get(&pool->workers, i);

		worker->id = i;
		worker->idle = true;
		worker->pool = pool;
		
		worker->thread = sys_thread_create(_threadpool_worker_proc);

		sys_thread_start(worker->thread, worker);
		sys_thread_detach(worker->thread);
	}

	return pool;

free_active_tasks:
	rt_array_release(&pool->active_tasks);

free_workers:
	rt_array_release(&pool->workers);

free_task:
	sys_mutex_destroy(pool->task_mutex);
	sys_cond_var_destroy(pool->task_notify);

	free(pool);

	return NULL;
}

bool
sys_threadpool_is_idle(sys_threadpool *pool)
{
	size_t i = 0;
	
	if (!pool)
		return false;

	if (pool->task_queue.count)
		return false;

	for (i = 0; i < pool->workers.count; ++i) {
		if (!((sys_worker *)rt_array_get(&pool->workers, i))->idle)
			return false;
	}

	return true;
}

uint32_t
sys_threadpool_worker_count(sys_threadpool *pool)
{
	if (!pool)
		return 0;
	
	return pool->num_workers;
}

uint32_t
sys_threadpool_worker_id(sys_threadpool *pool)
{
	if (!pool)
		return SYS_TP_MAIN_THREAD;

	sys_worker *w = sys_tls_get_value(pool->worker_key);

	if (!w)
		return SYS_TP_MAIN_THREAD;

	return w->id;
}

int
sys_threadpool_add_task(sys_threadpool *pool,
	void(*task_proc)(void *),
	void *args,
	uint64_t *id)
{
	sys_task task;
	
	if (!pool)
		return SYS_INVALID_ARGS;

	if (pool->shutdown)
		return SYS_TP_SHUTTING_DOWN;

	sys_mutex_lock(pool->task_mutex);

	task.id = pool->next_task_id++;
	task.proc = task_proc;
	task.args = args;
	task.worker = NULL;

	rt_queue_push(&pool->task_queue, &task);

	sys_cond_var_signal(pool->task_notify);

	sys_mutex_unlock(pool->task_mutex);

	if (id)
		*id = task.id;

	return SYS_OK;
}

bool
sys_threadpool_wait_task(sys_threadpool *pool,
	uint64_t id,
	uint32_t timeout)
{
	return false;
}

bool
sys_threadpool_wait_all(sys_threadpool *pool,
	uint32_t timeout)
{
	while (!sys_threadpool_is_idle(pool));

	return true;

	/*size_t i = 0;

	if (!pool)
		return false;
	
	while (pool->task_queue.count);

	while (1) {
		bool working = false;

		for (i = 0; i < pool->workers.count; ++i) {
			if (!((sys_worker *)rt_array_get(&pool->workers, i))->idle)
				working = true;
		}

		if (!working)
			break;
	}

	return true;*/
}

void
sys_threadpool_cancel_task(sys_threadpool *pool,
	uint64_t id)
{

}

void
sys_threadpool_cancel_all(sys_threadpool *pool)
{

}

void
sys_threadpool_destroy(sys_threadpool *pool)
{
	size_t i;

	if (!pool)
		return;
	
	pool->shutdown = true;

	sys_threadpool_wait_all(pool, UINT32_MAX);

	sys_msleep(10);

	sys_tls_free_key(pool->worker_key);

	for (i = 0; i < pool->workers.count; ++i)
		sys_thread_destroy(((sys_worker *)rt_array_get(&pool->workers, i))->thread);

	rt_array_release(&pool->workers);
	rt_array_release(&pool->active_tasks);

	rt_queue_release(&pool->task_queue);

	sys_cond_var_destroy(pool->task_notify);
	sys_mutex_destroy(pool->task_mutex);

	free(pool);
}

