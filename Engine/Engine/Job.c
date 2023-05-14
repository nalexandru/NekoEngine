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

static NeConditionVariable f_wakeCond;
static NeFutex f_wakeLock;
static NeThread *f_threads;
static uint32_t f_numThreads;
static volatile bool f_shutdown;
static struct NeJobQueue f_jobQueue;
static NE_ALIGN(16) volatile uint64_t f_submittedJobs;
static THREAD_LOCAL uint32_t f_workerId;

static void ThreadProc(void *args);
static void DispatchWrapper(int worker, struct NeDispatchArgs *argPtr);
static inline bool JQ_Push(struct NeJobQueue *jq, NeJobProc exec, void *args, NeJobCompletedProc completed, void *completionArgs, uint64_t *id);

bool
E_InitJobSystem(void)
{
	bool useLogicalCores = E_GetCVarBln("Engine_UseLogicalCores", false)->bln;
	f_numThreads = (useLogicalCores ? Sys_CpuThreadCount() : Sys_CpuCount()) - 1;
	f_numThreads = f_numThreads > 1 ? f_numThreads : 1;

	const int maxJobWorkers = CVAR_INT32("Engine_MaxJobWorkers");
	if (maxJobWorkers)
		f_numThreads = maxJobWorkers;

	Sys_InitConditionVariable(&f_wakeCond);
	Sys_InitFutex(&f_wakeLock);

	f_jobQueue.head = 0;
	f_jobQueue.tail = 0;
	f_jobQueue.size = 1000;
	f_jobQueue.jobs = Sys_Alloc((size_t)f_jobQueue.size, sizeof(*f_jobQueue.jobs), MH_System);

	if (!f_jobQueue.jobs)
		return false;

	Sys_InitFutex(&f_jobQueue.lock);

	f_threads = Sys_Alloc(f_numThreads, sizeof(NeThread), MH_System);
	if (!f_threads)
		return false;

	int step, coreId;
	if (useLogicalCores)
		step = coreId = 1;
	else
		step = coreId = Sys_CpuCount() == Sys_CpuThreadCount() ? 1 : 2;

	for (uint32_t i = 0; i < f_numThreads; ++i) {
		char name[10];
		snprintf(name, sizeof(name), "Worker %u", i);
		Sys_InitThread(&f_threads[i], name, ThreadProc, &i);
		Sys_SetThreadAffinity(f_threads[i], coreId);
		coreId += step;
	}

	f_workerId = f_numThreads;

	return true;
}

uint32_t
E_JobWorkerThreads(void)
{
	return f_numThreads;
}

uint32_t
E_WorkerId(void)
{
	return f_workerId;
}

uint64_t
E_ExecuteJob(NeJobProc proc, void *args, NeJobCompletedProc completed, void *completionArgs)
{
	uint64_t id;

	while (!JQ_Push(&f_jobQueue, proc, args, completed, completionArgs, &id))
		Sys_Yield();
	Sys_Signal(f_wakeCond);

	return id;
}

uint64_t
E_DispatchJobs(uint64_t count, NeJobProc proc, void **args, NeJobCompletedProc completed, void *completionArgs)
{
	uint64_t dispatches, id, endId;
	uint64_t jobs, extraJobs, next_start = 0;

	if (count <= f_numThreads) {
		dispatches = count;
		jobs = 1;
		extraJobs = 0;
	} else {
		dispatches = f_numThreads;
		jobs  = count / f_numThreads;
		extraJobs = count - ((uint64_t)jobs * f_numThreads);
	}

	Sys_LockFutex(f_jobQueue.lock);
	{
		id = f_submittedJobs;
		f_submittedJobs += dispatches;
		endId = f_submittedJobs - 1;
	}
	Sys_UnlockFutex(f_jobQueue.lock);

	_Atomic uint64_t *completedTasks = Sys_Alloc(sizeof(*completedTasks), 1, MH_Frame);
	*completedTasks = 0;

	for (int i = 0; i < dispatches; ++i) {
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

		while(!JQ_Push(&f_jobQueue, (NeJobProc)DispatchWrapper, dargs, NULL, NULL, NULL))
			Sys_Yield();
	}

	Sys_Broadcast(f_wakeCond);

	return endId;
}

void
E_TermJobSystem(void)
{
	f_shutdown = true;

	Sys_LockFutex(f_jobQueue.lock);
	f_jobQueue.head = 0;
	f_jobQueue.tail = 0;
	Sys_UnlockFutex(f_jobQueue.lock);

	Sys_Broadcast(f_wakeCond);

	for (uint32_t i = 0; i < f_numThreads; ++i)
		Sys_JoinThread(f_threads[i]);

	Sys_TermFutex(f_jobQueue.lock);

	Sys_TermConditionVariable(f_wakeCond);
	Sys_TermFutex(f_wakeLock);

	Sys_Free(f_jobQueue.jobs);
	Sys_Free(f_threads);
}

static void
ThreadProc(void *args)
{
	static volatile _Atomic uint32_t workerId = 0;
	uint32_t id = atomic_fetch_add_explicit(&workerId, 1, memory_order_acq_rel);
	struct NeJob job = { 0, 0 };

	f_workerId = id;
	Sys_InitMemory();

	Sys_LogEntry(JOBMOD, LOG_INFORMATION, "Worker %d started", f_workerId);

	while (!f_shutdown) {
		Sys_LockFutex(f_wakeLock);

		if (f_jobQueue.tail == f_jobQueue.head)
			Sys_WaitFutex(f_wakeCond, f_wakeLock);

		Sys_LockFutex(f_jobQueue.lock);
		if (f_jobQueue.tail != f_jobQueue.head) {
			memcpy(&job, &f_jobQueue.jobs[f_jobQueue.tail], sizeof(job));
			f_jobQueue.tail = (f_jobQueue.tail + 1) % f_jobQueue.size;
		}
		Sys_UnlockFutex(f_jobQueue.lock);

		Sys_UnlockFutex(f_wakeLock);

		if (job.exec) {
			job.exec(id, job.args);
			if (job.completed)
				job.completed(job.id, job.completionArgs);

			memset(&job, 0x0, sizeof(job));
		}
	}

	Sys_LogEntry(JOBMOD, LOG_INFORMATION, "Worker %d stopped", f_workerId);
	Sys_LogMemoryStatistics();
	Sys_TermMemory();
}

static void
DispatchWrapper(int worker, struct NeDispatchArgs *argPtr)
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
JQ_Push(struct NeJobQueue *jq, NeJobProc exec, void *args, NeJobCompletedProc completed, void *completionArgs, uint64_t *id)
{
	bool ret = false;
	uint64_t next;

	Sys_LockFutex(jq->lock);

	next = (jq->head + 1) % jq->size;
	if (next != jq->tail) {
		jq->jobs[jq->head].id = f_submittedJobs++;
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
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
