#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <unistd.h>

#include <System/Window.h>
#include <System/Memory.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>

#include "UNIXPlatform.h"

static Window f_wnd;

bool
Sys_CreateWindow(void)
{
	int x = 10, y = 10;
	Window root;
	XSetWindowAttributes swa;
	XEvent xev;
	XWMHints wmHints;
	XClassHint classHint;
	XSizeHints sizeHints;
	pid_t pid;
	long compositor = 1;

	root = DefaultRootWindow(X11_display);
	swa.event_mask = KeyPressMask | KeyReleaseMask | PointerMotionMask |
					 ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
					 ExposureMask | StructureNotifyMask | FocusChangeMask;
	swa.override_redirect = False;
	swa.background_pixel = 0;

	f_wnd = XCreateWindow(X11_display, root, x, y, *E_screenWidth, *E_screenHeight, 0,
						X11_visualInfo.depth, InputOutput, X11_visualInfo.visual,
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

	XSetWMProperties(X11_display, f_wnd, NULL, NULL, NULL, 0, &sizeHints, &wmHints, &classHint);

	if (X11_NET_WM_PID) {
		pid = getpid();
		XChangeProperty(X11_display, f_wnd,
						X11_NET_WM_PID, XA_CARDINAL, 32, PropModeReplace,
						(const unsigned char *)&pid, 1);
	}

	if (X11_NET_WM_WINDOW_TYPE && X11_NET_WM_WINDOW_TYPE_NORMAL)
		XChangeProperty(X11_display, f_wnd,
						X11_NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace,
						(const unsigned char *)&X11_NET_WM_WINDOW_TYPE_NORMAL, 1);

	if (E_GetCVarBln("X11_BypassCompositor", true)->bln && X11_NET_WM_BYPASS_COMPOSITOR)
		XChangeProperty(X11_display, f_wnd,
						X11_NET_WM_BYPASS_COMPOSITOR, XA_CARDINAL, 32,
						PropModeReplace, (const unsigned char *)&compositor, 1);

	XSetWMProtocols(X11_display, f_wnd, &X11_WM_DELETE_WINDOW, 1);

	XMapWindow(X11_display, f_wnd);
	XStoreName(X11_display, f_wnd, "Olivia");

	memset(&xev, 0x0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = f_wnd;
	xev.xclient.message_type = X11_NET_WM_STATE;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = 1;
	xev.xclient.data.l[1] = False;

	XSendEvent(X11_display, root, False, SubstructureNotifyMask, &xev);

	E_screen = (void *)f_wnd;

	return true;
}

void
Sys_SetEngineWindow(void *wnd)
{

}

void
Sys_SetWindowTitle(const char *title)
{
	XStoreName(X11_display, (Window)E_screen, title);
	XSync(X11_display, False);
}

void
Sys_MoveWindow(int x, int y)
{
	XMoveWindow(X11_display, (Window)E_screen, x, y);
}

void
Sys_ShowWindow(bool show)
{
	//ShowWindow((HWND)E_screen, show ? SW_SHOW : SW_HIDE);
}

void
Sys_WorkArea(int *top, int *left, int *right, int *bottom)
{
	Atom type, *workArea = NULL;
	int format;
	unsigned long count, bytes;

	XGetWindowProperty(X11_display, XRootWindow(X11_display, 0), X11_NET_WORKAREA, 0, LONG_MAX,
						False, XA_CARDINAL, &type, &format, &count, &bytes, (unsigned char **)&workArea);

	if (top) *top = workArea[1];
	if (left) *left = workArea[0];
	if (right) *right = workArea[2];
	if (bottom) *bottom = workArea[3];
}

void
Sys_NonClientMetrics(int32_t *titleBarHeight, int32_t *borderHeight, int32_t *borderWidth)
{
	if (titleBarHeight) *titleBarHeight = 0;
	if (borderHeight) *borderHeight = 0;
	if (borderWidth) *borderWidth = 0;
}

void
Sys_DestroyWindow(void)
{
	XDestroyWindow(X11_display, (Window)E_screen);
}

/* NekoEngine
 *
 * Window.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
