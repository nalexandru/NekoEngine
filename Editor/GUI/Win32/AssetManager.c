#include "Win32GUI.h"
#include "AboutDialog.h"
#include "AssetManager.h"

#include "../../resource.h"
#include "Inspector.h"
#include "SceneHierarchy.h"

#include <dwmapi.h>
#include <CommCtrl.h>
#include <Uxtheme.h>

#include <Engine/IO.h>
#include <Engine/Engine.h>
#include <System/Log.h>
#include <Runtime/Runtime.h>
#include <Editor/Asset/Asset.h>
#include <Editor/Asset/Import.h>

#define AM_IMPORT_BUTTON	60000
#define AM_LIST_VIEW		60001

static HWND _wnd, _list, _path;
static char _currentPath[4096] = { '/', 0x0 };
static const char **_fileList;

static void _UpdateList(void);
static void _ListDoubleClick(LPNMITEMACTIVATE nmi);

static void _ImportAsset(void);

static LRESULT CALLBACK _AMWndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

bool
GUI_InitAssetManager(int x, int y, int width, int height)
{
	DWORD style = WS_VISIBLE | (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME) | WS_CLIPCHILDREN;
	DWORD exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_LAYERED | WS_EX_COMPOSITED;
	WNDCLASS wincl =
	{
		.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
		.lpfnWndProc = _AMWndProc,
		.hInstance = Win32_instance,
//		.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1),
		.lpszClassName = ED_AM_WND_CLASS_NAME,
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

	HMENU menu = LoadMenu(Win32_instance, MAKEINTRESOURCE(IDR_AM_MENU));

	_wnd = CreateWindowExW(exStyle, ED_AM_WND_CLASS_NAME,
		L"Asset Manager", style, x, y, rc.right - rc.left, rc.bottom - rc.top,
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

	MARGINS m = { 10, 10, 65, 10 };
	DwmExtendFrameIntoClientArea(_wnd, &m);

	ShowWindow(_wnd, SW_SHOWDEFAULT);

	if (!_wnd)
		return false;

	_UpdateList();

	return true;
}

void
GUI_ShowAssetManager(void)
{
	ShowWindow(_wnd, SW_SHOW);
	SetForegroundWindow(_wnd);
	SetActiveWindow(_wnd);
}

void
GUI_TermAssetManager(void)
{
	E_FreeFileList(_fileList);

	DestroyWindow(_wnd);
}

static LRESULT CALLBACK
_AMWndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg) {
	case WM_NOTIFY: {
		LPNMHDR hdr = (LPNMHDR)lparam;

		if (hdr->code == NM_DBLCLK)
			_ListDoubleClick((LPNMITEMACTIVATE)lparam);
	} break;
	case WM_COMMAND: {
		switch (wparam) {
		case ID_FILE_QUIT: { E_Shutdown(); } break;
		case ID_TOOLS_SCENEHIERARCHY: { GUI_ShowInspector(); } break;
		case ID_TOOLS_INSPECTOR: { GUI_ShowInspector(); } break;
		case ID_HELP_ABOUT: { GUI_AboutDialog(); } break;
		case AM_IMPORT_BUTTON: { _ImportAsset(); } break;
		}
	} break;
	case WM_CREATE: {
		RECT rc;
		GetClientRect(hwnd, &rc);

		const uint32_t w = rc.right - rc.left;
		const uint32_t h = rc.bottom - rc.top;

		HWND wnd = CreateWindow(WC_BUTTON, L"Import", WS_VISIBLE | WS_CHILD, 10, 10, 75, 25, hwnd, (HMENU)AM_IMPORT_BUTTON, Win32_instance, NULL);
		SendMessage(wnd, WM_SETFONT, (WPARAM)Ed_uiFont, 0);

		_path = CreateWindowEx(WS_EX_CLIENTEDGE, WC_STATIC, L"/", WS_VISIBLE | WS_CHILD, 95, 13, w - 105, 20, hwnd, 0, Win32_instance, NULL);
		SendMessage(_path, WM_SETFONT, (WPARAM)Ed_uiFont, 0);

		_list = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", WS_VISIBLE | WS_CHILD | LVS_LIST, 10, 45, w - 20, h - 55, hwnd, (HMENU)AM_LIST_VIEW, Win32_instance, NULL);
		SendMessage(_list, WM_SETFONT, (WPARAM)Ed_uiFont, 0);
		SetWindowTheme(_list, L"Explorer", NULL);
	} break;
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	} break;
	case WM_SIZE: {
		RECT rc;
		GetClientRect(hwnd, &rc);

		const uint32_t w = rc.right - rc.left;
		const uint32_t h = rc.bottom - rc.top;

		MoveWindow(_path, 95, 13, w - 105, 20, true);
		MoveWindow(_list, 10, 45, w - 20, h - 55, true);
	} break;
	default: {
	} break;
	}
	
	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

static void
_UpdateList(void)
{
	if (_fileList)
		E_FreeFileList(_fileList);

	_fileList = E_ListFiles(_currentPath);

	ListView_DeleteAllItems(_list);

#define ADD_ITEM(x) \
	LVITEMA item =								\
	{											\
		.mask = LVIF_TEXT,						\
		.iItem = ListView_GetItemCount(_list),	\
		.pszText = x							\
	};											\
	SendMessageA(_list, LVM_INSERTITEMA, 0, (LPARAM)&item)

	//ListView_InsertItem(_list, &item)

	if (strnlen(_currentPath, sizeof(_currentPath)) > 1) {
		ADD_ITEM("..");
	}

	for (const char **i = _fileList; *i != NULL; ++i) {
		if (*i[0] == '.')
			continue;

		ADD_ITEM((char *)*i);
	}

	SetWindowTextA(_path, _currentPath);
}

static void
_ListDoubleClick(LPNMITEMACTIVATE nmi)
{
	char newPath[4096] = { 0x0 }, text[4096] = { 0x0 };

	LV_ITEMA lvi =
	{
		.cchTextMax = sizeof(text),
		.pszText = text
	};
	SendMessageA(_list, LVM_GETITEMTEXTA, (WPARAM)nmi->iItem, (LPARAM)&lvi);

	if (!strncmp(text, "..", strnlen(text, sizeof(text)))) {
		char *p = strrchr(_currentPath, '/');
		size_t pos = p - _currentPath;

		if (pos) {
			_currentPath[pos] = 0x0;
			memcpy(newPath, _currentPath, pos);
		} else {
			newPath[0] = '/';
		}
	} else {
		snprintf(newPath, sizeof(newPath), strnlen(_currentPath, sizeof(_currentPath)) > 1 ? "%s/%s" : "%s%s", _currentPath, text);
	}

	if (E_IsDirectory(newPath)) {
		memcpy(_currentPath, newPath, sizeof(_currentPath));
		_UpdateList();
	} else {
		Ed_OpenAsset(newPath);
	}
}

static void
_ImportAsset(void)
{
	char file[MAX_PATH] = { 0x0 };

	OPENFILENAMEA ofn =
	{
		.lStructSize = sizeof(ofn),
		.hwndOwner = _wnd,
		.lpstrFile = file,
		.nMaxFile = sizeof(file),
		.lpstrFilter = "glTF Binary Files\0*.glb\0",
		.nFilterIndex = 1,
		.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST
	};

	if (!GetOpenFileNameA(&ofn))
		return;

	Asset_Import(file);
}
