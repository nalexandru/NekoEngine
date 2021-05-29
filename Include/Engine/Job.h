#ifndef _NE_ENGINE_JOB_H_
#define _NE_ENGINE_JOB_H_

#include <stdint.h>
#include <stdbool.h>

typedef void (*JobProc)(int worker, void *args);
typedef void (*JobCompletedProc)(uint64_t id);

bool E_InitJobSystem(void);

uint32_t E_JobWorkerThreads(void);
uint32_t E_WorkerId(void);

uint64_t E_ExecuteJob(JobProc proc, void *args, JobCompletedProc completed);
uint64_t E_DispatchJobs(uint64_t count, JobProc proc, void **args, JobCompletedProc completed);

void E_TermJobSystem(void);

#endif /* _NE_ENGINE_JOB_H_ */
