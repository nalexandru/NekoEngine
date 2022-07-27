#include "GTKGUI.h"
#include "Inspector.h"
#include "AboutDialog.h"
#include "AssetManager.h"
#include "SceneHierarchy.h"

#include "../../resource.h"

#include <Math/Math.h>
#include <Editor/GUI.h>
#include <Editor/Editor.h>
#include <Editor/Project.h>
#include <Engine/Engine.h>
#include <System/Window.h>
#include <System/Thread.h>
#include <Runtime/Runtime.h>

GtkApplication *Ed_gtkApplication;

static GtkWidget *_wdWindow, *_wdLabel, *_wdProgress;

static void _FileNew(GSimpleAction *act, GVariant *param, gpointer *user);
static void _FileOpen(GSimpleAction *act, GVariant *param, gpointer *user);
static void _FileSave(GSimpleAction *act, GVariant *param, gpointer *user);
static void _FileClose(GSimpleAction *act, GVariant *param, gpointer *user);
static void _FileQuit(GSimpleAction *act, GVariant *param, gpointer *user);

static void _ProjectSettings(GSimpleAction *act, GVariant *param, gpointer *user);

static void _ToolsSceneHierarchy(GSimpleAction *act, GVariant *param, gpointer *user);
static void _ToolsInspector(GSimpleAction *act, GVariant *param, gpointer *user);
static void _ToolsPreferences(GSimpleAction *act, GVariant *param, gpointer *user);

static void _HelpNativeReference(GSimpleAction *act, GVariant *param, gpointer *user);
static void _HelpScriptingReference(GSimpleAction *act, GVariant *param, gpointer *user);
static void _HelpAbout(GSimpleAction *act, GVariant *param, gpointer *user);

static int _ShowProgressDialog(const char *text);
static int _UpdateProgressDialog(const char *text);
static int _HideProgressDialog(void *a);

