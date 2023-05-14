#ifndef NE_WIN32_PLATFORM_H
#define NE_WIN32_PLATFORM_H

#undef _WIN32_WINNT
#define _WIN32_WINNT	0x0600

#include <winsock2.h>
#include <windows.h>
#include <shlobj.h>
#include <xinput.h>

#include <Input/Codes.h>
#include <Engine/Types.h>
#include <System/PlatformDetect.h>

ENGINE_API extern HINSTANCE Win32_instance;
ENGINE_API extern UINT WM_SHOWCURSOR;
ENGINE_API extern UINT WM_HIDECURSOR;

extern enum NeButton Win32_keymap[256];
wchar_t *NeWin32_UTF8toUCS2(const char *text);
char *NeWin32_UCS2toUTF8(const wchar_t *text);

void UpdateControllers(void);
void HandleInput(HWND wnd, LPARAM lParam, WPARAM wParam);

// NTAPI Undocumented Functions (http://undocumented.ntinternals.net/)
typedef enum _OBJECT_WAIT_TYPE
{
	WaitAllObject,
	WaitAnyObject
} OBJECT_WAIT_TYPE, *POBJECT_WAIT_TYPE;

NTSYSAPI NTSTATUS NTAPI NtQueryPerformanceCounter(OUT PLARGE_INTEGER PerformanceCounter, OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL);
NTSYSAPI NTSTATUS NTAPI NtDelayExecution(IN BOOLEAN Alertable, IN PLARGE_INTEGER DelayInterval);
NTSYSAPI NTSTATUS NTAPI NtYieldExecution(VOID);
NTSYSAPI NTSTATUS NTAPI NtWaitForSingleObject(IN HANDLE ObjectHandle, IN BOOLEAN Alertable, IN PLARGE_INTEGER Timeout OPTIONAL);
NTSYSAPI NTSTATUS NTAPI NtWaitForMultipleObjects(IN ULONG ObjectCount, IN PHANDLE ObjectsArray, IN OBJECT_WAIT_TYPE WaitType, IN BOOLEAN Alertable, IN PLARGE_INTEGER Timeout OPTIONAL);

// Windows XP
extern BOOL (WINAPI *k32_AttachConsole)(DWORD);
extern void (WINAPI *k32_GetNativeSystemInfo)(LPSYSTEM_INFO);

// Windows XP with SP3 or Windows XP x64
extern BOOL (WINAPI *k32_GetLogicalProcessorInformation)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

// Windows Vista
extern HRESULT (WINAPI *dwmapi_DwmSetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD);

// Windows 10, version 1607
extern HRESULT (WINAPI *k32_SetThreadDescription)(HANDLE, PCWSTR);

#if defined(SYS_PLATFORM_MINGW) && !defined(NE_MINGW_NO_NT5_SUPPORT)
#	define NE_NT5_SUPPORT
#endif

#ifndef NE_NT5_SUPPORT
#	define Win32_SHGetKnownFolderPath SHGetKnownFolderPath

#	define Win32_RegisterRawInputDevices RegisterRawInputDevices
#	define Win32_GetRawInputData GetRawInputData
#	define Win32_DefRawInputProc DefRawInputProc

#	define Win32_InitializeSRWLock InitializeSRWLock
#	define Win32_AcquireSRWLockExclusive AcquireSRWLockExclusive
#	define Win32_ReleaseSRWLockExclusive ReleaseSRWLockExclusive

#	define Win32_InitializeConditionVariable InitializeConditionVariable
#	define Win32_WakeConditionVariable WakeConditionVariable
#	define Win32_WakeAllConditionVariable WakeAllConditionVariable
#	define Win32_SleepConditionVariableCS SleepConditionVariableCS
#	define Win32_SleepConditionVariableSRW SleepConditionVariableSRW

