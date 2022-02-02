#include "MotifGUI.h"
#include "AboutDialog.h"
#include "AssetManager.h"

#include <Engine/IO.h>
#include <Engine/Engine.h>
#include <System/Log.h>
#include <Runtime/Runtime.h>
#include <Editor/Asset/Asset.h>
#include <Editor/Asset/Import.h>

#include <Xm/Form.h>
#include <Xm/List.h>
#include <Xm/MainW.h>
#include <Xm/PushB.h>
#include <Xm/FileSB.h>
#include <Xm/RowColumn.h>
#include <X11/IntrinsicP.h>

static Widget _wnd, _list;
static char _currentPath[4096] = { '/', 0x0 };
static const char **_fileList;
static struct NeArray _items;

static void _updateList(void);

static Widget _CreateMenu(Widget wnd);
static Widget _CreateWorkArea(Widget wnd);

static void _FileCallback(Widget w, XtPointer client, XtPointer call);
static void _ProjectCallback(Widget w, XtPointer client, XtPointer call);
static void _ToolsCallback(Widget w, XtPointer client, XtPointer call);
static void _HelpCallback(Widget w, XtPointer client, XtPointer call);
static void _ListAction(Widget w, XtPointer client, XtPointer call);
static void _ImportAsset(Widget w, XtPointer client, XtPointer call);
static void _DoImport(Widget w, XtPointer client, XtPointer call);
static void _CancelImport(Widget w, XtPointer client, XtPointer call);

bool
GUI_InitAssetManager(int x, int y, int width, int height)
{
	_wnd = XmVaCreateMainWindow(Ed_appShell, "mainWindow",
				XmNwidth, width, XmNheight, height,
				NULL);

	if (!_wnd)
		return false;

	Widget menuBar = _CreateMenu(_wnd);
	Widget workArea = _CreateWorkArea(_wnd);

	XtVaSetValues(_wnd, XmNmenuBar, menuBar, XmNworkWindow, workArea, NULL);
	XtVaSetValues(Ed_appShell, XmNx, x, XmNy, y, NULL);

	XtManageChild(_wnd);
	XtManageChild(menuBar);
	XtManageChild(workArea);

	XtResizeWidget(_wnd, width, height, 0);

	Rt_InitPtrArray(&_items, 10, MH_System);
	_updateList();

	_wnd = Ed_appShell;

	return true;
}

void
GUI_ShowAssetManager(void)
{
}

void
GUI_TermAssetManager(void)
{
	XtDestroyWidget(_wnd);
}

static void
_updateList(void)
{
	if (_fileList)
		E_FreeFileList(_fileList);

	_fileList = E_ListFiles(_currentPath);

	Rt_ClearArray(&_items, false);
	XmListDeleteAllItems(_list);

#define ADD_ITEM(x) \
	Rt_ArrayAddPtr(&_items, x); \
	XmString str = XmStringCreateLocalized(x); \
	XmListAddItemUnselected(_list, str, 0); \
	XmStringFree(str)

	if (strlen(_currentPath) > 1) {
		ADD_ITEM("..");
	}

	for (const char **i = _fileList; *i != NULL; ++i) {
		if (*i[0] == '.')
			continue;

		ADD_ITEM((char *)*i);
	}
}

Widget
_CreateMenu(Widget wnd)
{
	XmString strings[6];

	strings[0] = XmStringCreateLocalized("File");
	strings[1] = XmStringCreateLocalized("Project");
	strings[2] = XmStringCreateLocalized("Tools");
	strings[3] = XmStringCreateLocalized("Help");

	Widget menuBar = XmVaCreateSimpleMenuBar(wnd, "mainMenu",
						XmVaCASCADEBUTTON, strings[0], 'F',
						XmVaCASCADEBUTTON, strings[1], 'P',
						XmVaCASCADEBUTTON, strings[2], 'T',
						XmVaCASCADEBUTTON, strings[3], 'H',
						NULL);

	Widget w = XtNameToWidget(menuBar, "button_3");
	if (w)
		XtVaSetValues(menuBar, XmNmenuHelpWidget, w, NULL);

	#pragma unroll(4)
	for (uint32_t i = 0; i < 4; ++i)
		XmStringFree(strings[i]);

	strings[0] = XmStringCreateLocalized("New");
	strings[1] = XmStringCreateLocalized("Open");
	strings[2] = XmStringCreateLocalized("Save");
	strings[3] = XmStringCreateLocalized("Close");
    strings[4] = XmStringCreateLocalized("Quit");
    XmVaCreateSimplePulldownMenu(menuBar, "fileMenu", 0, _FileCallback,
								XmVaPUSHBUTTON, strings[0], 'N', NULL, NULL,
								XmVaSEPARATOR,
								XmVaPUSHBUTTON, strings[1], 'O', NULL, NULL,
								XmVaPUSHBUTTON, strings[2], 'S', NULL, NULL,
								XmVaPUSHBUTTON, strings[3], 'W', NULL, NULL,
								XmVaSEPARATOR,
								XmVaPUSHBUTTON, strings[4], 'Q', NULL, NULL,
								NULL);

	#pragma unroll(5)
	for (uint32_t i = 0; i < 5; ++i)
		XmStringFree(strings[i]);

	strings[0] = XmStringCreateLocalized("Settings");
    XmVaCreateSimplePulldownMenu(menuBar, "projectMenu", 1, _ProjectCallback,
								XmVaPUSHBUTTON, strings[0], 'N', NULL, NULL,
								NULL);

	#pragma unroll(1)
	for (uint32_t i = 0; i < 1; ++i)
		XmStringFree(strings[i]);

	strings[0] = XmStringCreateLocalized("Scene Hierarchy");
	strings[1] = XmStringCreateLocalized("Inspector");
	strings[2] = XmStringCreateLocalized("Preferences");
    XmVaCreateSimplePulldownMenu(menuBar, "toolsMenu", 2, _ToolsCallback,
								XmVaPUSHBUTTON, strings[0], 'N', NULL, NULL,
								XmVaPUSHBUTTON, strings[1], 'O', NULL, NULL,
								XmVaSEPARATOR,
								XmVaPUSHBUTTON, strings[2], 'Q', NULL, NULL,
								NULL);

	#pragma unroll(2)
	for (uint32_t i = 0; i < 3; ++i)
		XmStringFree(strings[i]);

	strings[0] = XmStringCreateLocalized("Native API Reference");
	strings[1] = XmStringCreateLocalized("Scripting API Reference");
	strings[2] = XmStringCreateLocalized("About");
    XmVaCreateSimplePulldownMenu(menuBar, "helpMenu", 3, _HelpCallback,
								XmVaPUSHBUTTON, strings[0], 'N', NULL, NULL,
								XmVaPUSHBUTTON, strings[1], 'O', NULL, NULL,
								XmVaSEPARATOR,
								XmVaPUSHBUTTON, strings[2], 'Q', NULL, NULL,
								NULL);

	#pragma unroll(2)
	for (uint32_t i = 0; i < 3; ++i)
		XmStringFree(strings[i]);

	return menuBar;
}

