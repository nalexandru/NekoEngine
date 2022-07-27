#include "GTKGUI.h"
#include "Inspector.h"

#include <Scene/Scene.h>
#include <Engine/Entity.h>
#include <Engine/Component.h>
#include <Runtime/Runtime.h>

#define NE_TYPE_INS_ENTITY_COMP (ne_ins_entity_comp_get_type())
G_DECLARE_FINAL_TYPE(NeInsEntityComp, ne_ins_entity_comp, NE, INS_ENTITY_COMP, GObject)

typedef struct _NeInsEntityComp NeInsEntityComp;
struct _NeInsEntityComp
{
	GObject  parent_instance;

	NeCompTypeId type;
	NeCompHandle handle;
};

static void _Close(GtkWindow *wnd, gpointer user);

static void _SetupListItem(GtkListItemFactory *factory, GtkListItem *listItem);
static void _BindListItem(GtkListItemFactory *factory, GtkListItem *listItem);

static int _Update(void *ptr);

static void _SetupListItemUI(GtkBox *rootLyt, NeCompTypeId typeId, void *ptr);

static GtkWidget *_wnd, *_title, *_list;
static GListStore *_compStore;
static NeEntityHandle _currentEntity;

void
GUI_InspectEntity(NeEntityHandle handle)
{
	_currentEntity = handle;

	if (handle == ES_INVALID_ENTITY) {
		gtk_label_set_text(GTK_LABEL(_title), "No selection");
		return;
	}

	gtk_label_set_text(GTK_LABEL(_title), E_EntityName(handle));

	g_list_store_remove_all(_compStore);

	struct NeArray components;
	E_GetComponents(_currentEntity, &components);

	struct NeEntityComp *c = NULL;
	Rt_ArrayForEach(c, &components) {
		NeInsEntityComp *iec = g_object_new(NE_TYPE_INS_ENTITY_COMP, NULL);

		iec->type = c->type;
		iec->handle = c->handle;

		g_list_store_append(_compStore, iec);
		g_object_unref(iec);
	}
}

void
GUI_InspectScene(void)
{
	gtk_label_set_text(GTK_LABEL(_title), Scn_activeScene->name);
}

bool
GUI_InitInspector(int x, int y, int width, int height)
{
	_wnd = gtk_application_window_new(Ed_gtkApplication);
	if (!_wnd)
		return false;

	gtk_window_set_title(GTK_WINDOW(_wnd), "Inspector");
	gtk_window_set_default_size(GTK_WINDOW(_wnd), width, height);

	GtkWidget *topLyt = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_widget_set_margin_top(topLyt, 5);
	gtk_widget_set_margin_start(topLyt, 5);
	gtk_widget_set_margin_end(topLyt, 5);
	gtk_widget_set_margin_bottom(topLyt, 5);
	gtk_window_set_child(GTK_WINDOW(_wnd), topLyt);

	_title = gtk_label_new("No selection");
	gtk_box_append(GTK_BOX(topLyt), _title);

	GtkWidget *tmp = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_box_append(GTK_BOX(topLyt), tmp);

	GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
	g_signal_connect(factory, "setup", G_CALLBACK(_SetupListItem), NULL);
	g_signal_connect(factory, "bind", G_CALLBACK(_BindListItem), NULL);

	_compStore = g_list_store_new(NE_TYPE_INS_ENTITY_COMP);
	//_cTypes = gtk_string_list_new(NULL);

	GtkNoSelection *ns = gtk_no_selection_new(G_LIST_MODEL(_compStore));

	tmp = gtk_scrolled_window_new();
	gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(tmp), true);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(tmp), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	_list = gtk_list_view_new(GTK_SELECTION_MODEL(ns), factory);
	gtk_widget_set_hexpand(_list, true);
	gtk_widget_set_vexpand(_list, true);

	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(tmp), _list);
	gtk_box_append(GTK_BOX(topLyt), tmp);

	g_signal_connect(_wnd, "close-request", G_CALLBACK(_Close), NULL);

	gtk_widget_show(_wnd);

	GUI_MoveWindow(GTK_WINDOW(_wnd), x, y);

	g_object_unref(ns);

	g_idle_add(G_SOURCE_FUNC(_Update), NULL);

	return true;
}

void
GUI_ShowInspector(void)
{
	gtk_window_present(GTK_WINDOW(_wnd));
}

