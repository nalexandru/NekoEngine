#include <stdio.h>
#include <stdatomic.h>

#include <Engine/Job.h>
#include <System/Log.h>
#include <Engine/Config.h>
#include <System/System.h>
#include <System/Memory.h>
#include <System/Thread.h>
#include <System/AtomicLock.h>

#define JOBMOD	"JobSystem"

#define ST_WAITING	0
#define ST_RUNNING	1
#define ST_DONE		2

struct NeJob
{
	void *args, *completionArgs;
	NeJobProc exec;
	uint64_t id;
	NeJobCompletedProc completed;
};

struct NeJobQueue
{
	struct NeJob *jobs;
	uint64_t head;
	uint64_t tail;
	uint64_t size;
	NeFutex lock;
};

struct NeDispatchArgs
{
	uint64_t count;
	void **args, *completionArgs;
	NeJobProc exec;
	_Atomic uint64_t *completed;
	uint64_t id, endId, jobCount;
	NeJobCompletedProc completedHandler;
};

static NeConditionVariable _wakeCond;
static NeFutex _wakeLock;
static NeThread *_threads;
static uint32_t _numThreads;
static bool _shutdown;
static struct NeJobQueue _jobQueue;
static NE_ALIGN(16) volatile uint64_t _submittedJobs; 
static THREAD_LOCAL uint32_t _workerId;

static void _ThreadProc(void *args);
static void _DispatchWrapper(int worker, struct NeDispatchArgs *args);
static inline bool _JQ_Push(struct NeJobQueue *jq, NeJobProc exec, void *args, NeJobCompletedProc completed, void *completionArgs, uint64_t *id);

bool
E_InitJobSystem(void)
{
	bool useLogicalCores = E_GetCVarBln("Engine_UseLogicalCores", false)->bln;
	_numThreads = (useLogicalCores ? Sys_CpuThreadCount() : Sys_CpuCount()) - 1;
	_numThreads = _numThreads > 1 ? _numThreads : 1;

	Sys_InitConditionVariable(&_wakeCond);
	Sys_InitFutex(&_wakeLock);

	_jobQueue.head = 0;
	_jobQueue.tail = 0;
	_jobQueue.size = 1000;
	_jobQueue.jobs = Sys_Alloc((size_t)_jobQueue.size, sizeof(*_jobQueue.jobs), MH_System);

	if (!_jobQueue.jobs)
		return false;

	Sys_InitFutex(&_jobQueue.lock);

	_threads = Sys_Alloc(_numThreads, sizeof(NeThread), MH_System);
	if (!_threads)
		return false;

	int step, coreId;
	if (useLogicalCores)
		step = coreId = 1;
	else
		step = coreId = Sys_CpuCount() == Sys_CpuThreadCount() ? 1 : 2;

	for (uint32_t i = 0; i < _numThreads; ++i) {
		char name[10];
		snprintf(name, sizeof(name), "Worker %u", i);
		Sys_InitThread(&_threads[i], name, _ThreadProc, &i);
		Sys_SetThreadAffinity(_threads[i], coreId);
		coreId += step;
	}

	_workerId = _numThreads;

	return true;
}

uint32_t
E_JobWorkerThreads()
{
	return _numThreads;
}

uint32_t
E_WorkerId(void)
{
	return _workerId;
}

uint64_t
E_ExecuteJob(NeJobProc proc, void *args, NeJobCompletedProc completed, void *completionArgs)
{
	uint64_t id;

	while (!_JQ_Push(&_jobQueue, proc, args, completed, completionArgs, &id))
		Sys_Yield();
	Sys_Signal(_wakeCond);

	return id;
}

