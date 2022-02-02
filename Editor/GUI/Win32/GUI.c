#include "Win32GUI.h"
#include "Inspector.h"
#include "AssetManager.h"
#include "SceneHierarchy.h"

#include "../../resource.h"

#include <CommCtrl.h>

#include <Editor/GUI.h>
#include <Editor/Editor.h>
#include <Editor/Project.h>
#include <Engine/Engine.h>
#include <System/Window.h>
#include <Runtime/Runtime.h>

HFONT Ed_uiFont = NULL;

static HWND _workingDialog, _dialogText;

INT_PTR CALLBACK _ProgressDlgProc(HWND hdlg, UINT umsg, WPARAM wparam, LPARAM lparam);

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls'	\
					version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

void
Ed_ShowProjectDialog(void)
{
	Ed_activeProject = (struct NeProject *)1;
}

bool
Ed_CreateGUI(void)
{
	INITCOMMONCONTROLSEX icce = { sizeof(icce), ICC_WIN95_CLASSES | ICC_PROGRESS_CLASS };
	InitCommonControlsEx(&icce);

	NONCLIENTMETRICS ncm = { .cbSize = sizeof(ncm) };
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

	RECT workArea;
	SystemParametersInfo(SPI_GETWORKAREA, sizeof(workArea), &workArea, 0);

	Ed_uiFont = CreateFontIndirect(&ncm.lfMessageFont);

	if (!GUI_InitAssetManager(workArea.left, workArea.top + *E_screenHeight + ncm.iCaptionHeight, 600, 290))
		return false;

	if (!GUI_InitSceneHierarchy(workArea.left, workArea.top, 250, *E_screenHeight))
		return false;

	Sys_MoveWindow(workArea.left + 250, workArea.top);

	if (!GUI_InitInspector(*E_screenWidth + workArea.left + 250, 0, 250, *E_screenHeight - 31))
		return false;

	GUI_ShowAssetManager();

	return true;
}

void
EdGUI_ProcessEvents(void)
{
/*	MSG msg;

	if (!_workingDialog)
		return;

	while (PeekMessageW(&msg, _workingDialog, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}*/
}

void
EdGUI_MessageBox(const char *title, const char *message)
{
	MessageBoxA((HWND)E_screen, message, title, MB_OK);
}

void
EdGUI_ShowProgressDialog(const char *text)
{
	_workingDialog = CreateDialog(Win32_instance, MAKEINTRESOURCE(IDD_PROGRESS), (HWND)E_screen, _ProgressDlgProc);
	ShowWindow(_workingDialog, SW_SHOW);

	SetWindowText(_dialogText, NeWin32_UTF8toUCS2(text));
}

void
EdGUI_UpdateProgressDialog(const char *text)
{
	SetWindowText(_dialogText, NeWin32_UTF8toUCS2(text));
}

void
EdGUI_HideProgressDialog(void)
{
	EndDialog(_workingDialog, 0);
}

void
Ed_TermGUI(void)
{
	GUI_TermSceneHierarchy();
	GUI_TermAssetManager();
	GUI_TermInspector();

	DeleteObject(Ed_uiFont);
}

INT_PTR CALLBACK
_ProgressDlgProc(HWND hdlg, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg) {
	case WM_INITDIALOG: {
		_dialogText = GetDlgItem(hdlg, IDC_DIALOG_TEXT);
		SendDlgItemMessage(hdlg, IDC_PROGRESS, PBM_SETMARQUEE, TRUE, 0);
	} break;
	case WM_COMMAND: {
		EndDialog(hdlg, wparam);
		return TRUE;
	} break;
	}
	return FALSE;
}
