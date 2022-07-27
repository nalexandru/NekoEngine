#include "GTKGUI.h"
#include "AboutDialog.h"
#include "AssetManager.h"

#include "Inspector.h"
#include "SceneHierarchy.h"

#include <Engine/IO.h>
#include <Engine/Engine.h>
#include <System/Log.h>
#include <Runtime/Runtime.h>
#include <Editor/Asset/Asset.h>
#include <Editor/Asset/Import.h>

static char _currentPath[4096] = { '/', 0x0 };
static const char **_fileList;
static GtkStringList *_stringList;

static GtkWidget *_wnd, *_path, *_grid;

static void _SetupListItem(GtkListItemFactory *factory, GtkListItem *listItem);
static void _BindListItem(GtkListItemFactory *factory, GtkListItem *listItem);

static void _ImportResponse(GtkDialog *dlg, int response);
static void _Import(GtkButton *btn, gpointer user);
static void _Close(GtkWindow *wnd, gpointer user);

static void _GridActivate(GtkGridView *view, guint position, gpointer user);

static void _UpdateList(void);

static void _GetFullPath(const char *file, char *path, size_t pathSize);

bool
GUI_InitAssetManager(int x, int y, int width, int height)
{
	_wnd = gtk_application_window_new(Ed_gtkApplication);
	if (!_wnd)
		return false;

	gtk_window_set_title(GTK_WINDOW(_wnd), "Asset Manager");
	gtk_window_set_default_size(GTK_WINDOW(_wnd), width, height);

	GtkWidget *topLyt = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_widget_set_margin_top(topLyt, 5);
	gtk_widget_set_margin_start(topLyt, 5);
	gtk_widget_set_margin_end(topLyt, 5);
	gtk_widget_set_margin_bottom(topLyt, 5);
	gtk_window_set_child(GTK_WINDOW(_wnd), topLyt);

	GtkWidget *lyt = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_box_append(GTK_BOX(topLyt), lyt);

	GtkWidget *tmp = gtk_button_new_with_label("Import");
	g_signal_connect(tmp, "clicked", G_CALLBACK(_Import), NULL);
	gtk_box_append(GTK_BOX(lyt), tmp);

	_path = gtk_entry_new();
	gtk_widget_set_hexpand(_path, true);
	gtk_box_append(GTK_BOX(lyt), _path);

	GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
	g_signal_connect(factory, "setup", G_CALLBACK(_SetupListItem), NULL);
	g_signal_connect(factory, "bind", G_CALLBACK(_BindListItem), NULL);

	_stringList = gtk_string_list_new(NULL);
	GtkSingleSelection *sm = gtk_single_selection_new(G_LIST_MODEL(_stringList));

	tmp = gtk_scrolled_window_new();
	gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(tmp), true);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(tmp), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	_grid = gtk_grid_view_new(GTK_SELECTION_MODEL(sm), GTK_LIST_ITEM_FACTORY(factory));
	gtk_widget_set_hexpand(_grid, true);
	gtk_widget_set_vexpand(_grid, true);

	g_signal_connect(_grid, "activate", G_CALLBACK(_GridActivate), NULL);

	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(tmp), _grid);
	gtk_box_append(GTK_BOX(topLyt), tmp);

	g_signal_connect(_wnd, "close-request", G_CALLBACK(_Close), Ed_gtkApplication);

	gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(_wnd), true);
	gtk_widget_show(_wnd);

	GUI_MoveWindow(GTK_WINDOW(_wnd), x, y);

	_UpdateList();

	g_object_unref(sm);

	return true;
}

void
GUI_ShowAssetManager(void)
{
	gtk_window_present(GTK_WINDOW(_wnd));
}

void
GUI_TermAssetManager(void)
{
	E_FreeFileList(_fileList);
}

