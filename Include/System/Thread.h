#ifndef _SYS_THREAD_H_
#define _SYS_THREAD_H_

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

bool Sys_InitThread(Thread *t, const wchar_t *name, void (*proc)(void *), void *args);
void Sys_SetThreadAffinity(Thread t, int cpu);
void Sys_JoinThread(Thread t);
void Sys_TermThread(Thread t);

bool Sys_InitMutex(Mutex *mtx);
bool Sys_LockMutex(Mutex mtx);
bool Sys_UnlockMutex(Mutex mtx);
void Sys_TermMutex(Mutex mtx);

bool Sys_InitFutex(Futex *ftx);
bool Sys_LockFutex(Futex ftx);
bool Sys_UnlockFutex(Futex ftx);
void Sys_TermFutex(Futex ftx);

bool Sys_InitConditionVariable(ConditionVariable *cv);
void Sys_Signal(ConditionVariable cv);
void Sys_Broadcast(ConditionVariable cv);
bool Sys_WaitMutex(ConditionVariable cv, Mutex mtx);
bool Sys_WaitFutex(ConditionVariable cv, Futex ftx);
void Sys_TermConditionVariable(ConditionVariable cv);

#endif /* _SYS_THREAD_H_ */
