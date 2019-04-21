/* NekoEngine
 *
 * input_unix.c
 * Author: Alexandru Naiman
 *
 * NekoEngine UNIX (X11) Input Subsystem
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

#include <string.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/cursorfont.h>

#include <system/log.h>

#include <engine/input.h>

#define INPUT_UNIX_MODULE		"Input_UNIX"

extern uint8_t cursor_state;
extern Display *x_display;
extern Window x11_active_window;

ne_key x11_to_ne_keycodes[256];

static inline uint16_t
_x11key_to_nekey(int code)
{
	KeySym *ks = NULL;

	if (code < 8 || code > 255)
		return NE_KEY_UNRECOGNIZED;

	ks = XGetKeyboardMapping(x_display, code, 1, (int *)&ks);
	code = ks[0];
	XFree(ks);

	switch (code) {
	case XK_0: return NE_KEY_0;
	case XK_1: return NE_KEY_1;
	case XK_2: return NE_KEY_2;
	case XK_3: return NE_KEY_3;
	case XK_4: return NE_KEY_4;
	case XK_5: return NE_KEY_5;
	case XK_6: return NE_KEY_6;
	case XK_7: return NE_KEY_7;
	case XK_8: return NE_KEY_8;
	case XK_9: return NE_KEY_9;

	case XK_a: return NE_KEY_A;
	case XK_b: return NE_KEY_B;
	case XK_c: return NE_KEY_C;
	case XK_d: return NE_KEY_D;
	case XK_e: return NE_KEY_E;
	case XK_f: return NE_KEY_F;
	case XK_g: return NE_KEY_G;
	case XK_h: return NE_KEY_H;
	case XK_i: return NE_KEY_I;
	case XK_j: return NE_KEY_J;
	case XK_k: return NE_KEY_K;
	case XK_l: return NE_KEY_L;
	case XK_m: return NE_KEY_M;
	case XK_n: return NE_KEY_N;
	case XK_o: return NE_KEY_O;
	case XK_p: return NE_KEY_P;
	case XK_q: return NE_KEY_Q;
	case XK_r: return NE_KEY_R;
	case XK_s: return NE_KEY_S;
	case XK_t: return NE_KEY_T;
	case XK_u: return NE_KEY_U;
	case XK_v: return NE_KEY_V;
	case XK_w: return NE_KEY_W;
	case XK_x: return NE_KEY_X;
	case XK_y: return NE_KEY_Y;
	case XK_z: return NE_KEY_Z;

	case XK_Up: return NE_KEY_UP;
	case XK_Down: return NE_KEY_DOWN;
	case XK_Left: return NE_KEY_LEFT;
	case XK_Right: return NE_KEY_RIGHT;
	case XK_space: return NE_KEY_SPACE;
	case XK_plus: return NE_KEY_PLUS;
	case XK_minus: return NE_KEY_MINUS;
	case XK_comma: return NE_KEY_COMMA;
	case XK_period: return NE_KEY_PERIOD;
	case XK_Scroll_Lock: return NE_KEY_SCROLL;
	case XK_Shift_L: return NE_KEY_LSHIFT;
	case XK_Shift_R: return NE_KEY_RSHIFT;
	case XK_Alt_L: return NE_KEY_LALT;
	case XK_Alt_R: return NE_KEY_RALT;
	case XK_Super_L: return NE_KEY_LSUPER;
	case XK_Super_R: return NE_KEY_RSUPER;
	case XK_Control_L: return NE_KEY_LCTRL;
	case XK_Control_R: return NE_KEY_RCTRL;
	case XK_Page_Up: return NE_KEY_PGUP;
	case XK_Page_Down: return NE_KEY_PGDN;
	case XK_End: return NE_KEY_END;
	case XK_Home: return NE_KEY_HOME;
	case XK_Escape: return NE_KEY_ESCAPE;
	case XK_Insert: return NE_KEY_INSERT;
	case XK_Return: return NE_KEY_RETURN;
	case XK_Caps_Lock: return NE_KEY_CAPS;
	case XK_Delete: return NE_KEY_DELETE;
	case XK_BackSpace: return NE_KEY_BKSPACE;
	case XK_Tab: return NE_KEY_TAB;
	case XK_Print: return NE_KEY_PRTSCRN;
	case XK_Pause: return NE_KEY_PAUSE;
	case XK_semicolon: return NE_KEY_SEMICOLON;
	case XK_slash: return NE_KEY_SLASH;
	case XK_asciitilde: return NE_KEY_TILDE;
	case XK_bracketleft: return NE_KEY_LBRACKET;
	case XK_bracketright: return NE_KEY_RBRACKET;
	case XK_backslash: return NE_KEY_BKSLASH;
	case XK_quotedbl: return NE_KEY_QUOTE;

	case XK_F1: return NE_KEY_F1;
	case XK_F2: return NE_KEY_F2;
	case XK_F3: return NE_KEY_F3;
	case XK_F4: return NE_KEY_F4;
	case XK_F5: return NE_KEY_F5;
	case XK_F6: return NE_KEY_F6;
	case XK_F7: return NE_KEY_F7;
	case XK_F8: return NE_KEY_F8;
	case XK_F9: return NE_KEY_F9;
	case XK_F10: return NE_KEY_F10;
	case XK_F11: return NE_KEY_F11;
	case XK_F12: return NE_KEY_F12;
	case XK_F13: return NE_KEY_F13;
	case XK_F14: return NE_KEY_F14;
	case XK_F15: return NE_KEY_F15;
	case XK_F16: return NE_KEY_F16;
	case XK_F17: return NE_KEY_F17;
	case XK_F18: return NE_KEY_F18;
	case XK_F19: return NE_KEY_F19;
	case XK_F20: return NE_KEY_F20;
	case XK_F21: return NE_KEY_F21;
	case XK_F22: return NE_KEY_F22;
	case XK_F23: return NE_KEY_F23;
	case XK_F24: return NE_KEY_F24;

	case XK_Num_Lock: return NE_KEY_F10;
	case XK_KP_0: return NE_KEY_NUM_0;
	case XK_KP_1: return NE_KEY_NUM_1;
	case XK_KP_2: return NE_KEY_NUM_2;
	case XK_KP_3: return NE_KEY_NUM_3;
	case XK_KP_4: return NE_KEY_NUM_4;
	case XK_KP_5: return NE_KEY_NUM_5;
	case XK_KP_6: return NE_KEY_NUM_6;
	case XK_KP_7: return NE_KEY_NUM_7;
	case XK_KP_8: return NE_KEY_NUM_8;
	case XK_KP_9: return NE_KEY_NUM_9;
	case XK_KP_Add: return NE_KEY_NUM_PLUS;
	case XK_KP_Subtract: return NE_KEY_NUM_MINUS;
	case XK_KP_Divide: return NE_KEY_NUM_DIVIDE;
	case XK_KP_Multiply: return NE_KEY_NUM_MULT;
	case XK_KP_Decimal: return NE_KEY_NUM_DECIMAL;
	}

	return NE_KEY_UNRECOGNIZED;
}

ne_status
sys_input_init(void)
{
	memset(x11_to_ne_keycodes, 0x0, sizeof(x11_to_ne_keycodes));

	for (uint16_t i = 0; i < 256; ++i) {
		if (x11_to_ne_keycodes[i] == 0)
			x11_to_ne_keycodes[i] = _x11key_to_nekey(i);
	}

	return NE_OK;
}

void
sys_input_poll_controllers(void)
{
	//
}

void
sys_input_release(void)
{
	//
}

void
input_show_cursor(bool show)
{
	Cursor cur;
	Pixmap pix;
	XColor black;
	static char cursor_data[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	black.red = black.green = black.blue = 0;

	if (show) {
		cur = XCreateFontCursor(x_display, XC_left_ptr);
		XDefineCursor(x_display, x11_active_window, cur);
		XFreeCursor(x_display, cur);

		cursor_state |= (uint8_t)1 << CURSOR_VISIBLE;
	} else {
		pix = XCreateBitmapFromData(x_display, x11_active_window,
			cursor_data, 8, 8);
		cur = XCreatePixmapCursor(x_display, pix, pix, &black, &black,
			0, 0);

		XDefineCursor(x_display, x11_active_window, cur);
		XFreeCursor(x_display, cur);
		XFreePixmap(x_display, pix);

		cursor_state &= ~((uint8_t)1 << CURSOR_VISIBLE);
	}

	XFlush(x_display);
}

void
input_capture_cursor(bool capture)
{
	if (capture) {
		XSetInputFocus(x_display, x11_active_window, RevertToPointerRoot,
			CurrentTime);
		XGrabPointer(x_display, x11_active_window, False,
			ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
			GrabModeAsync, GrabModeAsync, x11_active_window,
			None, CurrentTime);
		cursor_state |= (uint8_t)1 << CURSOR_CAPTURED;
	} else {
		XUngrabPointer(x_display, CurrentTime);
		cursor_state &= ~((uint8_t)1 << CURSOR_CAPTURED);
	}
}