Widget
_CreateWorkArea(Widget wnd)
{
	Widget workArea = XmCreateRowColumn(wnd, "workArea", NULL, 0);
	Widget form = XmCreateForm(workArea, "topBar", NULL, 0);
	Widget importButton = XmCreatePushButton(form, "importButton", NULL, 0);

	XmString buttonText = XmStringCreateLocalized("Import");
	XtVaSetValues(importButton, XmNlabelString, buttonText, NULL);

	_list = XmCreateScrolledList(workArea, "assetList", NULL, 0);

	XtVaSetValues(_list, XmNvisibleItemCount, 14, XmNselectionPolicy, XmSINGLE_SELECT, NULL);
	XtAddCallback(_list, XmNdefaultActionCallback, _ListAction, NULL);
	XtAddCallback(importButton, XmNactivateCallback, _ImportAsset, NULL);

	XtManageChild(form);
	XtManageChild(importButton);
	XtManageChild(_list);

	return workArea;
}

static void
_FileCallback(Widget w, XtPointer client, XtPointer call)
{
	switch ((intptr_t)client) {
	case 0: break;
	case 1: break;
	case 2: break;
	case 3: break;
	case 4: E_Shutdown(); break;
	}
}

static void
_ProjectCallback(Widget w, XtPointer client, XtPointer call)
{
}

static void
_ToolsCallback(Widget w, XtPointer client, XtPointer call)
{
}

static void
_HelpCallback(Widget w, XtPointer client, XtPointer call)
{
	switch ((intptr_t)client) {
	case 0: break;
	case 1: break;
	case 2: GUI_AboutDialog(); break;
	}
}

static void
_ListAction(Widget w, XtPointer client, XtPointer call)
{
	int *selected, count;
	char newPath[4096] = { 0x0 };

	XmListGetSelectedPos(_list, &selected, &count);
	if (!count)
		return;

	const char *item = Rt_ArrayGetPtr(&_items, selected[0] - 1);

	if (!strncmp(item, "..", strlen(item))) {
		char *p = strrchr(_currentPath, '/');
		size_t pos = p - _currentPath;

		if (pos) {
			_currentPath[pos] = 0x0;
			memcpy(newPath, _currentPath, pos);
		} else {
			newPath[0] = '/';
		}
	} else {
		snprintf(newPath, sizeof(newPath), strlen(_currentPath) > 1 ? "%s/%s" : "%s%s", _currentPath, item);
	}

	if (E_IsDirectory(newPath)) {
		memcpy(_currentPath, newPath, sizeof(_currentPath));
		_updateList();
	} else {
		Ed_OpenAsset(newPath);
	}

	XtFree((void *)selected);
}

static void
_ImportAsset(Widget w, XtPointer client, XtPointer call)
{
	XmString title = XmStringCreateLocalized("Select Asset File");

	Arg args[2];
	XtSetArg(args[0], XmNpathMode, XmPATH_MODE_FULL);
	XtSetArg(args[1], XmNdialogTitle, title);

	Widget dialog = XmCreateFileSelectionDialog(Ed_appShell, "filesb", args, 2);

	XtAddCallback(dialog, XmNokCallback, _DoImport, (XtPointer)dialog);
	XtAddCallback(dialog, XmNcancelCallback, _CancelImport, (XtPointer)dialog);

	XtUnmanageChild(XmFileSelectionBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

	XtManageChild(dialog);

	do {
		XtAppProcessEvent(XtWidgetToApplicationContext(dialog), XtIMAll);
	} while (XtIsManaged(dialog));

	XtDestroyWidget(dialog);
	XmStringFree(title);
}

static void
_DoImport(Widget w, XtPointer client, XtPointer call)
{
	XmFileSelectionBoxCallbackStruct *cbs = (XmFileSelectionBoxCallbackStruct *)call;

	char *file = XmStringUnparse(cbs->value, XmFONTLIST_DEFAULT_TAG, XmCHARSET_TEXT, XmCHARSET_TEXT, NULL, 0, XmOUTPUT_ALL);
	if (!file)
		return;

	if (strlen(file))
		Asset_Import(file);

	XtFree(file);

	XtUnmanageChild((Widget)client);
}

static void
_CancelImport(Widget w, XtPointer client, XtPointer call)
{
	XtUnmanageChild((Widget)client);
}
