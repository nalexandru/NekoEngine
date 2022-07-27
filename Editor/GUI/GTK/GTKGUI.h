#ifndef _EDITOR_GTK_GUI_H_
#define _EDITOR_GTK_GUI_H_

#include <gtk/gtk.h>

#include <Engine/Types.h>

#define ED_APPLICATION_ID		"xyz.nalexandru.NekoEditor"

GtkApplication *Ed_gtkApplication;

// Platform specific
void GUI_InitPlatform(void);
void GUI_MoveWindow(GtkWindow *wnd, int x, int y);

#endif /* _EDITOR_GTK_GUI_H_ */
