#include "GTKGUI.h"
#include "Inspector.h"
#include "SceneHierarchy.h"

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

struct TreeFindArgs
{
	NeEntityHandle handle;
	GtkTreeIter iter;
};

static uint64_t _sceneActivatedHandler, _entityCreateHandler, _entityDestroyHandler,
	_componentCreateHandler;
static NeFutex _ftx;

static void _EntityCreate(void *user, NeEntityHandle eh);
static void _EntityDestroy(void *user, NeEntityHandle eh);
static void _ComponentCreate(void *user, const struct NeComponentCreationData *ccd);
static void _SceneActivated(void *user, struct NeScene *scn);
static void _AddTransform(const struct NeTransform *xform, GtkTreeIter *parent);
static inline GtkTreeIter _AddNode(const char *name, GtkTreeIter *parent, NeEntityHandle handle);
static inline bool _FindNode(NeEntityHandle handle, GtkTreeIter *iter);

static void _Close(GtkWindow *wnd, gpointer user);
static void _RowActivated(GtkTreeView *self, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user);
static gboolean _TreeForEach(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data);

static GtkWidget *_wnd, *_treeView;
static GtkTreeStore *_tree;

bool
GUI_InitSceneHierarchy(int x, int y, int width, int height)
{
	_wnd = gtk_application_window_new(Ed_gtkApplication);
	gtk_window_set_title(GTK_WINDOW(_wnd), "Scene Hierarchy");
	gtk_window_set_default_size(GTK_WINDOW(_wnd), width, height);

	_tree = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);

	_treeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(_tree));
	gtk_widget_set_margin_top(_treeView, 5);
	gtk_widget_set_margin_start(_treeView, 5);
	gtk_widget_set_margin_end(_treeView, 5);
	gtk_widget_set_margin_bottom(_treeView, 5);

	gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(_treeView), true);
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(_treeView), 0, "Entity", gtk_cell_renderer_text_new(), "text", 0, NULL);

	g_signal_connect(_treeView, "row-activated", G_CALLBACK(_RowActivated), NULL);
	g_signal_connect(_wnd, "close-request", G_CALLBACK(_Close), NULL);

	gtk_window_set_child(GTK_WINDOW(_wnd), _treeView);
	gtk_widget_show(_wnd);

	GUI_MoveWindow(GTK_WINDOW(_wnd), x, y);

	Sys_InitFutex(&_ftx);

	_sceneActivatedHandler = E_RegisterHandler(EVT_SCENE_ACTIVATED, (NeEventHandlerProc)_SceneActivated, NULL);
	_entityCreateHandler = E_RegisterHandler(EVT_ENTITY_CREATED, (NeEventHandlerProc)_EntityCreate, NULL);
	_entityDestroyHandler = E_RegisterHandler(EVT_ENTITY_DESTROYED, (NeEventHandlerProc)_EntityDestroy, NULL);
	_componentCreateHandler = E_RegisterHandler(EVT_COMPONENT_CREATED, (NeEventHandlerProc)_ComponentCreate, NULL);

	_SceneActivated(NULL, Scn_activeScene);

	return true;
}

void
GUI_ShowSceneHierarchy(void)
{
	gtk_window_present(GTK_WINDOW(_wnd));
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

	GtkTreeIter iter;
	if (_FindNode(eh, &iter))
		return;

	GtkTreeIter *parentIter = NULL;
	if (xform->parent != E_INVALID_HANDLE) {
		struct NeTransform *parent = E_ComponentPtrS(Scn_GetScene((uint8_t)xform->_sceneId), xform->parent);
		if (_FindNode(parent->_owner, &iter))
			parentIter = &iter;
	}
	_AddTransform(xform, parentIter);
}

static void
_EntityDestroy(void *user, NeEntityHandle eh)
{
	Sys_LockFutex(_ftx);

	GtkTreeIter iter;
	if (!_FindNode(eh, &iter))
		return;

	gtk_tree_store_remove(_tree, &iter);

	Sys_UnlockFutex(_ftx);
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

	Sys_LockFutex(_ftx);
	{
		gtk_tree_store_clear(_tree);

		GtkTreeViewColumn *col = gtk_tree_view_get_column(GTK_TREE_VIEW(_treeView), 0);
		gtk_tree_view_column_set_title(col, scn->name);

	}
	Sys_UnlockFutex(_ftx);
	
	Rt_ArrayForEach(xform, transforms)
		if (xform->parent == E_INVALID_HANDLE)
			_AddTransform(xform, NULL);
}

static void
_AddTransform(const struct NeTransform *xform, GtkTreeIter *parent)
{
	GtkTreeIter iter = _AddNode(E_EntityName(xform->_owner), parent, xform->_owner);

	const struct NeTransform *child;
	Rt_ArrayForEach(child, &xform->children)
		_AddTransform(child, &iter);
}

static inline GtkTreeIter
_AddNode(const char *name, GtkTreeIter *parent, NeEntityHandle handle)
{
	GtkTreeIter iter;
	
	Sys_LockFutex(_ftx);
	{
		gtk_tree_store_append(_tree, &iter, parent);
		gtk_tree_store_set(_tree, &iter, 0, name, 1, handle, -1);
	}
	Sys_UnlockFutex(_ftx);

	return iter;
}

static inline bool
_FindNode(NeEntityHandle handle, GtkTreeIter *iter)
{
	struct TreeFindArgs tfa = { .handle = handle };
	
	Sys_LockFutex(_ftx);
	{
		gtk_tree_model_foreach(GTK_TREE_MODEL(_tree), _TreeForEach, &tfa);
	}
	Sys_UnlockFutex(_ftx);

	return gtk_tree_store_iter_is_valid(_tree, &tfa.iter);
}

static void
_Close(GtkWindow *wnd, gpointer user)
{
	gtk_widget_hide(GTK_WIDGET(wnd));
}

static void
_RowActivated(GtkTreeView *self, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user)
{
	GtkTreeIter iter;
	NeEntityHandle handle = NULL;

	gtk_tree_model_get_iter(GTK_TREE_MODEL(_tree), &iter, path);
	gtk_tree_model_get(GTK_TREE_MODEL(_tree), &iter, 1, &handle, -1);

	GUI_InspectEntity(handle);
}

static gboolean
_TreeForEach(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	NeEntityHandle handle = NULL;
	struct TreeFindArgs *args = data;

	gtk_tree_model_get(model, iter, 1, &handle, -1);

	if (handle != args->handle)
		return false;

	memcpy(&args->iter, iter, sizeof(args->iter));

	return true;
}
