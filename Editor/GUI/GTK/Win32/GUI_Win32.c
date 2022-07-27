#include "../GTKGUI.h"

#include <gdk/win32/gdkwin32.h>

void
GUI_InitPlatform(void)
{
	SetEnvironmentVariable(L"GTK_CSD", L"0");
}

void
GUI_MoveWindow(GtkWindow *wnd, int x, int y)
{
	GdkSurface *s = gtk_native_get_surface(GTK_NATIVE(wnd));
	HWND hwnd = gdk_win32_surface_get_impl_hwnd(s);
	MoveWindow(hwnd, x, y, 0, 0, TRUE);
}
