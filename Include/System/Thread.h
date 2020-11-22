#ifndef _SYS_THREAD_H_
#define _SYS_THREAD_H_

#include <Engine/Types.h>

#ifdef __cplusplus
extern "C"  {
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

int32_t Sys_AtomicAdd(volatile int32_t *i, int32_t v);
int32_t Sys_AtomicSub(volatile int32_t *i, int32_t v);
int32_t Sys_AtomicCompareAndSwap(volatile int32_t *i, int32_t e, int32_t c);
int32_t Sys_AtomicIncrement(volatile int32_t *i);
int32_t Sys_AtomicDecrement(volatile int32_t *i);

int64_t Sys_AtomicAdd64(volatile int64_t *i, int64_t v);
int64_t Sys_AtomicSub64(volatile int64_t *i, int64_t v);
int64_t Sys_AtomicCompareAndSwap64(volatile int64_t *i, int64_t e, int64_t c);
int64_t Sys_AtomicIncrement64(volatile int64_t *i);
int64_t Sys_AtomicDecrement64(volatile int64_t *i);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_THREAD_H_ */
