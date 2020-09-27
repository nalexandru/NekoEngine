#include <stdio.h>

#include <Windows.h>
#include <VersionHelpers.h>

#include <Engine/Job.h>
#include <System/System.h>
#include <System/Memory.h>

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
	SRWLOCK lock;
};

struct DispatchArgs
{
	uint64_t count;
	void **args;
	void (*exec)(int worker, void *args);
};

static CONDITION_VARIABLE _wakeCond;
static SRWLOCK _wakeLock;
static HANDLE *_threads;
static int _numThreads;
static bool _shutdown;
static struct JobQueue _jobQueue;
static LONG64 _submittedJobs;
static volatile LONG64 _finishedJobs;
static volatile LONG _workerId = -1;

static DWORD WINAPI _ThreadProc(_In_ LPVOID args);
static void _DispatchWrapper(int worker, struct DispatchArgs *args);
static inline bool _JQ_Push(struct JobQueue *jq, JobProc exec, void *args, void (*completed)(uint64_t));

// Thread naming info:
// https://docs.microsoft.com/en-us/visualstudio/debugger/how-to-set-a-thread-name-in-native-code?view=vs-2015&redirectedfrom=MSDN
const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

bool
E_InitJobSystem(void)
{
	_numThreads = Sys_NumCpus() - 2;

	InitializeConditionVariable(&_wakeCond);
	InitializeSRWLock(&_wakeLock);

	_jobQueue.head = 0;
	_jobQueue.tail = 0;
	_jobQueue.size = 1000;
	_jobQueue.jobs = calloc(_jobQueue.size, sizeof(*_jobQueue.jobs));

	if (!_jobQueue.jobs)
		return false;

	InitializeSRWLock(&_jobQueue.lock);

	_threads = calloc(_numThreads, sizeof(HANDLE));
	if (!_threads)
		return false;

	for (int i = 0; i < _numThreads; ++i) {
		DWORD id;
		_threads[i] = CreateThread(NULL, 0, _ThreadProc, &i, 0, &id);
		if (!_threads[i])
			return false;

		SetThreadAffinityMask(_threads[i], (DWORD_PTR)1 << ((DWORD_PTR)i + 2));

		if (IsWindows10OrGreater()) {
			HANDLE k32 = LoadLibrary(L"kernel32");
			if (k32) {
				HRESULT (*k32_SetThreadDescription)(HANDLE, PCWSTR);
				k32_SetThreadDescription = (HRESULT (*)(HANDLE, PCWSTR))GetProcAddress(k32, "SetThreadDescription");

				if (k32_SetThreadDescription) {
					wchar_t wname[10];
				
					swprintf(wname, 10, L"Worker %d", i);
					k32_SetThreadDescription(_threads[i], wname);
				}

				FreeLibrary(k32);
			}
		}

		char name[10];
		
		snprintf(name, 10, "Worker %d", i);
		THREADNAME_INFO info = { 0x1000, name, id, 0 };
		__try {
			RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), (const ULONG_PTR *)&info);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
		}
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
		SwitchToThread();
	WakeConditionVariable(&_wakeCond);

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
		extraJobs = count - ((LONG64)jobs * _numThreads);
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

		while(!_JQ_Push(&_jobQueue, _DispatchWrapper, dargs, NULL))
			SwitchToThread();
	}

	WakeAllConditionVariable(&_wakeCond);

	return _submittedJobs;
}

void
E_TermJobSystem(void)
{
	_shutdown = true;

	AcquireSRWLockExclusive(&_jobQueue.lock);
	_jobQueue.head = 0;
	_jobQueue.tail = 0;
	ReleaseSRWLockExclusive(&_jobQueue.lock);

	WakeAllConditionVariable(&_wakeCond);
	WaitForMultipleObjects(_numThreads, _threads, TRUE, INFINITE);

	free(_jobQueue.jobs);
	free(_threads);
}

static DWORD WINAPI
_ThreadProc(_In_ LPVOID args)
{
	int id = InterlockedIncrement(&_workerId);
	struct Job job = { 0, 0 };

	while (!_shutdown) {
		AcquireSRWLockExclusive(&_wakeLock);

		if (_jobQueue.tail == _jobQueue.head)
			SleepConditionVariableSRW(&_wakeCond, &_wakeLock, INFINITE, 0);

		AcquireSRWLockExclusive(&_jobQueue.lock);
		if (_jobQueue.tail != _jobQueue.head) {
			memcpy(&job, &_jobQueue.jobs[_jobQueue.tail], sizeof(job));
			_jobQueue.tail = (_jobQueue.tail + 1) % _jobQueue.size;
		}
		ReleaseSRWLockExclusive(&_jobQueue.lock);

		ReleaseSRWLockExclusive(&_wakeLock);

		if (job.exec) {
			job.exec(id, job.args);
			memset(&job, 0x0, sizeof(job));
			InterlockedIncrement64(&_finishedJobs);

			if (job.completed)
				job.completed(job.id);
		}
	}

	return 0;
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

	AcquireSRWLockExclusive(&jq->lock);

	uint64_t next = (jq->head + 1) % jq->size;
	if (next != jq->tail) {
		jq->jobs[jq->head].args = args;
		jq->jobs[jq->head].exec = exec;
		jq->jobs[jq->head].completed = completed;
		jq->head = next;
		ret = true;
	}

	ReleaseSRWLockExclusive(&jq->lock);

	return ret;
}
