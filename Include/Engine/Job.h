#ifndef _E_JOB_H_
#define _E_JOB_H_

#include <stdint.h>
#include <stdbool.h>

typedef void (*JobProc)(int, void *);

bool E_InitJobSystem(void);

int E_JobWorkerThreads(void);

uint64_t E_ExecuteJob(JobProc proc, void *args, void (*completed)(uint64_t));
uint64_t E_DispatchJobs(uint64_t count, JobProc proc, void **args, void (*completed)(uint64_t));

void E_TermJobSystem(void);

#endif /* _E_JOB_H_ */
