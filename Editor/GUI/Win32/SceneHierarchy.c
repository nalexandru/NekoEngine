#include "Win32GUI.h"
#include "Inspector.h"
#include "SceneHierarchy.h"

#include <dwmapi.h>
#include <CommCtrl.h>

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

#define SH_TREE_VIEW		60000

struct TreeNode
{
	HTREEITEM item;
	NeEntityHandle handle;
	UT_hash_handle hh;
};

static uint64_t _sceneActivatedHandler, _entityCreateHandler, _entityDestroyHandler,
	_componentCreateHandler;
static struct TreeNode *_nodes = NULL;
static struct NeArray _nodeArray;
static NeFutex _ftx;

static void _EntityCreate(void *user, NeEntityHandle eh);
static void _EntityDestroy(void *user, NeEntityHandle eh);
static void _ComponentCreate(void *user, const struct NeComponentCreationData *ccd);
static void _SceneActivated(void *user, struct NeScene *scn);
static void _AddTransform(const struct NeTransform *xform, HTREEITEM parentNode);
static inline HTREEITEM _AddNode(const char *name, HTREEITEM parentNode, NeEntityHandle handle);
static inline struct TreeNode *_FindNode(NeEntityHandle handle);
static void _ItemSelected(LPNMTREEVIEW nmtv);

static HWND _wnd, _treeView;
static HTREEITEM _treeRoot, _inspectedItem;

static LRESULT CALLBACK _SHWndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

bool
GUI_InitSceneHierarchy(int x, int y, int width, int height)
{
	DWORD style = WS_VISIBLE | (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME) | WS_CLIPCHILDREN;
	DWORD exStyle = WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_COMPOSITED;
	WNDCLASS wincl =
	{
		.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
		.lpfnWndProc = _SHWndProc,
		.hInstance = Win32_instance,
//		.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1),
		.lpszClassName = ED_SH_WND_CLASS_NAME,
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

	_wnd = CreateWindowExW(exStyle, ED_SH_WND_CLASS_NAME,
		L"Scene Hierarchy", style, x, y, rc.right - rc.left, rc.bottom - rc.top,
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
	ShowWindow(_wnd, SW_SHOW);
	SetForegroundWindow(_wnd);
	SetActiveWindow(_wnd);
}

void
GUI_TermSceneHierarchy(void)
{
	E_UnregisterHandler(_sceneActivatedHandler);
	E_UnregisterHandler(_entityCreateHandler);
	E_UnregisterHandler(_entityDestroyHandler);
	E_UnregisterHandler(_componentCreateHandler);

	Rt_TermArray(&_nodeArray);

	Sys_TermFutex(_ftx);

	DestroyWindow(_wnd);
}

static LRESULT CALLBACK
_SHWndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg) {
	case WM_NOTIFY: {
		LPNMHDR hdr = (LPNMHDR)lparam;

		if (hdr->code == TVN_SELCHANGED)
			_ItemSelected((LPNMTREEVIEW)lparam);
	} break;
	case WM_COMMAND: {

	} break;
	case WM_CREATE: {
		RECT rc;
		GetClientRect(hwnd, &rc);

		const uint32_t w = rc.right - rc.left;
		const uint32_t h = rc.bottom - rc.top;

		_treeView = CreateWindowEx(0, WC_TREEVIEW, L"Scene Hierarchy", WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES,
			10, 10, w - 20, h - 20, hwnd, (HMENU)SH_TREE_VIEW, Win32_instance, NULL);
		SendMessage(_treeView, WM_SETFONT, (WPARAM)Ed_uiFont, 0);
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

		MoveWindow(_treeView, 10, 10, w - 20, h - 20, true);
	} break;
	default: {
	} break;
	}
	
	return DefWindowProc(hwnd, umsg, wparam, lparam);
}

static void
_EntityCreate(void *user, NeEntityHandle eh)
{
	const struct NeTransform *xform = E_GetComponent(eh, E_ComponentTypeId(TRANSFORM_COMP));
	if (!xform)
		return;

	HTREEITEM parent = NULL;
	if (!xform->parent)
		parent = _treeRoot;
	else
		parent = _FindNode(xform->parent->_owner)->item;

	_AddTransform(xform, parent);
}

static void
_EntityDestroy(void *user, NeEntityHandle eh)
{
	struct TreeNode *node = _FindNode(eh);
	if (!node)
		return;

	TreeView_DeleteItem(_treeView, node->item);

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
_AddTransform(const struct NeTransform *xform, HTREEITEM parentNode)
{
	HTREEITEM node = _AddNode(E_EntityName(xform->_owner), parentNode, xform->_owner);

	const struct NeTransform *child;
	Rt_ArrayForEach(child, &xform->children)
		_AddTransform(child, node);
}

static inline HTREEITEM
_AddNode(const char *name, HTREEITEM parentNode, NeEntityHandle handle)
{
	Sys_LockFutex(_ftx);

	struct TreeNode *tn = _FindNode(handle);
	HTREEITEM node = tn ? tn->item : NULL;

	if (node)
		goto exit;

	Sys_LogEntry("Win32SceneHierarchy", LOG_DEBUG, "AddNode: %s | %p", name, handle);

	TVITEM tvi;
	TVINSERTSTRUCT tvins;

	tvi.mask = TVIF_TEXT;
	tvi.pszText = NeWin32_UTF8toUCS2(name);
	tvins.item = tvi;
	tvins.hParent = parentNode ? parentNode : TVI_ROOT;

	node = TreeView_InsertItem(_treeView, &tvins);

	tn = Rt_ArrayAllocate(&_nodeArray);
	tn->item = node;
	tn->handle = handle;
	HASH_ADD_PTR(_nodes, item, tn);

exit:
	Sys_UnlockFutex(_ftx);

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
_ItemSelected(LPNMTREEVIEW nmtv)
{
	Sys_LogEntry("Win32SceneHierarchy", LOG_DEBUG, "_ItemSelected: %p | %p", nmtv->itemOld.hItem, nmtv->itemNew.hItem);

	if (!nmtv->itemNew.hItem) {
		GUI_InspectEntity(ES_INVALID_ENTITY);
		_inspectedItem = NULL;
		return;
	}

	if (nmtv->itemNew.hItem == _inspectedItem)
		return;

	struct TreeNode *node;
	HASH_FIND_PTR(_nodes, &nmtv->itemNew.hItem, node);

	if (node->handle != ES_INVALID_ENTITY)
		GUI_InspectEntity(node->handle);
	else
		GUI_InspectScene();

	_inspectedItem = nmtv->itemNew.hItem;
}
