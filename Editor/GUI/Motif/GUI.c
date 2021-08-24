#include "MotifGUI.h"
#include "Inspector.h"
#include "AssetManager.h"
#include "SceneHierarchy.h"

#include <Xm/MessageB.h>

#include <Editor/GUI.h>
#include <Editor/Editor.h>
#include <Engine/Engine.h>
#include <System/Window.h>

XtAppContext Ed_appContext;
Widget Ed_appShell;

static Widget _workingDialog;

bool
Ed_CreateGUI(void)
{
	int argc = 0;
	Ed_appShell = XtVaOpenApplication(&Ed_appContext, "NekoEditorApp", NULL, 0,
					&argc, NULL, NULL, sessionShellWidgetClass,
					XmNtitle, "Asset Manager",
					NULL);


	if (!GUI_InitAssetManager(0, 600, 600, 300))
		return false;

	if (!GUI_InitSceneHierarchy(0, 0, 250, *E_screenHeight))
		return false;

	Sys_MoveWindow(250, 0);

	if (!GUI_InitInspector(*E_screenWidth + 250, 0, 250, *E_screenHeight - 31))
		return false;

	_workingDialog = XmCreateWorkingDialog(Ed_appShell, "workingDialog", NULL, 0);
	XtUnmanageChild(XtNameToWidget(_workingDialog, "OK"));
    XtUnmanageChild(XtNameToWidget(_workingDialog, "Help"));
	XtUnmanageChild(XtNameToWidget(_workingDialog, "Cancel"));

	XtRealizeWidget(Ed_appShell);

	return true;
}

void
EdGUI_ProcessEvents(void)
{
	XtAppLock(Ed_appContext);

	while (XtAppPending(Ed_appContext))
		XtAppProcessEvent(Ed_appContext, XtIMAll);

	XtAppUnlock(Ed_appContext);
}

void
EdGUI_MessageBox(const char *title, const char *message)
{
	XtAppLock(Ed_appContext);

	XmString xms = XmStringCreateLocalized((char *)message);

	Widget dialog = XmCreateMessageBox(Ed_appShell, "messageBox", NULL, 0);
	XtVaSetValues(XtParent(dialog), XmNtitle, title, NULL);
	XtVaSetValues(dialog, XmNmessageString, xms, NULL);

	XmStringFree(xms);

	XtManageChild(dialog);

	XtAppUnlock(Ed_appContext);
}

void
EdGUI_ShowProgressDialog(const char *text)
{
	XtAppLock(Ed_appContext);

	XmString str = XmStringCreateLocalized((char *)text);
	XtVaSetValues(_workingDialog, XmNmessageString, str, NULL);
	XtManageChild(_workingDialog);

	XtAppUnlock(Ed_appContext);
}

void
EdGUI_UpdateProgressDialog(const char *text)
{
	XtAppLock(Ed_appContext);

	XmString str = XmStringCreateLocalized((char *)text);
	XtVaSetValues(_workingDialog, XmNmessageString, str, NULL);

	XtAppUnlock(Ed_appContext);
}

void
EdGUI_HideProgressDialog(void)
{
	XtAppLock(Ed_appContext);
	XtUnmanageChild(_workingDialog);
	XtAppUnlock(Ed_appContext);
}

void
Ed_TermGUI(void)
{
	XtAppLock(Ed_appContext);
	XtUnmanageChild(_workingDialog);
	XtDestroyWidget(_workingDialog);
	XtAppUnlock(Ed_appContext);
}
