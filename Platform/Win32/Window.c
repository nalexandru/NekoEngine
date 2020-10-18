#include <stdint.h>

#include <Windows.h>
#include <Dbt.h>

#include <System/Window.h>
#include <Engine/Engine.h>
#include <Render/Render.h>

#include "Win32Platform.h"

static HWND _window;

#define WND_CLASS_NAME				L"LightWindowClass"
#define WM_SHOWCURSOR_MSG_GUID		L"CLAIRE_WM_SHOWCURSOR_{916fcbf2-b4be-4df8-884b-f0dc086e03ad}"
#define WM_HIDECURSOR_MSG_GUID		L"CLAIRE_WM_HIDECURSOR_{916fcbf2-b4be-4df8-884b-f0dc086e03ad}"

UINT WM_SHOWCURSOR = 0;
UINT WM_HIDECURSOR = 0;

LRESULT CALLBACK
_WndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	
	switch (umsg) {
	case WM_PAINT: {
		hdc = BeginPaint(hwnd, &ps);
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
		*E_ScreenWidth = LOWORD(lparam);
		*E_ScreenHeight = HIWORD(lparam);

		if (Re.ScreenResized)
			Re.ScreenResized();
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

int
Sys_CreateWindow(void)
{
	RECT rc;
	DWORD style = WS_VISIBLE | (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME) | WS_CLIPCHILDREN;
	DWORD exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	uint16_t x = 0, y = 0;

	WNDCLASS wincl =
	{
		.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
		.lpfnWndProc = _WndProc,
		.hInstance = GetModuleHandle(NULL),
		.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH),
		.lpszClassName = WND_CLASS_NAME,
		.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(300))
	};

	if (!WM_SHOWCURSOR)
		WM_SHOWCURSOR = RegisterWindowMessage(WM_SHOWCURSOR_MSG_GUID);

	if (!WM_HIDECURSOR)
		WM_HIDECURSOR = RegisterWindowMessage(WM_HIDECURSOR_MSG_GUID);

	if (!RegisterClass(&wincl)) {
		MessageBox(HWND_DESKTOP, L"Failed to register window class. The program will now exit.", L"FATAL ERROR", MB_OK | MB_ICONERROR);
		return -1;
	}

	rc.left = rc.top = 0;
	rc.right = *E_ScreenWidth;
	rc.bottom = *E_ScreenHeight;

	AdjustWindowRectEx(&rc, style, FALSE, exStyle);

	x = (GetSystemMetrics(SM_CXSCREEN) - *E_ScreenWidth) / 2;
    y = (GetSystemMetrics(SM_CYSCREEN) - *E_ScreenHeight) / 2;

	_window = CreateWindowEx(exStyle, WND_CLASS_NAME,
		L"NekoEngine", style, x, y, rc.right - rc.left, rc.bottom - rc.top,
		HWND_DESKTOP, NULL, GetModuleHandle(NULL), NULL);

	if (!_window) {
		return -1;
	}

	ShowWindow(_window, SW_SHOWDEFAULT);
	SetForegroundWindow(_window);
	SetActiveWindow(_window);

	E_Screen = _window;

	return 0;
}

void
Sys_SetWindowTitle(const wchar_t *name)
{
	SetWindowText(_window, name);
}

void
Sys_DestroyWindow(void)
{
	
}
