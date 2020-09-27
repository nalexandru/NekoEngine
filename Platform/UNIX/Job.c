#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <stdio.h>

#include <sched.h>
#include <pthread.h>

#include <Engine/Job.h>
#include <System/System.h>
#include <System/Memory.h>

#define ST_WAITING	0
#define ST_RUNNING	1
#define ST_DONE		2

#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
#	define STDCATOMIC
#	include <stdatomic.h>
#else
#	error Atomic operations for this platform are not implemented
#endif


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
	pthread_mutex_t lock;
};

struct DispatchArgs
{
	uint64_t count;
	void **args;
	void (*exec)(int worker, void *args);
};

static pthread_cond_t _wakeCond;
static pthread_mutex_t _wakeLock;
static pthread_t *_threads;
static int _numThreads;
static bool _shutdown;
static struct JobQueue _jobQueue;
static uint64_t _submittedJobs;

#ifdef STDCATOMIC
static atomic_uint_fast64_t _finishedJobs;
static atomic_uint_fast32_t _workerId = -1;
#endif

static void *_ThreadProc(void *args);
static void _DispatchWrapper(int worker, struct DispatchArgs *args);
static inline bool _JQ_Push(struct JobQueue *jq, JobProc exec, void *args, void (*completed)(uint64_t));

bool
E_InitJobSystem(void)
{
	_numThreads = Sys_NumCpus() - 2;
	if (_numThreads < 1)
		_numThreads = 1;

	pthread_cond_init(&_wakeCond, NULL);
	pthread_mutex_init(&_wakeLock, NULL);

	_jobQueue.head = 0;
	_jobQueue.tail = 0;
	_jobQueue.size = 1000;
	_jobQueue.jobs = calloc(_jobQueue.size, sizeof(*_jobQueue.jobs));

	if (!_jobQueue.jobs)
		return false;

	pthread_mutex_init(&_jobQueue.lock, NULL);

	_threads = calloc(_numThreads, sizeof(pthread_t));
	if (!_threads)
		return false;

	for (int i = 0; i < _numThreads; ++i) {
		if (!pthread_create(&_threads[i], NULL, _ThreadProc, NULL))
			return false;

	// Thread affinity
	#if defined(__linux__)
		cpu_set_t cpu_set;
		CPU_ZERO(&cpu_set);
		CPU_SET(i, &cpu_set);
		pthread_setaffinity_np(_threads[i], sizeof(cpu_set_t), &cpu_set);
	#elif defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
		thread_affinity_policy_data_t pd = { cpu };
		thread_policy_set(&_threads[i], THREAD_AFFINITY_POLICY, (thread_policy_t)&pd, THREAD_AFFINITY_POLICY_COUNT);
	#else
	#	warning "Thread affinity not implemented for this platform"
	#endif

	// Thread naming
	#if defined(__linux__)
		char name[10];
		snprintf(name, 10, "Worker %d", i);
		pthread_setname_np(_threads[i], name);
	#else
	#	warning "Thread naming not implemented for this platform"
	#endif
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
	++_submittedJobs;

	while (!_JQ_Push(&_jobQueue, proc, args, completed))
		sched_yield();
	pthread_cond_signal(&_wakeCond);

	return _submittedJobs;
}

uint64_t
E_DispatchJobs(uint64_t count, JobProc proc, void **args, void (*completed)(uint64_t))
{
	uint64_t dispatches = 0;
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

	_submittedJobs += dispatches;

	for (int i = 0; i < dispatches; ++i) {
		struct DispatchArgs *dargs = Sys_Alloc(sizeof(*dargs), 1, MH_Transient);

		dargs->exec = proc;
		dargs->count = jobs;
		dargs->args = args ? &(args[next_start]) : NULL;

		if (extraJobs) {
			++dargs->count;
			--extraJobs;
		}

		next_start += dargs->count;

		while(!_JQ_Push(&_jobQueue, (JobProc)_DispatchWrapper, dargs, NULL))
			sched_yield();
	}

	pthread_cond_broadcast(&_wakeCond);

	return _submittedJobs;
}

void
E_TermJobSystem(void)
{
	_shutdown = true;

	pthread_mutex_lock(&_jobQueue.lock);
	_jobQueue.head = 0;
	_jobQueue.tail = 0;
	pthread_mutex_unlock(&_jobQueue.lock);

	pthread_cond_broadcast(&_wakeCond);

	for (int i = 0; i < _numThreads; ++i)
		pthread_join(_threads[i], NULL);

	free(_jobQueue.jobs);
	free(_threads);
}

void *
_ThreadProc(void *args)
{
	int id = atomic_fetch_add(&_workerId, 1);
	struct Job job = { 0, 0 };

	while (!_shutdown) {
		pthread_mutex_lock(&_wakeLock);

		if (_jobQueue.tail == _jobQueue.head)
			pthread_cond_wait(&_wakeCond, &_wakeLock);

		pthread_mutex_lock(&_jobQueue.lock);
		if (_jobQueue.tail != _jobQueue.head) {
			memcpy(&job, &_jobQueue.jobs[_jobQueue.tail], sizeof(job));
			_jobQueue.tail = (_jobQueue.tail + 1) % _jobQueue.size;
		}
		pthread_mutex_unlock(&_jobQueue.lock);

		pthread_mutex_unlock(&_wakeLock);

		if (job.exec) {
			job.exec(id, job.args);
			memset(&job, 0x0, sizeof(job));
			atomic_fetch_add(&_finishedJobs, 1);

			if (job.completed)
				job.completed(job.id);
		}
	}

	pthread_exit(NULL);
}

static void
_DispatchWrapper(int worker, struct DispatchArgs *args)
{
	for (int i = 0; i < args->count; ++i)
		args->exec(worker, args->args ? args->args[i] : NULL);
}

static inline bool
_JQ_Push(struct JobQueue *jq, JobProc exec, void *args, void (*completed)(uint64_t))
{
	bool ret = false;

	pthread_mutex_lock(&jq->lock);

	uint64_t next = (jq->head + 1) % jq->size;
	if (next != jq->tail) {
		jq->jobs[jq->head].args = args;
		jq->jobs[jq->head].exec = exec;
		jq->jobs[jq->head].completed = completed;
		jq->head = next;
		ret = true;
	}

	pthread_mutex_unlock(&jq->lock);

	return ret;
}
