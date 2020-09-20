#include <assert.h>

#include "DbgLogger.h"

#define MNU_FILE_SAVE	2000
#define MNU_FILE_QUIT	2001
#define MNU_HELP_ABOUT	2200

#define INITIAL_LOG_SIZE	65536

#define WIDTH		600
#define HEIGHT		400
#define CLASSNAME	L"ClaireDbgLoggerWnd"

static HWND _logEditView = NULL;
static HINSTANCE _instance = INVALID_HANDLE_VALUE;
static wchar_t *_logText = NULL;
static size_t _logTextSize = INITIAL_LOG_SIZE, _logTextUsed = 0;

static inline void _CreateInterface(HWND);
static inline void _HandleCommand(HWND, WPARAM, LPARAM);
static LRESULT CALLBACK _WndProc(HWND, UINT, WPARAM, LPARAM);

void
LogMessage(const wchar_t *msg)
{
	size_t len = wcsnlen(msg, MSG_BUFF_LEN) + 2;

	if (_logTextUsed + len > _logTextSize) {
		_logTextSize *= 2;
		assert(_logTextSize > _logTextUsed);

		_logText = realloc(_logText, _logTextSize);
		assert(_logText);
	}

	wcscat_s(_logText, _logTextSize, msg);
	wcscat_s(_logText, _logTextSize, L"\r\n");
	_logTextUsed += len;

	SetWindowText(_logEditView, _logText);
}

int APIENTRY
WinMain(HINSTANCE inst, HINSTANCE prevInst, LPSTR cmdList, INT showCmd)
{
	MSG msg;
	RECT rc;
	UINT x, y;
	HWND wnd;
	DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
	DWORD exStyle = WS_EX_APPWINDOW;
	WNDCLASS wincl;
	HMENU fileMenu, editMenu, helpMenu, menuBar;
	WSADATA wsaData;

	_instance = inst;

	ZeroMemory(&wincl, sizeof(wincl));

	{ // Create Menu
		fileMenu = CreateMenu();
		AppendMenu(fileMenu, MF_STRING, MNU_FILE_SAVE, L"Save");
		AppendMenu(fileMenu, MF_SEPARATOR, 0, NULL);
		AppendMenu(fileMenu, MF_STRING, MNU_FILE_QUIT, L"Quit");

		editMenu = CreateMenu();

		helpMenu = CreateMenu();
		AppendMenu(helpMenu, MF_STRING, MNU_HELP_ABOUT, L"About");

		menuBar = CreateMenu();
		AppendMenu(menuBar, MF_POPUP, (UINT_PTR)fileMenu, L"File");
		AppendMenu(menuBar, MF_POPUP, (UINT_PTR)editMenu, L"Edit");
		AppendMenu(menuBar, MF_POPUP, (UINT_PTR)helpMenu, L"Help");
	}

	{ // Create Window
		wincl.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wincl.lpfnWndProc = _WndProc;
		wincl.hInstance = inst;
		wincl.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wincl.lpszClassName = CLASSNAME;

		//wincl.hIcon = 

		if (!RegisterClass(&wincl))
			return false;

		rc.left = rc.top = 0;
		rc.right = WIDTH;
		rc.bottom = HEIGHT;

		AdjustWindowRectEx(&rc, style, TRUE, exStyle);

		x = (GetSystemMetrics(SM_CXSCREEN) - WIDTH) / 2;
		y = (GetSystemMetrics(SM_CYSCREEN) - HEIGHT) / 2;

		wnd = CreateWindowEx(exStyle, CLASSNAME, L"Claire Debug Log Server", style, x, y,
			rc.right - rc.left, rc.bottom - rc.top, HWND_DESKTOP, menuBar, inst, NULL);
	}

	_logText = calloc(_logTextSize, sizeof(wchar_t));
	ShowWindow(wnd, showCmd);

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	InitServer();

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	TermServer();

	WSACleanup();

	return (int)msg.wParam;
}

void
_CreateInterface(HWND wnd)
{
	_logEditView = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL,
		10, 10, WIDTH - 20, HEIGHT - 20, wnd, NULL, _instance, NULL);
	SendMessage(_logEditView, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
}

void
_HandleCommand(HWND wnd, WPARAM wParam, LPARAM lParam)
{
	OPENFILENAME ofn;

	switch (LOWORD(wParam)) {
	case MNU_FILE_SAVE: {
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = wnd;
		ofn.hInstance = _instance;
		ofn.lpstrFilter = L"Log Files (*.log)|*.log";
		ofn.lpstrTitle = L"Save Log File";
		ofn.lpstrFile = L"Claire.log";

		GetSaveFileName(&ofn);
	} break;
	case MNU_FILE_QUIT: {
		PostQuitMessage(0);
	} break;
	case MNU_HELP_ABOUT: {
		MessageBox(wnd, L"DbgLogger\nVersion: 0.1\n(c) 2020 Alexandru Naiman. All rights reserved.", L"About DbgLogger", MB_OK | MB_ICONINFORMATION);
	} break;
	}
}

static LRESULT CALLBACK
_WndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CREATE:
		_CreateInterface(wnd);
	break;
	case WM_SIZE:
		MoveWindow(_logEditView, 10, 10, LOWORD(lParam) - 20, HIWORD(lParam) - 20, TRUE);
	break;
	case WM_COMMAND:
		_HandleCommand(wnd, wParam, lParam);
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
	break;
	}

	return DefWindowProc(wnd, msg, wParam, lParam);
}

