#include "GTKGUI.h"
#include "AboutDialog.h"

#include "../../resource.h"

#include <stdio.h>

#include <Engine/Engine.h>
#include <Engine/Version.h>

/*INT_PTR CALLBACK
_AboutDlgProc(HWND hdlg, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg) {
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hdlg, &ps);

		HICON icon = LoadImage(Win32_instance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 128, 128, LR_DEFAULTCOLOR);
		
		DrawIconEx(hdc, 20, 20, icon, 128, 128, 0, 0, DI_NORMAL);

		EndPaint(hdlg, &ps);

		DeleteObject(icon);
	} break;
	case WM_INITDIALOG: {
		wchar_t buff[512];

		swprintf(buff, sizeof(buff) / sizeof(wchar_t), L"Version: %hs \"%hs\"", E_VER_STR, E_CODENAME);
		SendDlgItemMessage(hdlg, IDC_ABOUT_VERSION, WM_SETTEXT, 0, (LPARAM)buff);

		swprintf(buff, sizeof(buff) / sizeof(wchar_t), L"Copyright (c) %hs", E_CPY_STR);
		SendDlgItemMessage(hdlg, IDC_ABOUT_COPYRIGHT, WM_SETTEXT, 0, (LPARAM)buff);
	} break;
	case WM_COMMAND: {
		EndDialog(hdlg, wparam);
		return TRUE;
	} break;
	}
	return FALSE;
}*/

void
GUI_AboutDialog(void)
{
	const char *authors[] = { "Alexandru Naiman", NULL };

	gtk_show_about_dialog(NULL,
		"program-name", "NekoEditor",
		"version", E_VER_STR,
		"copyright", E_CPY_STR,
		"license-type", GTK_LICENSE_BSD_3,
		"comments", "NekoEngine Editor",
		"authors", authors,
		"title", "About NekoEditor",
		"logo-icon-name", "neko-editor",
		NULL);
}
