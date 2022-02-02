#ifndef _WIN32_PLATFORM_H_
#define _WIN32_PLATFORM_H_

#define _WIN32_WINNT	0x0600

#include <Windows.h>
#include <Engine/Types.h>

ENGINE_API extern HINSTANCE Win32_instance;
ENGINE_API extern UINT WM_SHOWCURSOR;
ENGINE_API extern UINT WM_HIDECURSOR;

wchar_t *NeWin32_UTF8toUCS2(const char *text);

void UpdateControllers(void);
void HandleInput(HWND wnd, LPARAM lParam, WPARAM wParam);

// NTAPI Undocumented Functions
NTSYSAPI NTSTATUS NTAPI NtQueryPerformanceCounter(OUT PLARGE_INTEGER PerformanceCounter, OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL);
NTSYSAPI NTSTATUS NTAPI NtDelayExecution(IN BOOLEAN Alertable, IN PLARGE_INTEGER DelayInterval);

// Functions added after Windows 7
extern HRESULT (WINAPI *k32_SetThreadDescription)(HANDLE, PCWSTR);

#endif /* _WIN32_PLATFORM_H_ */
