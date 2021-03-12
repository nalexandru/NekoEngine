#ifndef _WIN32_PLATFORM_H_
#define _WIN32_PLATFORM_H_

#define _WIN32_WINNT	0x0600

#include <Windows.h>

extern UINT WM_SHOWCURSOR;
extern UINT WM_HIDECURSOR;

void UpdateControllers(void);
void HandleInput(HWND wnd, LPARAM lParam, WPARAM wParam);

// Functions added after Windows 7
extern HRESULT (WINAPI *k32_SetThreadDescription)(HANDLE, PCWSTR);

#endif /* _WIN32_PLATFORM_H_ */
