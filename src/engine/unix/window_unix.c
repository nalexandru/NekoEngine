/* NekoEngine
 *
 * window_unix.c
 * Author: Alexandru Naiman
 *
 * NekoEngine UNIX (X11) Window Subsystem
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <unistd.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/cursorfont.h>

#include <string.h>
#include <stdbool.h>

#include <system/log.h>

#include <engine/input.h>
#include <engine/window.h>
#include <engine/keycodes.h>

#define WINDOW_UNIX_MODULE		"Window_UNIX"

Display *x_display = 0;
Window x11_active_window;
uint16_t x11_wnd_width = 0, x11_wnd_height = 0;
static bool _fullscreen = false, _own_window = false;

ne_status
window_create(
	char *title,
	uint16_t w,
	uint16_t h)
{
	int pos_x = 0, pos_y = 0;
	Window root;
	XSetWindowAttributes swa, xattr;
	Atom wm_state,
		cardinal,
		_NET_WM_ICON,
		_NET_WM_PID,
		_NET_WM_WINDOW_TYPE,
		_NET_WM_WINDOW_TYPE_NORMAL,
		_NET_WM_BYPASS_COMPOSITOR;
	XEvent xev;
	XWMHints wm_hints;
	XClassHint class_hint;
	XSizeHints size_hints;
	Screen *screen;
	pid_t pid;
	long compositor = 1;

	x_display = XOpenDisplay(NULL);
	if (!x_display)
		return NE_NO_DISPLAY;

	screen = XDefaultScreenOfDisplay(x_display);
	root = DefaultRootWindow(x_display);

	swa.event_mask = ExposureMask | PointerMotionMask | KeyPressMask |
		KeyReleaseMask | StructureNotifyMask | PropertyChangeMask |
		FocusChangeMask | ButtonPressMask | ButtonReleaseMask |
		ButtonMotionMask | EnterWindowMask | LeaveWindowMask |
		PointerMotionMask;

	x11_active_window = XCreateWindow(x_display, root, pos_x, pos_y, w, h,
			0, CopyFromParent, InputOutput, CopyFromParent,
			CWEventMask, &swa);

	xattr.override_redirect = False;
	XChangeWindowAttributes(x_display, x11_active_window,
		CWOverrideRedirect, &xattr);

	/*hints.input = True;
	hints.flags = InputHint;
	XSetWMHints(x_display, x11_active_window, &hints);*/

	XMapWindow(x_display, x11_active_window);
	XStoreName(x_display, x11_active_window, title);

	wm_state = XInternAtom(x_display, "_NET_WM_STATE", False);

	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = x11_active_window;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = 1;
	xev.xclient.data.l[1] = False;

	XSendEvent(x_display, DefaultRootWindow(x_display), False,
				SubstructureNotifyMask, &xev);

	_NET_WM_ICON = XInternAtom(x_display, "_NET_WM_ICON", False);
	cardinal = XInternAtom(x_display, "CARDINAL", False);

	x11_wnd_width = w;
	x11_wnd_height = h;
	_own_window = true;

	// Set WM hints

	memset(&class_hint, 0x0, sizeof(class_hint));
	memset(&size_hints, 0x0, sizeof(size_hints));
	memset(&wm_hints, 0x0, sizeof(wm_hints));

	class_hint.res_name = "NekoEngine";
	class_hint.res_class = "NekoEngine";

	size_hints.x = pos_x;
	size_hints.y = pos_y;
	size_hints.flags |= USPosition;

	wm_hints.input = True;
	wm_hints.flags = InputHint;

	XSetWMProperties(x_display, x11_active_window, NULL, NULL, NULL, 0,
		&size_hints, &wm_hints, &class_hint);

	_NET_WM_PID = XInternAtom(x_display, "_NET_WM_PID", False);
	if (_NET_WM_PID) {
		pid = getpid();
		XChangeProperty(x_display, x11_active_window,
			_NET_WM_PID, XA_CARDINAL, 32, PropModeReplace,
			(const unsigned char *)&pid, 1);
	}

	_NET_WM_WINDOW_TYPE =
		XInternAtom(x_display, "_NET_WM_WINDOW_TYPE", False);
	_NET_WM_WINDOW_TYPE_NORMAL =
		XInternAtom(x_display, "_NET_WM_WINDOW_TYPE_NORMAL", False);

	if (_NET_WM_WINDOW_TYPE && _NET_WM_WINDOW_TYPE_NORMAL)
		XChangeProperty(x_display, x11_active_window,
			_NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace,
			(const unsigned char *)&_NET_WM_WINDOW_TYPE_NORMAL, 1);

	// bypass compositor
	_NET_WM_BYPASS_COMPOSITOR =
		XInternAtom(x_display, "_NET_WM_BYPASS_COMPOSITOR", False);
	if (_NET_WM_BYPASS_COMPOSITOR)
		XChangeProperty(x_display, x11_active_window,
			_NET_WM_BYPASS_COMPOSITOR, XA_CARDINAL, 32,
			PropModeReplace, (const unsigned char *)&compositor, 1);

	XSync(x_display, False);

	return NE_OK;
}

ne_status
window_register(void *handle)
{
	x11_active_window = (Window)handle;

	return NE_OK;
}

ne_status
window_resize(uint16_t w, uint16_t h)
{
	return NE_FAIL;
}

ne_status
window_set_title(char *title)
{
	return XStoreName(x_display, x11_active_window, title);
}

ne_status
window_fullscreen(uint16_t w, uint16_t h)
{
	return NE_FAIL;
}

void
window_destroy(void)
{
	if (_own_window)
		XDestroyWindow(x_display, x11_active_window);
	XCloseDisplay(x_display);
}
