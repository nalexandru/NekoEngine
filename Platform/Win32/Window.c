#include <stdint.h>
#include <stdbool.h>

#include "Win32Platform.h"
#include <dbt.h>
#include <dwmapi.h>

#include <Input/Input.h>
#include <System/Window.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Render/Render.h>

#include "Win32Platform.h"

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#	define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

static HWND f_window;

#define WND_CLASS_NAME				L"NekoEngineWindowClass"
#define WM_SHOWCURSOR_MSG_GUID		L"NE_WM_SHOWCURSOR_{916fcbf2-b4be-4df8-884b-f0dc086e03ad}"
#define WM_HIDECURSOR_MSG_GUID		L"NE_WM_HIDECURSOR_{916fcbf2-b4be-4df8-884b-f0dc086e03ad}"

__declspec(dllexport) UINT WM_SHOWCURSOR = 0;
__declspec(dllexport) UINT WM_HIDECURSOR = 0;

static LRESULT CALLBACK
NeWndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg) {
	case WM_PAINT: {
		PAINTSTRUCT ps;
		BeginPaint(hwnd, &ps);
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
	case WM_KEYDOWN: {
		In_buttonState[Win32_keymap[wparam]] = true;
	} break;
	case WM_KEYUP: {
		In_buttonState[Win32_keymap[wparam]] = false;
	} break;
	case WM_LBUTTONDOWN: {
		In_buttonState[BTN_MOUSE_LMB] = true;
	} break;
	case WM_LBUTTONUP: {
		In_buttonState[BTN_MOUSE_LMB] = false;
	} break;
	case WM_RBUTTONDOWN: {
		In_buttonState[BTN_MOUSE_RMB] = true;
	} break;
	case WM_RBUTTONUP: {
		In_buttonState[BTN_MOUSE_RMB] = false;
	} break;
	case WM_MBUTTONDOWN: {
		In_buttonState[BTN_MOUSE_MMB] = true;
	} break;
	case WM_MBUTTONUP: {
		In_buttonState[BTN_MOUSE_MMB] = false;
	} break;
	case WM_XBUTTONDOWN: {
		In_buttonState[BTN_MOUSE_MMB + HIWORD(wparam)] = true;
	} break;
	case WM_XBUTTONUP: {
		In_buttonState[BTN_MOUSE_MMB + HIWORD(wparam)] = false;
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
	if (f_window)
		return true;

	RECT rc;
	DWORD style = (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME) | WS_CLIPCHILDREN;
	DWORD exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	uint16_t x = 0, y = 0;

	WNDCLASSW wincl = { 0 };
	wincl.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wincl.lpfnWndProc = NeWndProc;
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

	f_window = CreateWindowExW(exStyle, WND_CLASS_NAME,
							   L"NekoEngine", style, x, y, rc.right - rc.left, rc.bottom - rc.top,
							   HWND_DESKTOP, NULL, Win32_instance, NULL);

	if (!f_window) {
		MessageBoxW(HWND_DESKTOP, L"Failed to create window. The program will now exit.", L"FATAL ERROR", MB_OK | MB_ICONERROR);
		return false;
	}

	if (dwmapi_DwmSetWindowAttribute) {
		BOOL value = TRUE;
		dwmapi_DwmSetWindowAttribute(f_window, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
	}

	if (!CVAR_BOOL("Window_CreateHidden")) {
		ShowWindow(f_window, SW_SHOWDEFAULT);
		SetForegroundWindow(f_window);
		SetActiveWindow(f_window);
	}

	E_screen = f_window;

	return true;
}

void
Sys_SetEngineWindow(void *wnd)
{
	RECT rc;
	GetClientRect((HWND)wnd, &rc);

	*E_screenWidth = rc.right - rc.left;
	*E_screenHeight = rc.bottom - rc.top;
	f_window = (HWND)wnd;
	E_screen = wnd;
}

void
Sys_SetWindowTitle(const char *name)
{
	SetWindowTextW(f_window, NeWin32_UTF8toUCS2(name));
}

void
Sys_MoveWindow(int x, int y)
{
	RECT rc;
	GetWindowRect((HWND)E_screen, &rc);
	MoveWindow((HWND)E_screen, x, y, rc.right - rc.left, rc.bottom - rc.top, TRUE);
}

void
Sys_ShowWindow(bool show)
{
	ShowWindow((HWND)E_screen, show ? SW_SHOW : SW_HIDE);
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
Sys_NonClientMetrics(int32_t *titleBarHeight, int32_t *borderHeight, int32_t *borderWidth)
{
	if (titleBarHeight) *titleBarHeight = GetSystemMetrics(SM_CYCAPTION);
	if (borderHeight) *borderHeight = GetSystemMetrics(SM_CYBORDER);
	if (borderWidth) *borderWidth = GetSystemMetrics(SM_CXBORDER);
}

void
Sys_DestroyWindow(void)
{
	UnregisterClass(WND_CLASS_NAME, Win32_instance);
}

/* NekoEngine
 *
 * Window.c
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
