#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <System/Window.h>
#include <System/Memory.h>
#include <Engine/Engine.h>
#include <Render/Render.h>

#include "UNIXPlatform.h"

int
Sys_CreateWindow(void)
{
	int x, y;
	Window root, win;
	XSetWindowAttributes swa;
	XEvent xev;
	XWMHints wmHints;
	XClassHint classHint;
	XSizeHints sizeHints;
	pid_t pid;
	long compositor = 1;
	
	root = DefaultRootWindow(X11_Display);
	swa.event_mask = KeyPressMask | KeyReleaseMask |
			ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
			ExposureMask | StructureNotifyMask | FocusChangeMask;
	swa.override_redirect = False;
	swa.background_pixel = 0;
 
	win = XCreateWindow(X11_Display, root, x, y, *E_ScreenWidth, *E_ScreenHeight, 0,
			X11_VisualInfo.depth, InputOutput, X11_VisualInfo.visual,
			CWBackPixel | CWEventMask | CWOverrideRedirect, &swa);
 
	// Set WM hints

	memset(&classHint, 0x0, sizeof(classHint));
	memset(&sizeHints, 0x0, sizeof(sizeHints));
	memset(&wmHints, 0x0, sizeof(wmHints));

	classHint.res_name = "NekoEngine";
	classHint.res_class = "NekoEngine";

	sizeHints.x = x;
	sizeHints.y = y;
	sizeHints.flags |= USPosition;

	wmHints.input = True;
	wmHints.flags = InputHint;

	XSetWMProperties(X11_Display, win, NULL, NULL, NULL, 0, &sizeHints, &wmHints, &classHint);

	if (X11_NET_WM_PID) {
		pid = getpid();
		XChangeProperty(X11_Display, win,
				X11_NET_WM_PID, XA_CARDINAL, 32, PropModeReplace,
				(const unsigned char *)&pid, 1);
	}

	if (X11_NET_WM_WINDOW_TYPE && X11_NET_WM_WINDOW_TYPE_NORMAL)
		XChangeProperty(X11_Display, win,
				X11_NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace,
				(const unsigned char *)&X11_NET_WM_WINDOW_TYPE_NORMAL, 1);

	if (X11_NET_WM_BYPASS_COMPOSITOR)
		XChangeProperty(X11_Display, win,
				X11_NET_WM_BYPASS_COMPOSITOR, XA_CARDINAL, 32,
				PropModeReplace, (const unsigned char *)&compositor, 1);

	XSetWMProtocols(X11_Display, win, &X11_WM_DELETE_WINDOW, 1);

	XMapWindow(X11_Display, win);
	XStoreName(X11_Display, win, "ProjectClaire");

	memset(&xev, 0x0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = win;
	xev.xclient.message_type = X11_NET_WM_STATE;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = 1;
	xev.xclient.data.l[1] = False;

	XSendEvent(X11_Display, root, False, SubstructureNotifyMask, &xev);

	E_Screen = (void *)win;

	return 0;
}

void
Sys_SetWindowTitle(const wchar_t *name)
{
	size_t len = wcslen(name);
	char *title = Sys_Alloc(sizeof(char), len + 1, MH_Transient);
	wcstombs(title, name, len);
	XStoreName(X11_Display, (Window)E_Screen, title);
}

void
Sys_DestroyWindow(void)
{
	XDestroyWindow(X11_Display, (Window)E_Screen);
}
