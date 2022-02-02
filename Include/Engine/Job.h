#ifndef _NE_ENGINE_JOB_H_
#define _NE_ENGINE_JOB_H_

#include <stdint.h>
#include <stdbool.h>

typedef void (*NeJobProc)(int worker, void *args);
typedef void (*NeJobCompletedProc)(uint64_t id, void *args);

bool E_InitJobSystem(void);

uint32_t E_JobWorkerThreads(void);
uint32_t E_WorkerId(void);

uint64_t E_ExecuteJob(NeJobProc proc, void *args, NeJobCompletedProc completed, void *completionArgs);
uint64_t E_DispatchJobs(uint64_t count, NeJobProc proc, void **args, NeJobCompletedProc completed, void *completionArgs);

void E_TermJobSystem(void);

#endif /* _NE_ENGINE_JOB_H_ */
