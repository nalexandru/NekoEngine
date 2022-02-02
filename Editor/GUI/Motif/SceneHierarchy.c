#include "MotifGUI.h"
#include "Inspector.h"
#include "SceneHierarchy.h"

#include <Xm/IconG.h>
#include <Xm/Container.h>

#include <uthash.h>

#include <System/Log.h>
#include <Engine/Events.h>
#include <Engine/Entity.h>
#include <Engine/Component.h>
#include <Scene/Components.h>
#include <Scene/Transform.h>
#include <Scene/Scene.h>
#include <Runtime/Runtime.h>
#include <System/Thread.h>

struct TreeNode
{
	void *widget;
	NeEntityHandle handle;
	UT_hash_handle hh;
};

static Widget _container, _wnd, _inspectedItem, _treeRoot;
static uint64_t _sceneActivatedHandler, _entityCreateHandler, _entityDestroyHandler,
	_componentCreateHandler;
static struct TreeNode *_nodes = NULL;
static struct NeArray _nodeArray;
static NeFutex _ftx;

static void _EntityCreate(void *user, NeEntityHandle eh);
static void _EntityDestroy(void *user, NeEntityHandle eh);
static void _ComponentCreate(void *user, const struct NeComponentCreationData *ccd);
static void _SceneActivated(void *user, struct NeScene *scn);
static void _AddTransform(const struct NeTransform *xform, Widget parentNode);
static inline Widget _AddNode(const char *name, Widget parentNode, NeEntityHandle handle);
static inline struct TreeNode *_FindNode(NeEntityHandle handle);
static void _ItemSelected(Widget w, XtPointer client, XtPointer call);

bool
GUI_InitSceneHierarchy(int x, int y, int width, int height)
{
	_wnd = XtVaCreatePopupShell("Scene Hierarchy", topLevelShellWidgetClass, Ed_appShell, XmNx, x, XmNy, y, NULL);
	_container = XtVaCreateManagedWidget("form", xmContainerWidgetClass, _wnd,
											XmNlayoutType, XmOUTLINE,
											XmNselectionPolicy, XmSINGLE_SELECT,
											XmNwidth, width, XmNheight, height,
											NULL);

	XtAddCallback(_container, XmNselectionCallback, _ItemSelected, NULL);

	XtPopup(_wnd, XtGrabNone);

	Sys_InitFutex(&_ftx);

	Rt_InitArray(&_nodeArray, 10, sizeof(struct TreeNode), MH_System);
	_sceneActivatedHandler = E_RegisterHandler(EVT_SCENE_ACTIVATED, (NeEventHandlerProc)_SceneActivated, NULL);
	_entityCreateHandler = E_RegisterHandler(EVT_ENTITY_CREATED, (NeEventHandlerProc)_EntityCreate, NULL);
	_entityDestroyHandler = E_RegisterHandler(EVT_ENTITY_DESTROYED, (NeEventHandlerProc)_EntityDestroy, NULL);
	_componentCreateHandler = E_RegisterHandler(EVT_COMPONENT_CREATED, (NeEventHandlerProc)_ComponentCreate, NULL);

	return true;
}

void
GUI_ShowSceneHierarchy(void)
{
}

void
GUI_TermSceneHierarchy(void)
{
	E_UnregisterHandler(_sceneActivatedHandler);
	E_UnregisterHandler(_entityCreateHandler);
	E_UnregisterHandler(_entityDestroyHandler);
	E_UnregisterHandler(_componentCreateHandler);

	Sys_TermFutex(_ftx);
}

static void
_EntityCreate(void *user, NeEntityHandle eh)
{
	const struct NeTransform *xform = E_GetComponent(eh, E_ComponentTypeId(TRANSFORM_COMP));
	if (!xform)
		return;

	Widget parent = NULL;
	if (!xform->parent)
		parent = _treeRoot;
	else
		parent = _FindNode(xform->parent->_owner)->widget;

	_AddTransform(xform, parent);
}

static void
_EntityDestroy(void *user, NeEntityHandle eh)
{
	struct TreeNode *node = _FindNode(eh);
	if (!node)
		return;

	XtUnmanageChild(node->widget);
	XtDestroyWidget(node->widget);

	HASH_DEL(_nodes, node);

	// TODO free
}

static void
_ComponentCreate(void *user, const struct NeComponentCreationData *ccd)
{
	if (ccd->type != E_ComponentTypeId(TRANSFORM_COMP))
		return;

	_EntityCreate(NULL, ccd->owner);
}

static void
_SceneActivated(void *user, struct NeScene *scn)
{
	const struct NeArray *transforms = E_GetAllComponentsS(scn, E_ComponentTypeId(TRANSFORM_COMP));
	const struct NeTransform *xform = NULL;

	Rt_ClearArray(&_nodeArray, false);

	_treeRoot = _AddNode(scn->name, NULL, ES_INVALID_ENTITY);
	Rt_ArrayForEach(xform, transforms)
		if (!xform->parent)
			_AddTransform(xform, _treeRoot);
}

static void
_AddTransform(const struct NeTransform *xform, Widget parentNode)
{
	Widget node = _AddNode(E_EntityName(xform->_owner), parentNode, xform->_owner);

	const struct NeTransform *child;
	Rt_ArrayForEach(child, &xform->children)
		_AddTransform(child, node);
}

static inline Widget
_AddNode(const char *name, Widget parentNode, NeEntityHandle handle)
{
	XtAppLock(Ed_appContext);
	Sys_LockFutex(_ftx);

	struct TreeNode *tn = _FindNode(handle);
	Widget node = tn ? tn->widget : NULL;

	if (node)
		goto exit;

	Sys_LogEntry("MotifSceneHierarchy", LOG_DEBUG, "AddNode: %ls | %p", name, handle);

	XmString str = XmStringCreateLocalized((char *)name);
	node = XtVaCreateManagedWidget("iconGadget", xmIconGadgetClass, _container,
									XmNentryParent, parentNode,
									XmNlabelString, str,
									NULL);
	XmStringFree(str);

	tn = Rt_ArrayAllocate(&_nodeArray);
	tn->widget = node;
	tn->handle = handle;
	HASH_ADD_PTR(_nodes, widget, tn);

exit:
	Sys_UnlockFutex(_ftx);
	XtAppUnlock(Ed_appContext);

	return node;
}

static inline struct TreeNode *
_FindNode(NeEntityHandle handle)
{
	struct TreeNode *node = _nodes;
	while (node) {
		if (node->handle == handle)
			return node;

		node = node->hh.next;
	}
	return NULL;
}

static void
_ItemSelected(Widget w, XtPointer client, XtPointer call)
{
	XmContainerSelectCallbackStruct *cs = call;

	if (!cs->selected_item_count) {
		GUI_InspectEntity(ES_INVALID_ENTITY);
		_inspectedItem = NULL;
		return;
	}

	Widget selectedItem = cs->selected_items[0];
	if (selectedItem == _inspectedItem)
		return;

	struct TreeNode *node;
	HASH_FIND_PTR(_nodes, &selectedItem, node);

	if (node->handle != ES_INVALID_ENTITY)
		GUI_InspectEntity(node->handle);
	else
		GUI_InspectScene();

	_inspectedItem = selectedItem;
}
