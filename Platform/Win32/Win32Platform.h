#ifndef _WIN32_PLATFORM_H_
#define _WIN32_PLATFORM_H_

#define _WIN32_WINNT	0x0600

#include <Windows.h>

extern HINSTANCE Win32_instance;
extern UINT WM_SHOWCURSOR;
extern UINT WM_HIDECURSOR;

void UpdateControllers(void);
void HandleInput(HWND wnd, LPARAM lParam, WPARAM wParam);

// NTAPI Undocumented Functions
NTSYSAPI NTSTATUS NTAPI NtQueryPerformanceCounter(OUT PLARGE_INTEGER PerformanceCounter, OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL);
NTSYSAPI NTSTATUS NTAPI NtDelayExecution(IN BOOLEAN Alertable, IN PLARGE_INTEGER DelayInterval);

// Functions added after Windows 7
extern HRESULT (WINAPI *k32_SetThreadDescription)(HANDLE, PCWSTR);

#endif /* _WIN32_PLATFORM_H_ */