void
GUI_TermInspector()
{
//	gtk_window_destroy(_wnd);
}

static void
_Close(GtkWindow *wnd, gpointer user)
{
	gtk_widget_hide(GTK_WIDGET(wnd));
}

static void
_SetupListItem(GtkListItemFactory *factory, GtkListItem *listItem)
{
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
//	GtkWidget *checkBox = gtk_check_button_new();

	GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_append(GTK_BOX(row), gtk_check_button_new());
	gtk_box_append(GTK_BOX(row), gtk_label_new(""));

	gtk_box_append(GTK_BOX(box), row);

	//NeInsEntityComp *iec = NE_INS_ENTITY_COMP(gtk_list_item_get_item(listItem));

	gtk_list_item_set_child(listItem, box);
}

static void
_BindListItem(GtkListItemFactory *factory, GtkListItem *listItem)
{
	GtkWidget *box = gtk_list_item_get_child(listItem);
	GtkWidget *row = gtk_widget_get_first_child(box);
	GtkWidget *check = gtk_widget_get_first_child(row);
	GtkWidget *label = gtk_widget_get_last_child(row);

	NeInsEntityComp *iec = NE_INS_ENTITY_COMP(gtk_list_item_get_item(listItem));
	struct NeCompBase *comp = E_ComponentPtr(iec->handle);
	_SetupListItemUI(GTK_BOX(box), iec->type, comp);

	gtk_check_button_set_active(GTK_CHECK_BUTTON(check), (gboolean)comp->_enabled);
	gtk_label_set_text(GTK_LABEL(label), E_ComponentTypeName(iec->type));
}
#include <Engine/Engine.h>
#include <Scene/Transform.h>
GtkEntry *posX, *posY, *posZ;
double nextUpdate = 0.f;
int
_Update(void *ptr)
{
	if (!posX || !posY || !posZ)
		return 0;

	if (E_Time() < nextUpdate)
		return 1;

	char cbuff[64];
	GtkEntryBuffer *buff = NULL;
	struct NeTransform *xform = ptr;

	snprintf(cbuff, sizeof(cbuff), "%.02f", xform->position.x);
	buff = gtk_entry_get_buffer(posX);
	gtk_entry_buffer_set_text(buff, cbuff, (int)strnlen(cbuff, sizeof(cbuff)));

	snprintf(cbuff, sizeof(cbuff), "%.02f", xform->position.y);
	buff = gtk_entry_get_buffer(posY);
	gtk_entry_buffer_set_text(buff, cbuff, (int)strnlen(cbuff, sizeof(cbuff)));

	snprintf(cbuff, sizeof(cbuff), "%.02f", xform->position.z);
	buff = gtk_entry_get_buffer(posZ);
	gtk_entry_buffer_set_text(buff, cbuff, (int)strnlen(cbuff, sizeof(cbuff)));

	nextUpdate = E_Time() + .05;

	return 1;
}

