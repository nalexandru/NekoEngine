#include "Win32GUI.h"
#include "Inspector.h"

#include <dwmapi.h>
#include <CommCtrl.h>

#include <Scene/Scene.h>
#include <Engine/Entity.h>
#include <Engine/Component.h>
#include <Runtime/Runtime.h>

#define INS_TITLE_TEXT		60000

static HWND _wnd, _title;

static LRESULT CALLBACK _InsWndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

void
GUI_InspectEntity(NeEntityHandle handle)
{
	if (handle != ES_INVALID_ENTITY)
		SetWindowText(_title, NeWin32_UTF8toUCS2(E_EntityName(handle)));
	else
		SetWindowText(_title, L"No selection");
}

void
GUI_InspectScene(void)
{
	SetWindowText(_title, NeWin32_UTF8toUCS2(Scn_activeScene->name));
}

bool
GUI_InitInspector(int x, int y, int width, int height)
{
	DWORD style = WS_VISIBLE | (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME) | WS_CLIPCHILDREN;
	DWORD exStyle = WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_COMPOSITED;
	WNDCLASS wincl =
	{
		.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
		.lpfnWndProc = _InsWndProc,
		.hInstance = Win32_instance,
//		.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1),
		.lpszClassName = ED_INS_WND_CLASS_NAME,
		.hIcon = LoadIcon(Win32_instance, MAKEINTRESOURCE(300)),
	};

	if (!RegisterClassW(&wincl)) {
		MessageBoxW(HWND_DESKTOP, L"Failed to register window class. The program will now exit.", L"FATAL ERROR", MB_OK | MB_ICONERROR);
		return false;
	}

	RECT rc =
	{
		.top = 0,
		.left = 0,
		.right = width,
		.bottom = height
	};
	AdjustWindowRectEx(&rc, style, FALSE, exStyle);

	HMENU menu = NULL;

	_wnd = CreateWindowExW(exStyle, ED_INS_WND_CLASS_NAME,
		L"Inspector", style, x, y, rc.right - rc.left, rc.bottom - rc.top,
		HWND_DESKTOP, menu, Win32_instance, NULL);

	if (!_wnd) {
		MessageBoxW(HWND_DESKTOP, L"Failed to create window. The program will now exit.", L"FATAL ERROR", MB_OK | MB_ICONERROR);
		return false;
	}

	DWM_BLURBEHIND bb =
	{
		.dwFlags = DWM_BB_ENABLE,
		.fEnable = TRUE
	};
	DwmEnableBlurBehindWindow(_wnd, &bb);

	MARGINS m = { 10, 10, 10, 10 };
	DwmExtendFrameIntoClientArea(_wnd, &m);

	ShowWindow(_wnd, SW_SHOWDEFAULT);

	if (!_wnd)
		return false;

	return true;
}

void
GUI_ShowInspector(void)
{
	ShowWindow(_wnd, SW_SHOW);
	SetForegroundWindow(_wnd);
	SetActiveWindow(_wnd);
}

void
GUI_TermInspector()
{
	DestroyWindow(_wnd);
}

static LRESULT CALLBACK
_InsWndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg) {
	case WM_COMMAND: {

	} break;
	case WM_CREATE: {
		RECT rc;
		GetClientRect(hwnd, &rc);

		const uint32_t w = rc.right - rc.left;
		const uint32_t h = rc.bottom - rc.top;

		_title = CreateWindow(WC_STATIC, L"No selection", WS_VISIBLE | WS_CHILD, 10, 10, w - 20, 25, hwnd, (HMENU)INS_TITLE_TEXT, Win32_instance, NULL);
		SendMessage(_title, WM_SETFONT, (WPARAM)Ed_uiFont, 0);
	} break;
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	case WM_SIZE: {
		RECT rc;
		GetClientRect(hwnd, &rc);

		const uint32_t w = rc.right - rc.left;
		const uint32_t h = rc.bottom - rc.top;

		MoveWindow(_title, 10, 10, w - 20, 25, true);
	} break;
	default: {
	} break;
	}
	
	return DefWindowProc(hwnd, umsg, wparam, lparam);
}
