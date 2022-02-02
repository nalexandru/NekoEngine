#ifndef _NE_SYSTEM_THREAD_H_
#define _NE_SYSTEM_THREAD_H_

#include <Engine/Types.h>

#if defined(_MSC_VER)
#	define THREAD_LOCAL	__declspec(thread)
#elif defined(__GNUC__)
#	define THREAD_LOCAL	__thread
#endif

uint32_t Sys_TlsAlloc(void);
void *Sys_TlsGet(uint32_t key);
void Sys_TlsSet(uint32_t key, void *data);
void Sys_TlsFree(uint32_t key);

void Sys_Yield(void);

bool Sys_InitThread(NeThread *t, const char *name, void (*proc)(void *), void *args);
void Sys_SetThreadAffinity(NeThread t, int cpu);
void Sys_JoinThread(NeThread t);
void Sys_TermThread(NeThread t);

bool Sys_InitMutex(NeMutex *mtx);
bool Sys_LockMutex(NeMutex mtx);
bool Sys_UnlockMutex(NeMutex mtx);
void Sys_TermMutex(NeMutex mtx);

bool Sys_InitFutex(NeFutex *ftx);
bool Sys_LockFutex(NeFutex ftx);
bool Sys_UnlockFutex(NeFutex ftx);
void Sys_TermFutex(NeFutex ftx);

bool Sys_InitConditionVariable(NeConditionVariable *cv);
void Sys_Signal(NeConditionVariable cv);
void Sys_Broadcast(NeConditionVariable cv);
bool Sys_WaitMutex(NeConditionVariable cv, NeMutex mtx);
bool Sys_WaitFutex(NeConditionVariable cv, NeFutex ftx);
void Sys_TermConditionVariable(NeConditionVariable cv);

#endif /* _NE_SYSTEM_THREAD_H_ */