static void
_activate(GtkApplication *app, gpointer user)
{
	int top, left, right, bottom;
	Sys_WorkArea(&top, &left, &right, &bottom);

	GtkSettings *settings = gtk_settings_get_default();
	g_object_set(G_OBJECT(settings), "gtk-application-prefer-dark-theme", true, NULL);

	GtkIconTheme *iconTheme = gtk_icon_theme_get_for_display(gdk_display_get_default());
	gtk_icon_theme_add_search_path(iconTheme, "D:\\gtkres\\hicolor\\apps");

	gtk_window_set_default_icon_name("neko-editor");

	GSimpleAction *act;
	GMenu *menuBar = g_menu_new();
	GMenu *menu = NULL, *section = NULL;

	{ // File Menu
		act = g_simple_action_new("file-new", NULL);
		g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act));
		g_signal_connect(act, "activate", G_CALLBACK(_FileNew), NULL);

		act = g_simple_action_new("file-open", NULL);
		g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act));
		g_signal_connect(act, "activate", G_CALLBACK(_FileOpen), NULL);

		act = g_simple_action_new("file-save", NULL);
		g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act));
		g_signal_connect(act, "activate", G_CALLBACK(_FileSave), NULL);

		act = g_simple_action_new("file-close", NULL);
		g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act));
		g_signal_connect(act, "activate", G_CALLBACK(_FileClose), NULL);

		act = g_simple_action_new("file-quit", NULL);
		g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act));
		g_signal_connect(act, "activate", G_CALLBACK(_FileQuit), NULL);

		menu = g_menu_new();
	
		section = g_menu_new();
		g_menu_append(section, "New", "app.file-new");
		g_menu_append_section(menu, NULL, G_MENU_MODEL(section));
	
		section = g_menu_new();
		g_menu_append(section, "Open", "app.file-open");
		g_menu_append(section, "Save", "app.file-save");
		g_menu_append(section, "Close", "app.file-close");
		g_menu_append_section(menu, NULL, G_MENU_MODEL(section));
	
		section = g_menu_new();
		g_menu_append(section, "Quit", "app.file-quit");
		g_menu_append_section(menu, NULL, G_MENU_MODEL(section));
		g_menu_append_submenu(menuBar, "File", G_MENU_MODEL(menu));
	}

	{ // Project Menu
		act = g_simple_action_new("project-settings", NULL);
		g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act));
		g_signal_connect(act, "activate", G_CALLBACK(_ProjectSettings), NULL);

		menu = g_menu_new();

		g_menu_append(menu, "Settings", "app.project-settings");
		g_menu_append_submenu(menuBar, "Project", G_MENU_MODEL(menu));
	}

	{ // Tools Menu
		act = g_simple_action_new("tools-sceneHierarchy", NULL);
		g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act));
		g_signal_connect(act, "activate", G_CALLBACK(_ToolsSceneHierarchy), NULL);

		act = g_simple_action_new("tools-inspector", NULL);
		g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act));
		g_signal_connect(act, "activate", G_CALLBACK(_ToolsInspector), NULL);

		act = g_simple_action_new("tools-preferences", NULL);
		g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act));
		g_signal_connect(act, "activate", G_CALLBACK(_ToolsPreferences), NULL);

		menu = g_menu_new();

		section = g_menu_new();
		g_menu_append(section, "Scene Hierarchy", "app.tools-sceneHierarchy");
		g_menu_append(section, "Inspector", "app.tools-inspector");
		g_menu_append_section(menu, NULL, G_MENU_MODEL(section));

		section = g_menu_new();
		g_menu_append(section, "Preferences", "app.tools-preferences");
		g_menu_append_section(menu, NULL, G_MENU_MODEL(section));
		g_menu_append_submenu(menuBar, "Tools", G_MENU_MODEL(menu));
	}

	{ // Help Menu
		act = g_simple_action_new("help-nativeReference", NULL);
		g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act));
		g_signal_connect(act, "activate", G_CALLBACK(_HelpNativeReference), NULL);

		act = g_simple_action_new("help-scriptingReference", NULL);
		g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act));
		g_signal_connect(act, "activate", G_CALLBACK(_HelpScriptingReference), NULL);

		act = g_simple_action_new("help-about", NULL);
		g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(act));
		g_signal_connect(act, "activate", G_CALLBACK(_HelpAbout), NULL);

		menu = g_menu_new();

		section = g_menu_new();
		g_menu_append(section, "Native API Reference", "app.help-nativeReference");
		g_menu_append(section, "Scripting API Reference", "app.help-scriptingReference");
		g_menu_append_section(menu, NULL, G_MENU_MODEL(section));
	
		section = g_menu_new();
		g_menu_append(section, "About", "app.help-about");
		g_menu_append_section(menu, NULL, G_MENU_MODEL(section));
		g_menu_append_submenu(menuBar, "Help", G_MENU_MODEL(menu));
	}

	gtk_application_set_menubar(Ed_gtkApplication, G_MENU_MODEL(menuBar));

	GUI_InitAssetManager(left, top + *E_screenHeight, 600, 290);
	GUI_InitSceneHierarchy(left, top, 250, *E_screenHeight);
	GUI_InitInspector(*E_screenWidth + left + 250, 0, 350, *E_screenHeight);

	Sys_MoveWindow(left + 250, top);
}

void
Ed_ShowProjectDialog(void)
{
	Ed_activeProject = (struct NeProject *)1;
}

static void
_uiThreadProc(void *p)
{
	g_application_run(G_APPLICATION(Ed_gtkApplication), 0, NULL);
	g_object_unref(Ed_gtkApplication);
}

bool
Ed_CreateGUI(void)
{
	GUI_InitPlatform();

	Ed_gtkApplication = gtk_application_new(ED_APPLICATION_ID, G_APPLICATION_FLAGS_NONE);
	g_signal_connect(Ed_gtkApplication, "activate", G_CALLBACK(_activate), NULL);

	NeThread uiThread;
	Sys_InitThread(&uiThread, "Editor UI Thread", _uiThreadProc, NULL);

	return true;
}

void
EdGUI_ProcessEvents(void)
{
}

void
EdGUI_MessageBox(const char *title, const char *message)
{
	GtkWidget *dlg = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", message);
	gtk_widget_show(dlg);
}

