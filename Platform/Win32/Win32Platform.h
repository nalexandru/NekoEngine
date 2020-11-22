#ifndef _WIN32_PLATFORM_H_
#define _WIN32_PLATFORM_H_

#include <Windows.h>

extern UINT WM_SHOWCURSOR;
extern UINT WM_HIDECURSOR;

struct Win32CompatCV
{
	HANDLE evt[2];
	unsigned wait;
	CRITICAL_SECTION waitLock;
};

#if WINVER < 0x0600
typedef struct Win32CompatCV CONDITION_VARIABLE;
typedef CONDITION_VARIABLE * PCONDITION_VARIABLE;

typedef int SRWLOCK;
typedef SRWLOCK * PSRWLOCK;

typedef unsigned long ULONG_PTR;
typedef unsigned long DWORD_PTR;
#endif

void UpdateControllers(void);
void HandleInput(HWND wnd, LPARAM lParam, WPARAM wParam);

extern size_t k32_ConditionVariableSize;
extern HRESULT (WINAPI *k32_SetThreadDescription)(HANDLE, PCWSTR);
extern void (WINAPI *k32_InitializeSRWLock)(PSRWLOCK);
extern void (WINAPI *k32_AcquireSRWLockExclusive)(PSRWLOCK);
extern void (WINAPI *k32_ReleaseSRWLockExclusive)(PSRWLOCK);
extern BOOL (WINAPI *k32_SleepConditionVariableSRW)(PCONDITION_VARIABLE, PSRWLOCK, DWORD, ULONG);
extern BOOL (WINAPI *k32_SleepConditionVariableCS)(PCONDITION_VARIABLE, PCRITICAL_SECTION, DWORD);
extern void (WINAPI *k32_WakeAllConditionVariable)(PCONDITION_VARIABLE);
extern void (WINAPI *k32_WakeConditionVariable)(PCONDITION_VARIABLE);
extern void (WINAPI *k32_InitializeConditionVariable)(PCONDITION_VARIABLE);
extern void (WINAPI *k32_DeleteConditionVariable)(PCONDITION_VARIABLE);

// Compatibility shivs
void WINAPI win32Compat_InitializeConditionVariable(PCONDITION_VARIABLE cv);
BOOL WINAPI win32Compat_SleepConditionVariableSRW(PCONDITION_VARIABLE cv, PSRWLOCK srw, DWORD ms, ULONG flags);
BOOL WINAPI win32Compat_SleepConditionVariableCS(PCONDITION_VARIABLE cv, PCRITICAL_SECTION cs, DWORD ms);
void WINAPI win32Compat_WakeConditionVariable(PCONDITION_VARIABLE cv);
void WINAPI win32Compat_WakeAllConditionVariable(PCONDITION_VARIABLE cv);
void WINAPI win32Compat_DeleteConditionVariable(PCONDITION_VARIABLE cv);

#ifndef PROCESSOR_ARCHITECTURE_SHX
#define PROCESSOR_ARCHITECTURE_SHX				4
#endif

#ifndef PROCESSOR_ARCHITECTURE_ARM
#define PROCESSOR_ARCHITECTURE_ARM				5
#endif

#ifndef PROCESSOR_ARCHITECTURE_IA64
#define PROCESSOR_ARCHITECTURE_IA64				6
#endif

#ifndef PROCESSOR_ARCHITECTURE_ALPHA64
#define PROCESSOR_ARCHITECTURE_ALPHA64			7
#endif

#ifndef PROCESSOR_ARCHITECTURE_MSIL
#define PROCESSOR_ARCHITECTURE_MSIL				8
#endif

#ifndef PROCESSOR_ARCHITECTURE_AMD64
#define PROCESSOR_ARCHITECTURE_AMD64			9
#endif

#ifndef PROCESSOR_ARCHITECTURE_IA32_ON_WIN64
#define PROCESSOR_ARCHITECTURE_IA32_ON_WIN64	10
#endif

#ifndef PROCESSOR_ARCHITECTURE_NEUTRAL
#define PROCESSOR_ARCHITECTURE_NEUTRAL			11
#endif

#ifndef PROCESSOR_ARCHITECTURE_ARM64
#define PROCESSOR_ARCHITECTURE_ARM64			12
#endif

#ifndef PROCESSOR_ARCHITECTURE_ARM32_ON_WIN64
#define PROCESSOR_ARCHITECTURE_ARM32_ON_WIN64	13
#endif

#ifndef PROCESSOR_ARCHITECTURE_IA32_ON_ARM64
#define PROCESSOR_ARCHITECTURE_IA32_ON_ARM64	14
#endif

#endif /* _WIN32_PLATFORM_H_ */
