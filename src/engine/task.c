/* NekoEngine
 *
 * task.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Task Manager
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>

#include <system/log.h>
#include <system/mutex.h>
#include <system/config.h>
#include <system/thread.h>
#include <system/thread.h>
#include <system/system.h>
#include <system/cond_var.h>
#include <runtime/runtime.h>

#include <engine/task.h>

#define TASK_MODULE	"TaskScheduler"

struct ne_worker
{
	bool idle;
	uint32_t id;
	sys_thread *thread;
} ne_worker;

struct ne_task
{
	uint64_t id;
	void (*proc)(void *);
	void *args;
	//sys_thread *worker;
};

static uint32_t _worker_id;
static rt_array _workers;
static rt_array _new_tasks, _tasks;
static bool _shutdown = false;
static sys_ilock_int _task_lock = 0,
	_active_workers = 0,
	_current_task = 0,
	_completed_tasks = 0;
static sys_cond_var *_task_notify = NULL;
static sys_mutex *_task_mutex = NULL;

void
_task_worker_proc(void *arg)
{
	struct ne_worker *worker = arg;
	struct ne_task *task = NULL;
	uint32_t task_id = 0;

	if (!worker)
		return;

	sys_thread_set_affinity(worker->thread, worker->id);
	sys_tls_set_value(_worker_id, worker);

	while (!_shutdown) {
		if (!_tasks.count)
			sys_cond_var_wait(_task_notify, _task_mutex, 0);

		if (sys_ilock_inc(&_task_lock) == 1) {
			task_id = sys_ilock_inc(&_current_task);
			sys_ilock_dec(&_task_lock);
		} else {
			/*log_entry(TASK_MODULE, LOG_DEBUG, "Worker %d stalled",
				worker->id);*/
			sys_ilock_dec(&_task_lock);

			continue;
		}

		if (task_id >= _tasks.count) {
			// we are done
			sys_yield();
			continue;
		}

		task = rt_array_get(&_tasks, task_id);

		/*log_entry(TASK_MODULE, LOG_DEBUG, "Worker %d running task %d",
			worker->id, task->id);*/

		sys_ilock_inc(&_active_workers);

		task->proc(task->args);
		task = NULL;

		sys_ilock_inc(&_completed_tasks);

		sys_ilock_dec(&_active_workers);
	}
}

ne_status
task_add(
	void (*proc)(void *),
	void *args)
{
	struct ne_task t;
	t.id = _new_tasks.count;
	t.proc = proc;
	t.args = args;

	return rt_array_add(&_new_tasks, &t) == SYS_OK ? NE_OK : NE_FAIL;
}

void
task_start(void)
{
	memcpy(&_tasks, &_new_tasks, sizeof(_tasks));
	_new_tasks.count = 0;
	_current_task = -1;
	_completed_tasks = 0;

	sys_cond_var_broadcast(_task_notify);
}

void
task_wait(void)
{
	while (_completed_tasks != _tasks.count)
		sys_yield();
}

void
task_execute(void)
{
	task_start();
	task_wait();
}

uint32_t
task_num_workers(void)
{
	return _workers.count;
}

uint32_t
task_worker_id(void)
{
	struct ne_worker *w = sys_tls_get_value(_worker_id);

	if (!w)
		return ENGINE_MAIN_THREAD;

	return w->id;
}

ne_status
task_init(void)
{
	int32_t worker_count = sys_cpu_count() - 2;
	if (worker_count <= 0)
		worker_count = 1;

	if (sys_config_get_bool("dbg_force_single_thread", false)) {
		log_entry(TASK_MODULE, LOG_WARNING,
			"Forced in single thread mode by configuration file");
		worker_count = 1;
	}

	log_entry(TASK_MODULE, LOG_DEBUG,
		"Initializing scheduler with %d workers", worker_count);

	if (rt_array_init(&_workers, worker_count,
		sizeof(struct ne_worker)) != SYS_OK)
		return NE_FAIL;

	if (rt_array_init(&_new_tasks, 100,
		sizeof(struct ne_task)) != SYS_OK)
		return NE_FAIL;

	_worker_id = sys_tls_alloc_key();

	rt_array_fill(&_workers);
	for (uint32_t i = 0; i < _workers.count; ++i) {
		struct ne_worker *worker = rt_array_get(&_workers, i);

		worker->id = i;
		worker->idle = true;

		worker->thread = sys_thread_create(_task_worker_proc);

		sys_thread_start(worker->thread, worker);
		sys_thread_detach(worker->thread);
	}

	_task_mutex = sys_mutex_create();
	_task_notify = sys_cond_var_create();

	return NE_OK;
}

void
task_release(void)
{
	_shutdown = true;

	task_wait();

	sys_tls_free_key(_worker_id);

	for (uint32_t i = 0; i < _workers.count; ++i)
		sys_thread_destroy(((struct ne_worker *)rt_array_get(&_workers, i))->thread);

	sys_cond_var_destroy(_task_notify);
	sys_mutex_destroy(_task_mutex);

	rt_array_release(&_new_tasks);
	rt_array_release(&_workers);
}
