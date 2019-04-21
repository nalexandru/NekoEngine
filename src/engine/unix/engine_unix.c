/* NekoEngine
 *
 * engine_unix.c
 * Author: Alexandru Naiman
 *
 * NekoEngine UNIX (X11) Main Loop
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

#define _DEFAULT_SOURCE

#include <X11/X.h>
#include <X11/Xlib.h>

#include <stdint.h>
#include <stdbool.h>

#include <system/log.h>

#include <engine/input.h>
#include <engine/engine.h>
#include <engine/window.h>

extern Display *x_display;
extern Window x11_active_window;
extern uint16_t x11_wnd_height, x11_wnd_width;
extern ne_key x11_to_ne_keycodes[256];
extern bool engine_stop;
extern float engine_frame_delta;

static Atom _wm_proto;
static Atom _delete_msg;

static inline bool
_process_event(void)
{
	XEvent xev, nxev;

	XNextEvent(x_display, &xev);

	switch (xev.type) {
	case KeyPress: {
		input_key_event(x11_to_ne_keycodes[xev.xkey.keycode], 1);
	} break;
	case KeyRelease: {
		if (XEventsQueued(x_display, QueuedAfterReading)) {
			XPeekEvent(x_display, &nxev);

			if (nxev.type == KeyPress &&
				nxev.xkey.time == xev.xkey.time &&
				nxev.xkey.keycode == xev.xkey.keycode)
				break;
		}

		input_key_event(x11_to_ne_keycodes[xev.xkey.keycode], 0);
	} break;
	case ButtonPress:
	case ButtonRelease: {
		switch (xev.xbutton.button) {
		case Button1:
			input_key_event(NE_MOUSE_LMB, xev.type == ButtonPress);
		break;
		case Button2:
			input_key_event(NE_MOUSE_RMB, xev.type == ButtonPress);
		break;
		case Button3:
			input_key_event(NE_MOUSE_MMB, xev.type == ButtonPress);
		break;
		case Button4:
			input_key_event(NE_MOUSE_BTN4, xev.type == ButtonPress);
		break;
		case Button5:
			input_key_event(NE_MOUSE_BTN5, xev.type == ButtonPress);
		break;
		}
	} break;
	case ConfigureNotify: {
		XConfigureEvent xce = xev.xconfigure;

		if (xce.width == x11_wnd_width && xce.height == x11_wnd_height)
			break;

		x11_wnd_width = xce.width;
		x11_wnd_height = xce.height;

		engine_screen_resized(x11_wnd_width, x11_wnd_height);
	} break;
	case ClientMessage: {
		if (xev.xclient.data.l[0] == _delete_msg)
			return true;
	} break;
	case DestroyNotify: {
		return true;
	} break;
	default : {
		// do nothing
	} break;
	}

	return false;
}

int
sys_engine_run(void)
{
	_wm_proto = XInternAtom(x_display, "WM_PROTOCOLS", False);
	_delete_msg = XInternAtom(x_display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(x_display, x11_active_window, &_delete_msg, 1);

	while (!engine_stop) {
		while (XPending(x_display)) {
			if (_process_event()) {
				engine_stop = true;
				break;
			}
		}

		engine_frame();

		XSync(x_display, False);
	}

	return 0;
}