#	define Win32_XInputGetState XInputGetState
#else
	struct Win32c_ConditionVariable
	{
		HANDLE evt[2];
		unsigned wait;
		CRITICAL_SECTION lock;
	};

	// Windows XP
	extern BOOL (WINAPI *u32_RegisterRawInputDevices)(PCRAWINPUTDEVICE, UINT, UINT);
	extern UINT (WINAPI *u32_GetRawInputData)(HRAWINPUT, UINT, LPVOID, PUINT, UINT);
	extern LRESULT (WINAPI *u32_DefRawInputProc)(PRAWINPUT *, INT, UINT);

	// Windows Vista
	extern HRESULT (WINAPI *s32_SHGetKnownFolderPath)(REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR *);

	extern BOOL (WINAPI *k32_CancelIoEx)(HANDLE, LPOVERLAPPED);

	extern void (WINAPI *k32_InitializeSRWLock)(PSRWLOCK);
	extern void (WINAPI *k32_AcquireSRWLockExclusive)(PSRWLOCK);
	extern void (WINAPI *k32_ReleaseSRWLockExclusive)(PSRWLOCK);

	extern void (WINAPI *k32_InitializeConditionVariable)(PCONDITION_VARIABLE);
	extern void (WINAPI *k32_WakeConditionVariable)(PCONDITION_VARIABLE);
	extern void (WINAPI *k32_WakeAllConditionVariable)(PCONDITION_VARIABLE);
	extern BOOL (WINAPI *k32_SleepConditionVariableCS)(PCONDITION_VARIABLE, PCRITICAL_SECTION, DWORD);
	extern BOOL (WINAPI *k32_SleepConditionVariableSRW)(PCONDITION_VARIABLE, PSRWLOCK, DWORD, ULONG);

	// XInput2 - not present on Windows 2000
	extern DWORD (WINAPI *xi2_XInputGetState)(DWORD dwUserIndex, XINPUT_STATE *);

	void WINAPI Win32c_InitializeConditionVariable(PCONDITION_VARIABLE cv);
	BOOL WINAPI Win32c_SleepConditionVariableCS(PCONDITION_VARIABLE cv, PCRITICAL_SECTION cs, DWORD ms);
	BOOL WINAPI Win32c_SleepConditionVariableSRW(PCONDITION_VARIABLE cv, PSRWLOCK srw, DWORD ms, ULONG flags);
	void WINAPI Win32c_WakeConditionVariable(PCONDITION_VARIABLE cv);
	void WINAPI Win32c_WakeAllConditionVariable(PCONDITION_VARIABLE cv);
	void WINAPI Win32c_DeleteConditionVariable(PCONDITION_VARIABLE cv);

	HRESULT WINAPI Win32c_SHGetKnownFolderPath(REFKNOWNFOLDERID rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath);

#	define Win32_SHGetKnownFolderPath s32_SHGetKnownFolderPath

#	define Win32_RegisterRawInputDevices u32_RegisterRawInputDevices
#	define Win32_GetRawInputData u32_GetRawInputData
#	define Win32_DefRawInputProc u32_DefRawInputProc

#	define Win32_InitializeSRWLock k32_InitializeSRWLock
#	define Win32_AcquireSRWLockExclusive k32_AcquireSRWLockExclusive
#	define Win32_ReleaseSRWLockExclusive k32_ReleaseSRWLockExclusive

#	define Win32_InitializeConditionVariable k32_InitializeConditionVariable
#	define Win32_WakeConditionVariable k32_WakeConditionVariable
#	define Win32_WakeAllConditionVariable k32_WakeAllConditionVariable
#	define Win32_SleepConditionVariableCS k32_SleepConditionVariableCS
#	define Win32_SleepConditionVariableSRW k32_SleepConditionVariableSRW

#	define Win32_XInputGetState xi2_XInputGetState
#endif

#endif /* NE_WIN32_PLATFORM_H */

/* NekoEngine
 *
 * Win32Platform.h
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
