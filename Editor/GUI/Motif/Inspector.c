#include "MotifGUI.h"
#include "Inspector.h"

#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/PanedW.h>
#include <Xm/ScrolledW.h>
#include <Xm/RowColumn.h>

#include <Scene/Scene.h>
#include <Engine/Entity.h>
#include <Engine/Component.h>
#include <Runtime/Runtime.h>

static Widget _titleLabel, _wnd;

void
GUI_InspectEntity(EntityHandle handle)
{
	XmString str = NULL;

	if (handle != ES_INVALID_ENTITY) {
		str = XmStringCreateLocalized(Rt_WcsToMbs(E_EntityName(handle)));
	} else {
		str = XmStringCreateLocalized("No selection");
	}

	XtVaSetValues(_titleLabel, XmNlabelString, str, NULL);
	XmStringFree(str);
}

void
GUI_InspectScene(void)
{
	XmString str = XmStringCreateLocalized(Rt_WcsToMbs(Scn_activeScene->name));
	XtVaSetValues(_titleLabel, XmNlabelString, str, NULL);
	XmStringFree(str);
}

bool
GUI_InitInspector(int x, int y, int width, int height)
{
	_wnd = XtVaCreatePopupShell("Inspector", topLevelShellWidgetClass, Ed_appShell, XmNx, x, XmNy, y, NULL);
	Widget paned = XtVaCreateManagedWidget("form", xmPanedWindowWidgetClass, _wnd,
			XmNscrollBarDisplayPolicy, XmAS_NEEDED,
			XmNscrollingPolicy, XmAUTOMATIC,
			XmNwidth, width, XmNheight, height,
			NULL);

	XmString str = XmStringCreateLocalized("No selection");
	_titleLabel = XtVaCreateManagedWidget("titleLabel", xmLabelWidgetClass, paned,
											XmNlabelString, str,
											NULL);
	XmStringFree(str);

	Widget scroll = XtVaCreateManagedWidget("componentsScrollWindow", xmScrolledWindowWidgetClass, paned,
			XmNscrollBarDisplayPolicy, XmAS_NEEDED,
			XmNscrollingPolicy, XmAUTOMATIC,
			XmNwidth, width, XmNheight, height,
			NULL);

	Widget componentsArea = XtVaCreateManagedWidget("componentsArea", xmRowColumnWidgetClass, scroll, NULL);

	{
		Widget componentRoot = XtVaCreateManagedWidget("xformForm", xmRowColumnWidgetClass, componentsArea, NULL);

		str = XmStringCreateLocalized("Transform");
		XtVaCreateManagedWidget("xformTitle", xmLabelWidgetClass, componentRoot,
												XmNlabelString, str,
												NULL);
		XmStringFree(str);

		XtManageChild(XmCreatePushButton(componentRoot, "button", NULL, 0));
	}

	{
		Widget componentRoot = XtVaCreateManagedWidget("mrForm", xmRowColumnWidgetClass, componentsArea, NULL);

		str = XmStringCreateLocalized("Model Render");
		XtVaCreateManagedWidget("mrTitle", xmLabelWidgetClass, componentRoot,
												XmNlabelString, str,
												NULL);
		XmStringFree(str);

		XtManageChild(XmCreatePushButton(componentRoot, "button", NULL, 0));
	}

	XtPopup(_wnd, XtGrabNone);

	return true;

	return true;
}

void
GUI_ShowInspector(void)
{
}

void
GUI_TermInspector()
{
}