static void
_SetupListItem(GtkListItemFactory *factory, GtkListItem *listItem)
{
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);

	GtkWidget *image = gtk_image_new();
	gtk_image_set_icon_size(GTK_IMAGE(image), GTK_ICON_SIZE_LARGE);
	gtk_box_append(GTK_BOX(box), image);

	GtkWidget *label = gtk_label_new("");
	gtk_box_append(GTK_BOX(box), label);

	gtk_list_item_set_child(listItem, box);
}

static void
_BindListItem(GtkListItemFactory *factory, GtkListItem *listItem)
{
	GtkWidget *box = gtk_list_item_get_child(listItem);
	GtkWidget *label = gtk_widget_get_last_child(box);
	GtkWidget *image = gtk_widget_get_first_child(box);
	GtkStringObject *item = gtk_list_item_get_item(listItem);

	const char *file = gtk_string_object_get_string(item);
	gtk_label_set_label(GTK_LABEL(label), file);

	const char *icon = "nekoed-unknown";
	char path[4096] = { 0x0 }, buff[256] = { 0x0 };
	_GetFullPath(file, path, sizeof(path));

	if (E_IsDirectory(path)) {
		icon = "nekoed-directory";
	} else {
		char *ext = strrchr(file, '.');
		if (ext) {
			++ext;

			snprintf(buff, sizeof(buff), "nekoed-%s", ext);

			GtkIconTheme *it = gtk_icon_theme_get_for_display(gdk_display_get_default());
			if (gtk_icon_theme_has_icon(it, buff))
				icon = buff;
		}
	}

	gtk_image_set_from_icon_name(GTK_IMAGE(image), icon);
}

static void
_ImportResponse(GtkDialog *dlg, int response)
{
	if (response != GTK_RESPONSE_ACCEPT)
		return;

	GFile *f = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dlg));
	char *file = g_file_get_path(f);

	Asset_Import(file);

	g_free(file);
	g_object_unref(dlg);
}

static void
_Import(GtkButton *btn, gpointer user)
{
	GtkFileChooserNative *dlg = gtk_file_chooser_native_new("Import Asset", GTK_WINDOW(_wnd), GTK_FILE_CHOOSER_ACTION_OPEN, "OK", "Cancel");
	gtk_native_dialog_show(GTK_NATIVE_DIALOG(dlg));
	g_signal_connect(dlg, "response", G_CALLBACK(_ImportResponse), NULL);
}

static void
_Close(GtkWindow *wnd, gpointer user)
{
	E_Shutdown();
}

static void
_GridActivate(GtkGridView *grid, guint position, gpointer user)
{
	char path[4096] = { 0x0 };
	const char *item = gtk_string_list_get_string(_stringList, position);

	_GetFullPath(item, path, sizeof(path));

	if (E_IsDirectory(path)) {
		memcpy(_currentPath, path, sizeof(_currentPath));
		_UpdateList();
	} else {
		Ed_OpenAsset(path);
	}
}

static void
_UpdateList(void)
{
	const char **files = E_ListFiles(_currentPath);
	if (strlen(_currentPath) > 1) {
		const char *prev[] = { "..", NULL };
		gtk_string_list_splice(_stringList, 0, g_list_model_get_n_items(G_LIST_MODEL(_stringList)), prev);
		gtk_string_list_splice(_stringList, 1, 0, files);
	} else {
		gtk_string_list_splice(_stringList, 0, g_list_model_get_n_items(G_LIST_MODEL(_stringList)), files);
	}
	E_FreeFileList(files);
}

static void
_GetFullPath(const char *file, char *path, size_t pathSize)
{
	if (!strncmp(file, "..", strnlen(file, sizeof(file)))) {
		char *p = strrchr(_currentPath, '/');
		size_t pos = p - _currentPath;

		if (pos) {
			_currentPath[pos] = 0x0;
			memcpy(path, _currentPath, pos);
		} else {
			path[0] = '/';
		}
	} else {
		snprintf(path, pathSize, strnlen(_currentPath, sizeof(_currentPath)) > 1 ? "%s/%s" : "%s%s", _currentPath, file);
	}
}
