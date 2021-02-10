#include <stdio.h>
#include <stdatomic.h>

#include <Engine/Job.h>
#include <System/System.h>
#include <System/Memory.h>
#include <System/Thread.h>
#include <System/AtomicLock.h>

#define ST_WAITING	0
#define ST_RUNNING	1
#define ST_DONE		2

struct Job
{
	void *args;
	void (*exec)(int worker, void *args);
	uint64_t id;
	void (*completed)(uint64_t);
};

struct JobQueue
{
	struct Job *jobs;
	uint64_t head;
	uint64_t tail;
	uint64_t size;
	Futex lock;
};

struct DispatchArgs
{
	uint64_t count;
	void **args;
	void (*exec)(int worker, void *args);
};

static ConditionVariable _wakeCond;
static Futex _wakeLock;
static Thread *_threads;
static int _numThreads;
static bool _shutdown;
static struct JobQueue _jobQueue;
static ALIGN(16) volatile uint64_t _submittedJobs; 

static void _ThreadProc(void *args);
static void _DispatchWrapper(int worker, struct DispatchArgs *args);
static inline bool _JQ_Push(struct JobQueue *jq, JobProc exec, void *args, void (*completed)(uint64_t), uint64_t *id);

bool
E_InitJobSystem(void)
{
	int i = 0;
	wchar_t name[10];
	
	_numThreads = Sys_NumCpus() - 2;
	_numThreads = _numThreads > 1 ? _numThreads : 1;

	Sys_InitConditionVariable(&_wakeCond);
	Sys_InitFutex(&_wakeLock);

	_jobQueue.head = 0;
	_jobQueue.tail = 0;
	_jobQueue.size = 1000;
	_jobQueue.jobs = calloc((size_t)_jobQueue.size, sizeof(*_jobQueue.jobs));

	if (!_jobQueue.jobs)
		return false;

	Sys_InitFutex(&_jobQueue.lock);

	_threads = calloc(_numThreads, sizeof(Thread));
	if (!_threads)
		return false;

	for (i = 0; i < _numThreads; ++i) {
		swprintf(name, 10, L"Worker %d", i);
		Sys_InitThread(&_threads[i], name, _ThreadProc, &i);
		Sys_SetThreadAffinity(_threads[i], i + 2);
	}

	return true;
}

int
E_JobWorkerThreads()
{
	return _numThreads;
}

uint64_t
E_ExecuteJob(JobProc proc, void *args, void (*completed)(uint64_t))
{
	uint64_t id;

	while (!_JQ_Push(&_jobQueue, proc, args, completed, &id))
		Sys_Yield();
	Sys_Signal(_wakeCond);

	return id;
}

uint64_t
E_DispatchJobs(uint64_t count, JobProc proc, void **args, void (*completed)(uint64_t))
{
	int i;
	uint64_t dispatches = 0, id = 0;
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
	
	for (i = 0; i < dispatches; ++i) {
		struct DispatchArgs *dargs = Sys_Alloc(sizeof(*dargs), 1, MH_Transient);

		dargs->exec = proc;
		dargs->count = jobs;
		dargs->args = args ? &(args[next_start]) : NULL;

		if (extraJobs) {
			++dargs->count;
			--extraJobs;
		}

		next_start += dargs->count;

		while(!_JQ_Push(&_jobQueue, (JobProc)_DispatchWrapper, dargs, NULL, &id))
			Sys_Yield();
	}

	Sys_Broadcast(_wakeCond);

	return id;
}

void
E_TermJobSystem(void)
{
	int i = 0;

	_shutdown = true;

	Sys_LockFutex(_jobQueue.lock);
	_jobQueue.head = 0;
	_jobQueue.tail = 0;
	Sys_UnlockFutex(_jobQueue.lock);

	Sys_Broadcast(_wakeCond);

	for (i = 0; i < _numThreads; ++i)
		Sys_JoinThread(_threads[i]);

	Sys_TermFutex(_jobQueue.lock);

	Sys_TermConditionVariable(_wakeCond);
	Sys_TermFutex(_wakeLock);

	free(_jobQueue.jobs);
	free(_threads);
}

static void
_ThreadProc(void *args)
{
	static _Atomic int32_t workerId = -1;
	int32_t id = atomic_fetch_add(&workerId, 1);
	struct Job job = { 0, 0 };

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
			memset(&job, 0x0, sizeof(job));

			if (job.completed)
				job.completed(job.id);
		}
	}
}

static void
_DispatchWrapper(int worker, struct DispatchArgs *args)
{
	for (int i = 0; i < args->count; ++i)
		args->exec(worker, args->args ? args->args[i] : NULL);
}

static inline bool
_JQ_Push(struct JobQueue *jq, JobProc exec, void *args, void (*completed)(uint64_t), uint64_t *id)
{
	bool ret = false;
	uint64_t next;

	Sys_LockFutex(jq->lock);

	next = (jq->head + 1) % jq->size;
	if (next != jq->tail) {
		jq->jobs[jq->head].id = _submittedJobs;
		jq->jobs[jq->head].args = args;
		jq->jobs[jq->head].exec = exec;
		jq->jobs[jq->head].completed = completed;
		jq->head = next;
		
		ret = true;
		*id = _submittedJobs++;
	}

	Sys_UnlockFutex(jq->lock);

	return ret;
}
