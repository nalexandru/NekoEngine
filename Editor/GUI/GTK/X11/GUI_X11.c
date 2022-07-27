#include "../GTKGUI.h"

#include <gdk/x11/gdkx.h>

void
GUI_InitPlatform(void)
{
	//SetEnvironmentVariable(L"GTK_CSD", L"0");
}

void
GUI_MoveWindow(GtkWindow *wnd, int x, int y)
{
	GdkSurface *s = gtk_native_get_surface(GTK_NATIVE(wnd));
	Window w = gdk_x11_surface_get_xid(s);
    if (w) // this will be false on XWayland
	    XMoveWindow(gdk_x11_display_get_xdisplay(gdk_surface_get_display(s)), w, x, y);
}
