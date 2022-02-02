#ifndef _EDITOR_WIN32_GUI_H_
#define _EDITOR_WIN32_GUI_H_

#define VC_EXTRALEAN

#include <Windows.h>
#include <Engine/Types.h>

#define ED_AM_WND_CLASS_NAME	L"NekoEditorAssetManagerWindowClass"
#define ED_INS_WND_CLASS_NAME	L"NekoEditorInspectorWindowClass"
#define ED_SH_WND_CLASS_NAME	L"NekoEditorSceneHierarchyWindowClass"

extern HWND Ed_mainWindow;
extern HFONT Ed_uiFont;
extern HINSTANCE Win32_instance;

// Implemented in Platform
wchar_t *NeWin32_UTF8toUCS2(const char *text);

#endif /* _EDITOR_WIN32_GUI_H_ */