uint64_t
E_DispatchJobs(uint64_t count, NeJobProc proc, void **args, NeJobCompletedProc completed, void *completionArgs)
{
	int i;
	uint64_t dispatches = 0, id = 0, endId = 0;
	uint64_t jobs = 0, extraJobs = 0, next_start = 0;

	if (count <= _numThreads) {
		dispatches = count;
		jobs = 1;
		extraJobs = 0;
	} else {
		dispatches = _numThreads;
		jobs  = count / _numThreads;
		extraJobs = count - ((uint64_t)jobs * _numThreads);
	}
	
	Sys_LockFutex(_jobQueue.lock);
	{
		id = _submittedJobs;
		_submittedJobs += dispatches;
		endId = _submittedJobs - 1;
	}
	Sys_UnlockFutex(_jobQueue.lock);

	_Atomic uint64_t *completedTasks = Sys_Alloc(sizeof(*completedTasks), 1, MH_Frame);
	*completedTasks = 0;

	for (i = 0; i < dispatches; ++i) {
		struct NeDispatchArgs *dargs = Sys_Alloc(sizeof(*dargs), 1, MH_Frame);

		dargs->exec = proc;
		dargs->count = jobs;
		dargs->args = args ? &(args[next_start]) : NULL;
		dargs->id = id++;
		dargs->endId = endId;
		dargs->jobCount = count;
		dargs->completed = completedTasks;
		dargs->completedHandler = completed;
		dargs->completionArgs = completionArgs;

		if (extraJobs) {
			++dargs->count;
			--extraJobs;
		}

		next_start += dargs->count;

		while(!_JQ_Push(&_jobQueue, (NeJobProc)_DispatchWrapper, dargs, NULL, NULL, NULL))
			Sys_Yield();
	}

	Sys_Broadcast(_wakeCond);

	return endId;
}

void
E_TermJobSystem(void)
{
	_shutdown = true;

	Sys_LockFutex(_jobQueue.lock);
	_jobQueue.head = 0;
	_jobQueue.tail = 0;
	Sys_UnlockFutex(_jobQueue.lock);

	Sys_Broadcast(_wakeCond);

	for (uint32_t i = 0; i < _numThreads; ++i)
		Sys_JoinThread(_threads[i]);

	Sys_TermFutex(_jobQueue.lock);

	Sys_TermConditionVariable(_wakeCond);
	Sys_TermFutex(_wakeLock);

	Sys_Free(_jobQueue.jobs);
	Sys_Free(_threads);
}

static void
_ThreadProc(void *args)
{
	static volatile _Atomic uint32_t workerId = 0;
	uint32_t id = atomic_fetch_add_explicit(&workerId, 1, memory_order_acq_rel);
	struct NeJob job = { 0, 0 };

	_workerId = id;
	Sys_LogEntry(JOBMOD, LOG_INFORMATION, "Worker %d started", _workerId);

	while (!_shutdown) {
		Sys_LockFutex(_wakeLock);

		if (_jobQueue.tail == _jobQueue.head)
			Sys_WaitFutex(_wakeCond, _wakeLock);

		Sys_LockFutex(_jobQueue.lock);
		if (_jobQueue.tail != _jobQueue.head) {
			memcpy(&job, &_jobQueue.jobs[_jobQueue.tail], sizeof(job));
			_jobQueue.tail = (_jobQueue.tail + 1) % _jobQueue.size;
		}
		Sys_UnlockFutex(_jobQueue.lock);

		Sys_UnlockFutex(_wakeLock);

		if (job.exec) {
			job.exec(id, job.args);
			if (job.completed)
				job.completed(job.id, job.completionArgs);

			memset(&job, 0x0, sizeof(job));
		}
	}
}

static void
_DispatchWrapper(int worker, struct NeDispatchArgs *argPtr)
{
	struct NeDispatchArgs args;
	memcpy(&args, argPtr, sizeof(args));

	for (int i = 0; i < args.count; ++i)
		args.exec(worker, args.args ? args.args[i] : NULL);

	uint64_t completed = atomic_fetch_add_explicit(args.completed, args.count, memory_order_acq_rel) + args.count;
	if (completed != args.jobCount || !args.completedHandler)
		return;

	args.completedHandler(args.endId, args.completionArgs);
}

static inline bool
_JQ_Push(struct NeJobQueue *jq, NeJobProc exec, void *args, NeJobCompletedProc completed, void *completionArgs, uint64_t *id)
{
	bool ret = false;
	uint64_t next;

	Sys_LockFutex(jq->lock);

	next = (jq->head + 1) % jq->size;
	if (next != jq->tail) {
		jq->jobs[jq->head].id = _submittedJobs++;
		jq->jobs[jq->head].args = args;
		jq->jobs[jq->head].exec = exec;
		jq->jobs[jq->head].completed = completed;
		jq->jobs[jq->head].completionArgs = completionArgs;

		if (id)
			*id = jq->jobs[jq->head].id;

		jq->head = next;
		
		ret = true;
	}

	Sys_UnlockFutex(jq->lock);

	return ret;
}

/* NekoEngine
 *
 * Job.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
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
 * -----------------------------------------------------------------------------
 */