void
EdGUI_ShowProgressDialog(const char *text)
{
	g_idle_add(G_SOURCE_FUNC(_ShowProgressDialog), (gpointer)text);
}

void
EdGUI_UpdateProgressDialog(const char *text)
{
	g_idle_add(G_SOURCE_FUNC(_UpdateProgressDialog), (gpointer)text);
}

void
EdGUI_HideProgressDialog(void)
{
	g_idle_add(G_SOURCE_FUNC(_HideProgressDialog), NULL);
}

void
Ed_TermGUI(void)
{
	GUI_TermSceneHierarchy();
	GUI_TermAssetManager();
	GUI_TermInspector();

	g_object_unref(Ed_gtkApplication);
}

static void _FileNew(GSimpleAction *act, GVariant *param, gpointer *user) { }
static void _FileOpen(GSimpleAction *act, GVariant *param, gpointer *user) { }
static void _FileSave(GSimpleAction *act, GVariant *param, gpointer *user) { }
static void _FileClose(GSimpleAction *act, GVariant *param, gpointer *user) { }

static void
_FileQuit(GSimpleAction *act, GVariant *param, gpointer *user)
{
	E_Shutdown();
}

static void
_ProjectSettings(GSimpleAction *act, GVariant *param, gpointer *user)
{

}

static void
_ToolsSceneHierarchy(GSimpleAction *act, GVariant *param, gpointer *user)
{
	GUI_ShowSceneHierarchy();
}

static void
_ToolsInspector(GSimpleAction *act, GVariant *param, gpointer *user)
{
	GUI_ShowInspector();
}

static void
_ToolsPreferences(GSimpleAction *act, GVariant *param, gpointer *user)
{
	//
}

static void
_HelpNativeReference(GSimpleAction *act, GVariant *param, gpointer *user)
{
	//
}

static void
_HelpScriptingReference(GSimpleAction *act, GVariant *param, gpointer *user)
{
	//
}

static void
_HelpAbout(GSimpleAction *act, GVariant *param, gpointer *user)
{
	GUI_AboutDialog();
}

static int
_ShowProgressDialog(const char *text)
{
	_wdWindow = gtk_window_new();
		
	gtk_widget_set_size_request(_wdWindow, 400, 100);
	gtk_window_set_modal(GTK_WINDOW(_wdWindow), true);

	GtkWidget *lyt = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
	gtk_widget_set_margin_top(lyt, 5);
	gtk_widget_set_margin_end(lyt, 5);
	gtk_widget_set_margin_start(lyt, 5);
	gtk_widget_set_margin_bottom(lyt, 5);
	gtk_widget_set_valign(lyt, GTK_ALIGN_CENTER);
	gtk_window_set_child(GTK_WINDOW(_wdWindow), lyt);

	_wdLabel = gtk_label_new(text);

	PangoAttrList *attrs = pango_attr_list_new();
	PangoAttribute *attr = pango_attr_size_new(18 * PANGO_SCALE);
	pango_attr_list_insert(attrs, attr);
	gtk_label_set_attributes(GTK_LABEL(_wdLabel), attrs);
	gtk_box_append(GTK_BOX(lyt), _wdLabel);

	_wdProgress = gtk_progress_bar_new();
	gtk_progress_bar_pulse(GTK_PROGRESS_BAR(_wdProgress));
	gtk_box_append(GTK_BOX(lyt), _wdProgress);

	gtk_window_set_application(GTK_WINDOW(_wdWindow), Ed_gtkApplication);
	
	gtk_window_set_title(GTK_WINDOW(_wdWindow), "Working...");
	gtk_window_present(GTK_WINDOW(_wdWindow));

	pango_attr_list_unref(attrs);

	return 0;
}

static int
_UpdateProgressDialog(const char *text)
{
	gtk_label_set_label(GTK_LABEL(_wdLabel), text);
	gtk_progress_bar_pulse(GTK_PROGRESS_BAR(_wdProgress));
	return 0;
}

static int
_HideProgressDialog(void *a)
{
	gtk_window_close(GTK_WINDOW(_wdWindow));
	return 0;
}
