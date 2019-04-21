/* NekoEngine
 *
 * launcher_win32.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Win32 Launcher Application
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


#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include <runtime/runtime.h>
#include <runtime/array.h>
#include <runtime/string.h>
#include <system/config.h>

#include <engine/engine.h>

#include "launcher.h"
#include "../../proj/vs/resource.h"

#define NE_LAUNCHER_WND_CLASS	"NekoEngineLauncherWindow"

#define BTN_START	100
#define BTN_EXIT	101

#define BTN_WIDTH	75
#define BTN_HEIGHT	23
#define BTN_SPACER	10

#define TAB_VOFFSET 35

typedef BOOL (WINAPI *InitCommonControlsExProc)(const INITCOMMONCONTROLSEX *);

HINSTANCE _hinst;
static HMODULE _comctl32_module = NULL;
static InitCommonControlsExProc _InitCommonControlsEx = NULL;

static HWND _launcher_wnd = INVALID_HANDLE_VALUE;
static bool _start_engine = false;
static uint32_t _width = LAUNCHER_WIDTH, _height = LAUNCHER_HEIGHT;
static HWND _tab_control = INVALID_HANDLE_VALUE,
	_gfx_combo_box = INVALID_HANDLE_VALUE,
	_snd_combo_box = INVALID_HANDLE_VALUE;
static rt_array _tab_pages[4];
static uint32_t _current_tab;

void show_tab(uint32_t id)
{
	if (_current_tab == id)
		return;

	for (uint32_t i = 0; i < _tab_pages[_current_tab].count; ++i)
		ShowWindow(*((HWND *)rt_array_get(&_tab_pages[_current_tab], i)), SW_HIDE);

	_current_tab = id;

	for (uint32_t i = 0; i < _tab_pages[_current_tab].count; ++i)
		ShowWindow(*((HWND *)rt_array_get(&_tab_pages[_current_tab], i)), SW_SHOW);
}

LRESULT CALLBACK
launcher_wnd_proc(
	HWND hwnd,
	UINT umsg,
	WPARAM wparam,
	LPARAM lparam)
{
	switch (umsg) {
	case WM_CREATE:
	{
		HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		HWND ctl;

		ctl = CreateWindow("BUTTON", "Exit", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			BTN_SPACER, _height - (BTN_HEIGHT + BTN_SPACER),
			BTN_WIDTH, BTN_HEIGHT, hwnd, (HMENU)BTN_EXIT, _hinst, NULL);
		SendMessage(ctl, WM_SETFONT, (WPARAM)font, TRUE);

		ctl = CreateWindow("BUTTON", "Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			_width - (BTN_WIDTH + BTN_SPACER), _height - (BTN_HEIGHT + BTN_SPACER),
			BTN_WIDTH, BTN_HEIGHT, hwnd, (HMENU)BTN_START, _hinst, NULL);
		SendMessage(ctl, WM_SETFONT, (WPARAM)font, TRUE);

		ctl = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE | SS_BITMAP,
			BTN_SPACER, BTN_SPACER, BANNER_WIDTH, BANNER_HEIGHT, hwnd, NULL,
			_hinst, NULL);

		HBITMAP banner = (HBITMAP)LoadImage(GetModuleHandle("Launcher.exe"),
			MAKEINTRESOURCE(IDB_BANNER),
			IMAGE_BITMAP, 330, 120, LR_LOADTRANSPARENT);

		InvalidateRect(ctl, NULL, FALSE);

		HBITMAP old = (HBITMAP)SendMessage(ctl, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)banner);
		if (old)
			DeleteObject(old);

		_tab_control = CreateWindow(WC_TABCONTROL, "", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
			BTN_SPACER, (BTN_SPACER * 2) + BANNER_HEIGHT, _width - (BTN_SPACER * 2),
			_height - BANNER_HEIGHT - (BTN_SPACER * 4) - BTN_HEIGHT, hwnd, (HMENU)10,
			_hinst, NULL);
		SendMessage(_tab_control, WM_SETFONT, (WPARAM)font, TRUE);

		TCITEM tie;
		tie.mask = TCIF_TEXT | TCIF_IMAGE;
		tie.iImage = -1;

		tie.pszText = (char *)"General";
		TabCtrl_InsertItem(_tab_control, 0, &tie);

		tie.pszText = (char *)"Graphics";
		TabCtrl_InsertItem(_tab_control, 1, &tie);

		tie.pszText = (char *)"Audio";
		TabCtrl_InsertItem(_tab_control, 2, &tie);

		tie.pszText = (char *)"About";
		TabCtrl_InsertItem(_tab_control, 3, &tie);

		RECT rc;
		GetWindowRect(_tab_control, &rc);
		POINT offset;
		ScreenToClient(hwnd, &offset);
		OffsetRect(&rc, offset.x, offset.y);
		rc.top += 50;

		{ // general page
			rt_array_init(&_tab_pages[0], 1, sizeof(HWND));

			ctl = CreateWindow(WC_STATIC, "", WS_CHILD,
				rc.left, rc.top, rc.right - rc.left, rc.top - rc.bottom, _tab_control, NULL, _hinst, NULL);
			SendMessage(ctl, WM_SETFONT, (WPARAM)font, TRUE);
			rt_array_add(&_tab_pages[0], &ctl);
		}

		{ // graphics page
			rt_array_init(&_tab_pages[1], 2, sizeof(HWND));

			ctl = CreateWindow(WC_STATIC, "Device:", WS_CHILD | SS_CENTER,
				10, TAB_VOFFSET, 40, 22, _tab_control, NULL, _hinst, NULL);
			SendMessage(ctl, WM_SETFONT, (WPARAM)font, TRUE);
			rt_array_add(&_tab_pages[1], &ctl);

			ctl = CreateWindow(WC_COMBOBOXA, "", WS_CHILD | CBS_DROPDOWN | CBS_HASSTRINGS,
				60, TAB_VOFFSET, BANNER_WIDTH - 80, 200, _tab_control, NULL, _hinst, NULL);
			SendMessage(ctl, WM_SETFONT, (WPARAM)font, TRUE);
			rt_array_add(&_tab_pages[1], &ctl);

			_gfx_combo_box = ctl;

			/*RtArray<RtString> devices;
			if (NeStatus ret = graphics_get_devices(devices); ret != NE_OK) {
				MessageBoxA(HWND_DESKTOP, "Failed to enumerate graphics devices", "FATAL ERROR", MB_OK | MB_ICONERROR);
				exit(-2);
			}

			for (const RtString str : devices)
				ComboBox_AddString(ctl, *str);

			int32_t id = 0;
			if (id = (int32_t)config_get_int("gfx_device_id", 0); id == -1)
				id = 0;

			ComboBox_SetCurSel(ctl, id);*/
		}

		{ // audio page
			rt_array_init(&_tab_pages[2], 2, sizeof(HWND));

			ctl = CreateWindow(WC_STATIC, "Device:", WS_CHILD | SS_CENTER,
				10, TAB_VOFFSET, 40, 22, _tab_control, NULL, _hinst, NULL);
			SendMessage(ctl, WM_SETFONT, (WPARAM)font, TRUE);
			rt_array_add(&_tab_pages[2], &ctl);

			ctl = CreateWindow(WC_COMBOBOXA, "", WS_CHILD | CBS_DROPDOWN | CBS_HASSTRINGS,
				60, TAB_VOFFSET, BANNER_WIDTH - 80, 200, _tab_control, NULL, _hinst, NULL);
			SendMessage(ctl, WM_SETFONT, (WPARAM)font, TRUE);
			rt_array_add(&_tab_pages[2], &ctl);

			_snd_combo_box = ctl;

			/*RtArray<RtString> devices;
			if (NeStatus ret = audio_get_devices(devices); ret != NE_OK) {
				MessageBoxA(HWND_DESKTOP, "Failed to enumerate audio devices", "FATAL ERROR", MB_OK | MB_ICONERROR);
				exit(-2);
			}

			for (const RtString str : devices)
				ComboBox_AddString(ctl, *str);

			int32_t id = 0;
			if (id = (int32_t)config_get_int("snd_device_id", 0); id == -1)
				id = 0;

			ComboBox_SetCurSel(ctl, id);*/
		}

		{ // about page
			rt_array_init(&_tab_pages[3], 3, sizeof(HWND));

			ctl = CreateWindow(WC_STATIC, "NekoEngine", WS_VISIBLE | WS_CHILD | SS_LEFT, 10,
				TAB_VOFFSET, rc.right - rc.left - 20, BTN_HEIGHT, _tab_control, NULL,
				_hinst, NULL);
			SendMessage(ctl, WM_SETFONT, (WPARAM)font, TRUE);
			rt_array_add(&_tab_pages[3], &ctl);

			ctl = CreateWindow(WC_STATIC, "Version: 0.6.0.600", WS_VISIBLE | WS_CHILD | SS_LEFT, 10,
				TAB_VOFFSET + 23, rc.right - rc.left - 20, BTN_HEIGHT, _tab_control, NULL,
				_hinst, NULL);
			SendMessage(ctl, WM_SETFONT, (WPARAM)font, TRUE);
			rt_array_add(&_tab_pages[3], &ctl);

			ctl = CreateWindow(WC_STATIC, "Engine version: 0.6.0.600", WS_VISIBLE | WS_CHILD | SS_LEFT, 10,
				TAB_VOFFSET + 46, rc.right - rc.left - 20, BTN_HEIGHT, _tab_control, NULL,
				_hinst, NULL);
			SendMessage(ctl, WM_SETFONT, (WPARAM)font, TRUE);
			rt_array_add(&_tab_pages[3], &ctl);
		}

		for (uint8_t i = 1; i < 4; ++i)
			for (uint32_t j = 0; j < _tab_pages[i].count; ++j)
				ShowWindow(*((HWND *)(HWND)rt_array_get(&_tab_pages[i], j)), SW_HIDE);

		for (uint32_t i = 0; i < _tab_pages[0].count; ++i)
			ShowWindow(*((HWND *)rt_array_get(&_tab_pages[0], i)), SW_SHOW);

		_current_tab = 0;
	} break;
	case WM_COMMAND:
	{
		if (HIWORD(wparam) == BN_CLICKED) {
			switch (LOWORD(wparam)) {
			case BTN_START:
			{
				_start_engine = true;

				sys_config_set_int("gfx_device_id", ComboBox_GetCurSel(_gfx_combo_box));
				sys_config_set_int("snd_device_id", ComboBox_GetCurSel(_snd_combo_box));

				DestroyWindow(_launcher_wnd);
			} break;
			case BTN_EXIT:
			{
				PostQuitMessage(0);
			} break;
			}
		}
	} break;
	case WM_NOTIFY:
	{
		if (((LPNMHDR)lparam)->code == TCN_SELCHANGE)
			show_tab(TabCtrl_GetCurSel(_tab_control));
	} break;
	case WM_DESTROY:
	{
		if (!_start_engine)
			PostQuitMessage(0);
	} break;
	}

	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

