#include <stdbool.h>

#include "Win32Platform.h"

/*
 * Condition variable implementation taken from GLFW 2.7.6
 * https://github.com/openglsuperbible/sb6code/blob/master/extern/glfw-2.7.6/lib/win32/win32_thread.c
 */

#define EVT_SIGNAL		0
#define EVT_BROADCAST	1

void WINAPI
win32Compat_InitializeConditionVariable(PCONDITION_VARIABLE cv)
{
	struct Win32CompatCV *v = (struct Win32CompatCV *)cv;
 
	v->evt[EVT_SIGNAL] = CreateEvent(NULL, FALSE, FALSE, NULL);
	v->evt[EVT_BROADCAST] = CreateEvent(NULL, TRUE, FALSE, NULL);
 
	InitializeCriticalSection(&v->waitLock);
}

BOOL WINAPI
win32Compat_SleepConditionVariableSRW(PCONDITION_VARIABLE cv, PSRWLOCK srw, DWORD ms, ULONG flags)
{
	(void)flags;
	return win32Compat_SleepConditionVariableCS(cv, (PCRITICAL_SECTION)srw, ms);
}

BOOL WINAPI
win32Compat_SleepConditionVariableCS(PCONDITION_VARIABLE cv, PCRITICAL_SECTION cs, DWORD ms)
{
	int res, lastWait;
	bool ret = true;
	struct Win32CompatCV *v = (struct Win32CompatCV *)cv;

	if (!cv || !cs)
		return false;

	EnterCriticalSection(&v->waitLock);
	++v->wait;
	LeaveCriticalSection(&v->waitLock);

	LeaveCriticalSection(cs);

	res = WaitForMultipleObjects(2, v->evt, FALSE, ms);
	ret = (res != WAIT_TIMEOUT && res != WAIT_FAILED);

	EnterCriticalSection(&v->waitLock);
	--v->wait;
	lastWait = (res == WAIT_OBJECT_0 + EVT_BROADCAST) && (v->wait == 0);
	LeaveCriticalSection(&v->waitLock);
 
	if (lastWait)
	ResetEvent(v->evt[EVT_BROADCAST]);

	EnterCriticalSection(cs);
 
    return ret;
}

void WINAPI
win32Compat_WakeConditionVariable(PCONDITION_VARIABLE cv)
{
	int set;
	struct Win32CompatCV *v = (struct Win32CompatCV *)cv;
 
	if (!cv)
		return;
	
	EnterCriticalSection(&v->waitLock);
	set = v->wait > 0;
	LeaveCriticalSection(&v->waitLock);
	
	if (set)
		SetEvent(v->evt[EVT_SIGNAL]);
}

void WINAPI
win32Compat_WakeAllConditionVariable(PCONDITION_VARIABLE cv)
{
	int set;
	struct Win32CompatCV *v = (struct Win32CompatCV *)cv;
 
	if (!cv)
		return;
 
	EnterCriticalSection(&v->waitLock);
	set = v->wait > 0;
	LeaveCriticalSection(&v->waitLock);

	if (set)
		SetEvent(v->evt[EVT_BROADCAST]);
}

void WINAPI
win32Compat_DeleteConditionVariable(PCONDITION_VARIABLE cv)
{
    struct Win32CompatCV *v = (struct Win32CompatCV *)cv;
 
    if (!cv)
        return;
 
    CloseHandle(v->evt[EVT_SIGNAL]);
    CloseHandle(v->evt[EVT_BROADCAST]);
 
    DeleteCriticalSection(&v->waitLock);
}

