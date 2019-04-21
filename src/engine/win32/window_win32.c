/* NekoEngine
 *
 * window_win32.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Win32 Window Subsystem
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define VC_EXTRALEAN
#define _WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdbool.h>

#include <system/log.h>

#include <engine/input.h>
#include <engine/window.h>
#include <engine/engine.h>
#include <engine/keycodes.h>
#include <engine/application.h>
#include <graphics/graphics.h>

#define NE_WND_CLASS_NAME		"NekoEngineWindow"
#define WINDOW_WIN32_MODULE		"Window_Win32"
#define NWM_SHOWCURSOR_MSG_GUID		"NWM_SHOWCURSOR_{916fcbf2-b4be-4df8-884b-f0dc086e03ad}"
#define NWM_HIDECURSOR_MSG_GUID		"NWM_HIDECURSOR_{916fcbf2-b4be-4df8-884b-f0dc086e03ad}"

HWND win32_window_handle = INVALID_HANDLE_VALUE;
static uint16_t _width = 0, _height = 0;
static bool _fullscreen = false, _own_window = false;
UINT NWM_SHOWCURSOR = 0;
UINT NWM_HIDECURSOR = 0;
extern ne_key win32_to_ne_keycodes[256];

extern UINT (*win32_GetRawInputData)(HRAWINPUT, UINT, LPVOID, PUINT, UINT);
extern LRESULT (*win32_DefRawInputProc)(PRAWINPUT *, INT, UINT);

LRESULT CALLBACK
ne_wnd_proc(
	HWND hwnd,
	UINT umsg,
	WPARAM wparam,
	LPARAM lparam)
{
	LRESULT ret = 0;
	PAINTSTRUCT ps;
	HDC hdc;
	LPBYTE lpb[sizeof(RAWINPUT)];
	UINT lpb_size = sizeof(RAWINPUT);
	uint8_t key_code;
	UINT scan;
	int ext;

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
	case WM_KEYDOWN: {
		key_code = win32_to_ne_keycodes[(uint8_t)wparam];
		input_key_event(key_code, true);
		return 0;
	}
	case WM_KEYUP: {
		key_code = win32_to_ne_keycodes[(uint8_t)wparam];
		input_key_event(key_code, false);
		return 0;
	}
	case WM_LBUTTONDOWN: {
		input_key_event(NE_MOUSE_LMB, true);
		return 0;
	}
	case WM_LBUTTONUP: {
		input_key_event(NE_MOUSE_LMB, false);
		return 0;
	}
	case WM_RBUTTONDOWN: {
		input_key_event(NE_MOUSE_RMB, true);
		return 0;
	}
	case WM_RBUTTONUP: {
		input_key_event(NE_MOUSE_RMB, false);
		return 0;
	}
	case WM_MBUTTONDOWN: {
		input_key_event(NE_MOUSE_MMB, true);
		return 0;
	}
	case WM_MBUTTONUP: {
		input_key_event(NE_MOUSE_MMB, false);
		return 0;
	}
	case WM_XBUTTONDOWN: {
		input_key_event(GET_XBUTTON_WPARAM(wparam) == XBUTTON1 ? NE_MOUSE_BTN4 : NE_MOUSE_BTN5, true);
		return 0;
	}
	case WM_XBUTTONUP: {
		input_key_event(GET_XBUTTON_WPARAM(wparam) == XBUTTON1 ? NE_MOUSE_BTN4 : NE_MOUSE_BTN5, false);
		return 0;
	}
	case WM_INPUT:
	{
		// see:
		// https://msdn.microsoft.com/en-us/library/windows/desktop/ms645546(v=vs.85).aspx

		memset(lpb, 0x0, lpb_size);

		if (win32_GetRawInputData((HRAWINPUT)lparam, RID_INPUT,
			lpb, &lpb_size, sizeof(RAWINPUTHEADER)) > lpb_size) {
			log_entry(WINDOW_WIN32_MODULE, LOG_WARNING,
				"Incorrect size from GetRawInputData: %d, %d", sizeof(lpb), lpb_size);
			lpb_size = sizeof(lpb);
		}

		RAWINPUT *raw = (RAWINPUT *)lpb;

		if (raw->header.dwType == RIM_TYPEKEYBOARD) {
			scan = (lparam & 0x00FF0000) >> 16;
			ext = (lparam & 0x01000000) != 0;

			if (raw->data.keyboard.VKey == 255)
				return 0;

			switch (raw->data.keyboard.VKey) {
			case VK_SHIFT:
				key_code = MapVirtualKey(raw->data.keyboard.MakeCode,
					MAPVK_VSC_TO_VK_EX) == VK_LSHIFT ?
					NE_KEY_LSHIFT : NE_KEY_RSHIFT;
				break;
			case VK_CONTROL:
				key_code = raw->data.keyboard.Flags & RI_KEY_E0
					? NE_KEY_RCTRL : NE_KEY_LCTRL;
				break;
			case VK_MENU:
				key_code = raw->data.keyboard.Flags & RI_KEY_E0
					? NE_KEY_RALT : NE_KEY_LALT;
				break;
			case VK_RETURN:
				key_code = raw->data.keyboard.Flags & RI_KEY_E0
					? NE_KEY_NUM_RETURN : NE_KEY_RETURN;
				break;
			case VK_INSERT:
				key_code = raw->data.keyboard.Flags & RI_KEY_E0
					? NE_KEY_INSERT : NE_KEY_NUM_0;
				break;
			case VK_DELETE:
				key_code = raw->data.keyboard.Flags & RI_KEY_E0
					? NE_KEY_DELETE : NE_KEY_NUM_DECIMAL;
				break;
			case VK_HOME:
				key_code = raw->data.keyboard.Flags & RI_KEY_E0
					? NE_KEY_HOME : NE_KEY_NUM_7;
				break;
			case VK_END:
				key_code = raw->data.keyboard.Flags & RI_KEY_E0
					? NE_KEY_END : NE_KEY_NUM_1;
				break;
			case VK_PRIOR:
				key_code = raw->data.keyboard.Flags & RI_KEY_E0
					? NE_KEY_PGUP : NE_KEY_NUM_9;
				break;
			case VK_NEXT:
				key_code = raw->data.keyboard.Flags & RI_KEY_E0
					? NE_KEY_PGDN : NE_KEY_NUM_3;
				break;
			case VK_LEFT:
				key_code = raw->data.keyboard.Flags & RI_KEY_E0
					? NE_KEY_LEFT : NE_KEY_NUM_4;
				break;
			case VK_RIGHT:
				key_code = raw->data.keyboard.Flags & RI_KEY_E0
					? NE_KEY_RIGHT : NE_KEY_NUM_6;
				break;
			case VK_UP:
				key_code = raw->data.keyboard.Flags & RI_KEY_E0
					? NE_KEY_UP : NE_KEY_NUM_8;
				break;
			case VK_DOWN:
				key_code = raw->data.keyboard.Flags & RI_KEY_E0
					? NE_KEY_DOWN : NE_KEY_NUM_2;
				break;
			case VK_CLEAR:
				key_code = raw->data.keyboard.Flags & RI_KEY_E0
					? NE_KEY_CLEAR : NE_KEY_NUM_5;
				break;
			case VK_NUMLOCK:
				key_code = MapVirtualKey(raw->data.keyboard.MakeCode,
					MAPVK_VSC_TO_VK_EX) == VK_NUMLOCK ?
					NE_KEY_NUMLOCK : NE_KEY_PAUSE;
				break;
			default:
				key_code = win32_to_ne_keycodes[(uint8_t)raw->data.keyboard.VKey];
				break;
			}

			input_key_event(key_code, !(raw->data.keyboard.Flags & RI_KEY_BREAK));
		} else if (raw->header.dwType == RIM_TYPEMOUSE) {
			//int xPosRelative = raw->data.mouse.lLastX;
			//int yPosRelative = raw->data.mouse.lLastY;.
		}

		win32_DefRawInputProc(&raw, 1, sizeof(RAWINPUTHEADER));
	} break;
	case WM_SIZE:
	{
		_width = LOWORD(lparam);
		_height = HIWORD(lparam);

		engine_screen_resized(_width, _height);
	} break;
	case WM_SYSCOMMAND:
	{
		// Prevent monitor from turning off & screen saver from starting
		if (wparam == SC_SCREENSAVE || wparam == SC_MONITORPOWER)
			return 0;
		else if (wparam == SC_RESTORE)
			ShowWindow(hwnd, SW_RESTORE); // Required to show a minimized fullscreen window
	} break;
	case WM_DEVICECHANGE:
	{
		//if (wParam != DBT_DEVNODES_CHANGED)
		//	break;

		// TODO: update controller list
	}
	default:
	{
		if (umsg == NWM_SHOWCURSOR) {
			ShowCursor(true);
			return 0;
		} else if (umsg == NWM_HIDECURSOR) {
			ShowCursor(false);
			return 0;
		}
	} break;
	}

	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

ne_status
window_create(
	char *title,
	uint16_t w,
	uint16_t h)
{
	WNDCLASS wincl;
	RECT rc;
	DWORD style = WS_VISIBLE | WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME | WS_CLIPCHILDREN;
	DWORD ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	uint16_t x = 0, y = 0;

	memset(&rc, 0x0, sizeof(rc));
	memset(&wincl, 0x0, sizeof(wincl));

	if (!NWM_SHOWCURSOR)
		NWM_SHOWCURSOR = RegisterWindowMessage(NWM_SHOWCURSOR_MSG_GUID);

	if (!NWM_HIDECURSOR)
		NWM_HIDECURSOR = RegisterWindowMessage(NWM_HIDECURSOR_MSG_GUID);

	wincl.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wincl.lpfnWndProc = ne_wnd_proc;
	wincl.hInstance = GetModuleHandle(NULL);
	wincl.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wincl.lpszClassName = NE_WND_CLASS_NAME;

	// 300 = IDI_ICON in the launcher's resources
	wincl.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(300));

	if (!RegisterClass(&wincl))
		return NE_WIN_CLS_CREATE_FAIL;

	rc.left = rc.top = 0;
	rc.right = w;
	rc.bottom = h;

	AdjustWindowRectEx(&rc, style, FALSE, ex_style);

	x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
	y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

	win32_window_handle = CreateWindowEx(ex_style, NE_WND_CLASS_NAME,
		title, style, x, y, rc.right - rc.left,
		rc.bottom - rc.top, HWND_DESKTOP, NULL,
		GetModuleHandle(NULL), NULL);

	if (!win32_window_handle)
		return NE_WIN_CREATE_FAIL;

	ShowWindow(win32_window_handle, SW_SHOWDEFAULT);
	SetForegroundWindow(win32_window_handle);
	SetActiveWindow(win32_window_handle);

	_width = w;
	_height = h;
	_own_window = true;

	return NE_OK;
}

ne_status
window_register(void *handle)
{
	win32_window_handle = handle;

	RECT rc;
	if (!GetClientRect(win32_window_handle, &rc))
		return NE_FAIL;

	_width = (uint16_t)rc.right;
	_height = (uint16_t)rc.bottom;

	return NE_OK;
}

ne_status
window_resize(
	uint16_t w,
	uint16_t h)
{
	return NE_FAIL;
}

ne_status
window_set_title(char *title)
{
	return SetWindowText(win32_window_handle, title) == TRUE ? NE_OK : NE_FAIL;
}

ne_status
window_fullscreen(
	uint16_t w,
	uint16_t h)
{
	DEVMODE dss;
	LONG ret = 0;

	dss.dmSize = sizeof(dss);
	dss.dmPelsWidth = w;
	dss.dmPelsHeight = h;
	dss.dmBitsPerPel = 32;
	dss.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	if ((ret = ChangeDisplaySettings(&dss, CDS_FULLSCREEN)) != DISP_CHANGE_SUCCESSFUL) {
		log_entry(WINDOW_WIN32_MODULE, LOG_CRITICAL, "ChangeDisplaySettings returned %d", ret);
		return NE_FAIL;
	}

	return NE_OK;
}

void
window_destroy(void)
{
	if (_own_window)
		DestroyWindow(win32_window_handle);

	win32_window_handle = INVALID_HANDLE_VALUE;
}
