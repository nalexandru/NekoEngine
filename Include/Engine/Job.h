#ifndef _E_JOB_H_
#define _E_JOB_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*JobProc)(int, void *);

bool E_InitJobSystem(void);

int E_JobWorkerThreads(void);

uint64_t E_ExecuteJob(JobProc proc, void *args, void (*completed)(uint64_t));
uint64_t E_DispatchJobs(uint64_t count, JobProc proc, void **args, void (*completed)(uint64_t));

void E_TermJobSystem(void);

#ifdef __cplusplus
}
#endif

#endif /* _E_JOB_H_ */