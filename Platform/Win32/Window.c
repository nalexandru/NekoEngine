#include <stdint.h>
#include <stdbool.h>

#include "Win32Platform.h"
#include <Dbt.h>

#include <System/Window.h>
#include <Engine/Engine.h>
#include <Render/Render.h>

#include "Win32Platform.h"

static HWND _window;

#define WND_CLASS_NAME				L"NekoEngineWindowClass"
#define WM_SHOWCURSOR_MSG_GUID		L"NE_WM_SHOWCURSOR_{916fcbf2-b4be-4df8-884b-f0dc086e03ad}"
#define WM_HIDECURSOR_MSG_GUID		L"NE_WM_HIDECURSOR_{916fcbf2-b4be-4df8-884b-f0dc086e03ad}"

__declspec(dllexport) UINT WM_SHOWCURSOR = 0;
__declspec(dllexport) UINT WM_HIDECURSOR = 0;

static LRESULT CALLBACK
_WndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg) {
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
	} break;
	case WM_ERASEBKGND: {
		return 0; // ignore
	}
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	case WM_SYSCOMMAND: {
		// Prevent monitor from turning off & screen saver from starting
		if (wparam == SC_SCREENSAVE || wparam == SC_MONITORPOWER)
			return 0;
		else if (wparam == SC_RESTORE)
			ShowWindow(hwnd, SW_RESTORE); // Required to show a minimized fullscreen window
	} break;
	case WM_DEVICECHANGE: {
		if (wparam != DBT_DEVNODES_CHANGED)
			break;

		UpdateControllers();
	} break;
	case WM_SIZE: {
		if (*E_screenWidth != LOWORD(lparam) || *E_screenHeight != HIWORD(lparam))
			E_ScreenResized(LOWORD(lparam), HIWORD(lparam));
	} break;
	default: {
		if (umsg == WM_SHOWCURSOR) {
			ShowCursor(true);
			return 0;
		} else if (umsg == WM_HIDECURSOR) {
			ShowCursor(false);
			return 0;
		}
	} break;
	}
	
	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

bool
Sys_CreateWindow(void)
{
	RECT rc;
	DWORD style = WS_VISIBLE | (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME) | WS_CLIPCHILDREN;
	DWORD exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	uint16_t x = 0, y = 0;

	WNDCLASSW wincl = { 0 };
	wincl.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wincl.lpfnWndProc = _WndProc;
	wincl.hInstance = Win32_instance;
	wincl.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wincl.lpszClassName = WND_CLASS_NAME;
	wincl.hIcon = LoadIcon(Win32_instance, MAKEINTRESOURCE(300));

	if (!WM_SHOWCURSOR)
		WM_SHOWCURSOR = RegisterWindowMessageW(WM_SHOWCURSOR_MSG_GUID);

	if (!WM_HIDECURSOR)
		WM_HIDECURSOR = RegisterWindowMessageW(WM_HIDECURSOR_MSG_GUID);

	if (!RegisterClassW(&wincl)) {
		MessageBoxW(HWND_DESKTOP, L"Failed to register window class. The program will now exit.", L"FATAL ERROR", MB_OK | MB_ICONERROR);
		return false;
	}

	rc.left = rc.top = 0;
	rc.right = *E_screenWidth;
	rc.bottom = *E_screenHeight;

	AdjustWindowRectEx(&rc, style, FALSE, exStyle);

	x = (GetSystemMetrics(SM_CXSCREEN) - *E_screenWidth) / 2;
	y = (GetSystemMetrics(SM_CYSCREEN) - *E_screenHeight) / 2;

	_window = CreateWindowExW(exStyle, WND_CLASS_NAME,
		L"NekoEngine", style, x, y, rc.right - rc.left, rc.bottom - rc.top,
		HWND_DESKTOP, NULL, Win32_instance, NULL);

	if (!_window) {
		MessageBoxW(HWND_DESKTOP, L"Failed to create window. The program will now exit.", L"FATAL ERROR", MB_OK | MB_ICONERROR);
		return false;
	}

	ShowWindow(_window, SW_SHOWDEFAULT);
	SetForegroundWindow(_window);
	SetActiveWindow(_window);

	E_screen = _window;

	return true;
}

void
Sys_SetWindowTitle(const char *name)
{
	SetWindowTextW(_window, NeWin32_UTF8toUCS2(name));
}

void
Sys_MoveWindow(int x, int y)
{
	RECT rc;
	GetWindowRect((HWND)E_screen, &rc);
	MoveWindow((HWND)E_screen, x, y, rc.right - rc.left, rc.bottom - rc.top, TRUE);
}

void
Sys_WorkArea(int *top, int *left, int *right, int *bottom)
{
	RECT rc;
	SystemParametersInfo(SPI_GETWORKAREA, sizeof(rc), &rc, 0);

	if (top) *top = rc.top;
	if (left) *left = rc.left;
	if (right) *right = rc.right;
	if (bottom) *bottom = rc.bottom;
}

void
Sys_DestroyWindow(void)
{
	UnregisterClass(WND_CLASS_NAME, Win32_instance);
}