static void
_SetupListItemUI(GtkBox *rootLyt, NeCompTypeId typeId, void *ptr)
{
	GtkWidget *lyt, *tmp;
	GtkEntryBuffer *buff;
	char cbuff[64];

	if (E_ComponentTypeId("Transform") == typeId) {
		struct NeTransform *xform = ptr;

		{ // Position
			lyt = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);

			tmp = gtk_label_new("Position");
			gtk_box_append(GTK_BOX(lyt), tmp);

			tmp = gtk_label_new("X");
			gtk_box_append(GTK_BOX(lyt), tmp);

			snprintf(cbuff, sizeof(cbuff), "%.02f", xform->position.x);
			buff = gtk_entry_buffer_new(cbuff, (int)strnlen(cbuff, sizeof(cbuff)));
			tmp = gtk_entry_new_with_buffer(buff);
			g_object_unref(buff);
			gtk_box_append(GTK_BOX(lyt), tmp);
			posX = GTK_ENTRY(tmp);

			tmp = gtk_label_new("Y");
			gtk_box_append(GTK_BOX(lyt), tmp);

			snprintf(cbuff, sizeof(cbuff), "%.02f", xform->position.y);
			buff = gtk_entry_buffer_new(cbuff, (int)strnlen(cbuff, sizeof(cbuff)));
			tmp = gtk_entry_new_with_buffer(buff);
			g_object_unref(buff);
			gtk_box_append(GTK_BOX(lyt), tmp);
			posY = GTK_ENTRY(tmp);

			tmp = gtk_label_new("Z");
			gtk_box_append(GTK_BOX(lyt), tmp);

			snprintf(cbuff, sizeof(cbuff), "%.02f", xform->position.z);
			buff = gtk_entry_buffer_new(cbuff, (int)strnlen(cbuff, sizeof(cbuff)));
			tmp = gtk_entry_new_with_buffer(buff);
			g_object_unref(buff);
			gtk_box_append(GTK_BOX(lyt), tmp);
			posZ = GTK_ENTRY(tmp);

			gtk_box_append(rootLyt, lyt);
		}

		{ // Rotation
			lyt = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);

			tmp = gtk_label_new("Rotation");
			gtk_box_append(GTK_BOX(lyt), tmp);

			tmp = gtk_label_new("X");
			gtk_box_append(GTK_BOX(lyt), tmp);

			snprintf(cbuff, sizeof(cbuff), "%.02f", M_QuatPitch(&xform->rotation));
			buff = gtk_entry_buffer_new(cbuff, (int)strnlen(cbuff, sizeof(cbuff)));
			tmp = gtk_entry_new_with_buffer(buff);
			g_object_unref(buff);
			gtk_box_append(GTK_BOX(lyt), tmp);

			tmp = gtk_label_new("Y");
			gtk_box_append(GTK_BOX(lyt), tmp);

			snprintf(cbuff, sizeof(cbuff), "%.02f", M_QuatYaw(&xform->rotation));
			buff = gtk_entry_buffer_new(cbuff, (int)strnlen(cbuff, sizeof(cbuff)));
			tmp = gtk_entry_new_with_buffer(buff);
			g_object_unref(buff);
			gtk_box_append(GTK_BOX(lyt), tmp);

			tmp = gtk_label_new("Z");
			gtk_box_append(GTK_BOX(lyt), tmp);

			snprintf(cbuff, sizeof(cbuff), "%.02f", M_QuatRoll(&xform->rotation));
			buff = gtk_entry_buffer_new(cbuff, (int)strnlen(cbuff, sizeof(cbuff)));
			tmp = gtk_entry_new_with_buffer(buff);
			g_object_unref(buff);
			gtk_box_append(GTK_BOX(lyt), tmp);

			gtk_box_append(rootLyt, lyt);
		}

		{ // Scale
			lyt = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);

			tmp = gtk_label_new("Scale");
			gtk_box_append(GTK_BOX(lyt), tmp);

			tmp = gtk_label_new("X");
			gtk_box_append(GTK_BOX(lyt), tmp);

			snprintf(cbuff, sizeof(cbuff), "%.02f", xform->scale.x);
			buff = gtk_entry_buffer_new(cbuff, (int)strnlen(cbuff, sizeof(cbuff)));
			tmp = gtk_entry_new_with_buffer(buff);
			g_object_unref(buff);
			gtk_box_append(GTK_BOX(lyt), tmp);

			tmp = gtk_label_new("Y");
			gtk_box_append(GTK_BOX(lyt), tmp);

			snprintf(cbuff, sizeof(cbuff), "%.02f", xform->scale.y);
			buff = gtk_entry_buffer_new(cbuff, (int)strnlen(cbuff, sizeof(cbuff)));
			tmp = gtk_entry_new_with_buffer(buff);
			g_object_unref(buff);
			gtk_box_append(GTK_BOX(lyt), tmp);

			tmp = gtk_label_new("Z");
			gtk_box_append(GTK_BOX(lyt), tmp);

			snprintf(cbuff, sizeof(cbuff), "%.02f", xform->scale.z);
			buff = gtk_entry_buffer_new(cbuff, (int)strnlen(cbuff, sizeof(cbuff)));
			tmp = gtk_entry_new_with_buffer(buff);
			g_object_unref(buff);
			gtk_box_append(GTK_BOX(lyt), tmp);

			gtk_box_append(rootLyt, lyt);
		}

		g_idle_add(G_SOURCE_FUNC(_Update), ptr);
	}
}

// List GObject

G_DEFINE_TYPE(NeInsEntityComp, ne_ins_entity_comp, G_TYPE_OBJECT)

static void
ne_ins_entity_comp_class_init(NeInsEntityCompClass *class)
{
}

static void
ne_ins_entity_comp_init(NeInsEntityComp *comp)
{
}