int
launcher_exec(void)
{
	MSG msg;
	RECT rc;
	WNDCLASS wincl;
	DWORD ex_style = WS_EX_CLIENTEDGE;
	DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
	INITCOMMONCONTROLSEX icex;
	int x = 0, y = 0;

	memset(&rc, 0x0, sizeof(rc));
	memset(&icex, 0x0, sizeof(icex));
	memset(&wincl, 0x0, sizeof(wincl));

	_comctl32_module = LoadLibrary("comctl32.dll");
	_InitCommonControlsEx = (InitCommonControlsExProc)
		GetProcAddress(_comctl32_module, "InitCommonControlsEx");

	if (_InitCommonControlsEx) {
		icex.dwSize = sizeof(icex);
		icex.dwICC = ICC_WIN95_CLASSES;
		_InitCommonControlsEx(&icex);
	} else {
		InitCommonControls();
	}
	
	wincl.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wincl.lpfnWndProc = launcher_wnd_proc;
	wincl.hInstance = _hinst;
	wincl.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wincl.lpszClassName = NE_LAUNCHER_WND_CLASS;

	if (!RegisterClass(&wincl)) {
		MessageBoxA(HWND_DESKTOP, "Failed to register launcher window class", "FATAL ERROR", MB_OK | MB_ICONERROR);
		return -1;
	}

	x = (GetSystemMetrics(SM_CXSCREEN) - _width) / 2;
	y = (GetSystemMetrics(SM_CYSCREEN) - _height) / 2;

	rc.top = rc.left = 0;
	rc.right = _width;
	rc.bottom = _height;

	AdjustWindowRectEx(&rc, style, FALSE, ex_style);

	_launcher_wnd = CreateWindowEx(ex_style, NE_LAUNCHER_WND_CLASS, LAUNCHER_TITLE,
		style, x, y, rc.right - rc.left, rc.bottom - rc.top, HWND_DESKTOP, NULL, _hinst, NULL);

	ShowWindow(_launcher_wnd, SW_SHOWDEFAULT);
	SetForegroundWindow(_launcher_wnd);

	PeekMessage(&msg, _launcher_wnd, 0, 0, PM_NOREMOVE);

	while (!_start_engine && msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	UnregisterClass(NE_LAUNCHER_WND_CLASS, _hinst);

	return msg.message == WM_QUIT;
}

void
cleanup(void)
{
	engine_destroy();
}

int APIENTRY
WinMain(_In_ HINSTANCE h_instance,
	_In_opt_ HINSTANCE h_prev_instance,
	_In_ LPSTR lp_cmd_line,
	_In_ int n_cmd_show)
{
	int exit_code;
	ne_status ret;
	rt_string str;
	rt_string_init_with_cstr(&str, lp_cmd_line);

	if (rt_string_find(&str, "--launcher") != RT_NOT_FOUND && launcher_exec())
		return 0;

	atexit(cleanup);

#if defined(_NE_CONFIG_DEBUG_) || defined(_NE_CONFIG_DEVELOPMENT_)
	if (!IsDebuggerPresent() && (rt_string_find(&str, "--noconsole") == RT_NOT_FOUND)) {
		FreeConsole();
		AllocConsole();
		AttachConsole(GetCurrentProcessId());

		(void)freopen("CON", "w", stdout);
		(void)freopen("CON", "w", stderr);

		system("title NekoEngine Log Console");
	}
#endif

	_hinst = h_instance;

	if (rt_string_find(&str, "--waitrdoc") != RT_NOT_FOUND)
		MessageBoxA(HWND_DESKTOP, "Press OK after RenderDoc injection",
			"Waiting for RenderDoc", MB_OK | MB_ICONINFORMATION);

	if ((ret = engine_init(__argc, __argv, NULL)) != NE_OK) {
		if (ret == NE_ABORT_START)
			return 0;

		MessageBoxA(HWND_DESKTOP, "Initialization failed. The program will now exit.", "FATAL ERROR", MB_OK | MB_ICONERROR);
		return -1;
	}

	exit_code = engine_run();

	if (_comctl32_module)
		FreeLibrary(_comctl32_module);

	return exit_code;
}
