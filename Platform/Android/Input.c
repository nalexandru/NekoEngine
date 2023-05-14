#include "AndroidPlatform.h"

#include <Input/Input.h>
#include <Engine/Engine.h>

#include <System/Log.h>
#include <System/Memory.h>

#define ANDINMOD	"AndroidInput"

enum NeButton Android_keymap[256];

bool In_p_rawMouseAxis = false;

static int f_opcode, f_event, f_error;

static inline enum NeButton MapKey(const int key);

bool
In_SysInit(void)
{
//	for (uint16_t i = 0; i < 256; ++i)
//		X11_keymap[i] = MapKey(i);

	return true;
}

void
In_SysTerm(void)
{
}

void
In_SysPollControllers(void)
{
}

void
In_PointerPosition(uint16_t *x, uint16_t *y)
{

}

void
In_SetPointerPosition(uint16_t x, uint16_t y)
{
}

void
In_CapturePointer(bool capture)
{
	/*if (capture) {
		XSetInputFocus(X11_display, (Window)E_screen, RevertToPointerRoot, CurrentTime);
		XGrabPointer(X11_display, (Window)E_screen, False, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
					 GrabModeAsync, GrabModeAsync, (Window)E_screen, None, CurrentTime);
	} else {
		XUngrabPointer(X11_display, CurrentTime);
	}

	In_pointerCaptured = capture;*/
}

void
In_ShowPointer(bool show)
{
	/*Cursor cur;
	Pixmap pix;
	XColor black = { 0 };
	static char cursor_data[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	if (show) {
		cur = XCreateFontCursor(X11_display, XC_left_ptr);
		XDefineCursor(X11_display, (Window)E_screen, cur);
		XFreeCursor(X11_display, cur);
	} else {
		pix = XCreateBitmapFromData(X11_display, (Window)E_screen, cursor_data, 8, 8);
		cur = XCreatePixmapCursor(X11_display, pix, pix, &black, &black, 0, 0);

		XDefineCursor(X11_display, (Window)E_screen, cur);
		XFreeCursor(X11_display, cur);
		XFreePixmap(X11_display, pix);
	}

	XFlush(X11_display);

	In_pointerVisible = show;*/
}

/*bool
HandleInput(XEvent *ev)
{
	static Time prevTime = { 0 };

	if (ev->xcookie.type != GenericEvent || ev->xcookie.extension != f_opcode || !XGetEventData(X11_display, &ev->xcookie))
		return false;

	XIDeviceEvent *de = ev->xcookie.data;

	if (In_pointerCaptured && ev->xcookie.evtype == XI_RawMotion) {
		XIRawEvent *re = ev->xcookie.data;
		if (re->time == prevTime)
			return true;

		uint32_t j = 0;
		double coords[2] = { 0 };
		for (uint32_t i = 0; i < re->valuators.mask_len * 8 && j < 2; ++i) {
			if (XIMaskIsSet(re->valuators.mask, i)) {
				coords[j] = *re->raw_values;
				re->raw_values++;
			}
			++j;
		}

		In_mouseAxis[0] = ((float)coords[0] / (float)(*E_screenWidth / 2)) * -10.f;
		In_mouseAxis[1] = ((float)coords[1] / (float)(*E_screenHeight / 2)) * -10.f;

		prevTime = re->time;
	} else if (ev->xcookie.evtype == XI_ButtonPress) {
		switch (de->detail) {
			case Button1: In_buttonState[BTN_MOUSE_LMB] = true;
			case Button2: In_buttonState[BTN_MOUSE_RMB] = true;
			case Button3: In_buttonState[BTN_MOUSE_MMB] = true;
			case Button4: In_buttonState[BTN_MOUSE_BTN4] = true;
			case Button5: In_buttonState[BTN_MOUSE_BTN5] = true;
		}
	} else if (ev->xcookie.evtype == XI_ButtonRelease) {
		switch (de->detail) {
			case Button1: In_buttonState[BTN_MOUSE_LMB] = false;
			case Button2: In_buttonState[BTN_MOUSE_RMB] = false;
			case Button3: In_buttonState[BTN_MOUSE_MMB] = false;
			case Button4: In_buttonState[BTN_MOUSE_BTN4] = false;
			case Button5: In_buttonState[BTN_MOUSE_BTN5] = false;
		}
	} else if (ev->xcookie.evtype == XI_KeyPress) {
		In_Key(X11_keymap[de->detail], true);
	} else if (ev->xcookie.evtype == XI_KeyRelease) {
		In_Key(X11_keymap[de->detail], false);
	}

	XFreeEventData(X11_display, &ev->xcookie);

	return true;
}*/

/*enum NeButton
MapKey(int key)
{
	if (key < 8 || key > 254)
		return BTN_KEY_UNRECOGNIZED;

	int ksPerKc;
	KeySym *ks = XGetKeyboardMapping(X11_display, key, 1, (int *)&ksPerKc);
	key = ks[0];
	XFree(ks);

	switch (key) {
	case XK_0: return BTN_KEY_0;
	case XK_1: return BTN_KEY_1;
	case XK_2: return BTN_KEY_2;
	case XK_3: return BTN_KEY_3;
	case XK_4: return BTN_KEY_4;
	case XK_5: return BTN_KEY_5;
	case XK_6: return BTN_KEY_6;
	case XK_7: return BTN_KEY_7;
	case XK_8: return BTN_KEY_8;
	case XK_9: return BTN_KEY_9;

	case XK_a: return BTN_KEY_A;
	case XK_b: return BTN_KEY_B;
	case XK_c: return BTN_KEY_C;
	case XK_d: return BTN_KEY_D;
	case XK_e: return BTN_KEY_E;
	case XK_f: return BTN_KEY_F;
	case XK_g: return BTN_KEY_G;
	case XK_h: return BTN_KEY_H;
	case XK_i: return BTN_KEY_I;
	case XK_j: return BTN_KEY_J;
	case XK_k: return BTN_KEY_K;
	case XK_l: return BTN_KEY_L;
	case XK_m: return BTN_KEY_M;
	case XK_n: return BTN_KEY_N;
	case XK_o: return BTN_KEY_O;
	case XK_p: return BTN_KEY_P;
	case XK_q: return BTN_KEY_Q;
	case XK_r: return BTN_KEY_R;
	case XK_s: return BTN_KEY_S;
	case XK_t: return BTN_KEY_T;
	case XK_u: return BTN_KEY_U;
	case XK_v: return BTN_KEY_V;
	case XK_w: return BTN_KEY_W;
	case XK_x: return BTN_KEY_X;
	case XK_y: return BTN_KEY_Y;
	case XK_z: return BTN_KEY_Z;

	case XK_Up: return BTN_KEY_UP;
	case XK_Down: return BTN_KEY_DOWN;
	case XK_Left: return BTN_KEY_LEFT;
	case XK_Right: return BTN_KEY_RIGHT;
	case XK_space: return BTN_KEY_SPACE;
	case XK_plus: return BTN_KEY_EQUAL;
	case XK_minus: return BTN_KEY_MINUS;
	case XK_comma: return BTN_KEY_COMMA;
	case XK_period: return BTN_KEY_PERIOD;
	case XK_Scroll_Lock: return BTN_KEY_SCROLL;
	case XK_Shift_L: return BTN_KEY_LSHIFT;
	case XK_Shift_R: return BTN_KEY_RSHIFT;
	case XK_Alt_L: return BTN_KEY_LALT;
	case XK_Alt_R: return BTN_KEY_RALT;
	case XK_Super_L: return BTN_KEY_LSUPER;
	case XK_Super_R: return BTN_KEY_RSUPER;
	case XK_Control_L: return BTN_KEY_LCTRL;
	case XK_Control_R: return BTN_KEY_RCTRL;
	case XK_Page_Up: return BTN_KEY_PGUP;
	case XK_Page_Down: return BTN_KEY_PGDN;
	case XK_End: return BTN_KEY_END;
	case XK_Home: return BTN_KEY_HOME;
	case XK_Escape: return BTN_KEY_ESCAPE;
	case XK_Insert: return BTN_KEY_INSERT;
	case XK_Return: return BTN_KEY_RETURN;
	case XK_Caps_Lock: return BTN_KEY_CAPS;
	case XK_Delete: return BTN_KEY_DELETE;
	case XK_BackSpace: return BTN_KEY_BKSPACE;
	case XK_Tab: return BTN_KEY_TAB;
	case XK_Print: return BTN_KEY_PRTSCRN;
	case XK_Pause: return BTN_KEY_PAUSE;
	case XK_semicolon: return BTN_KEY_SEMICOLON;
	case XK_slash: return BTN_KEY_SLASH;
	case XK_quoteleft:
	case XK_asciitilde: return BTN_KEY_TILDE;
	case XK_bracketleft: return BTN_KEY_LBRACKET;
	case XK_bracketright: return BTN_KEY_RBRACKET;
	case XK_backslash: return BTN_KEY_BKSLASH;
	case XK_quotedbl: return BTN_KEY_QUOTE;

	case XK_Num_Lock: return BTN_KEY_NUMLOCK;
	case XK_KP_0: return BTN_KEY_NUM_0;
	case XK_KP_1: return BTN_KEY_NUM_1;
	case XK_KP_2: return BTN_KEY_NUM_2;
	case XK_KP_3: return BTN_KEY_NUM_3;
	case XK_KP_4: return BTN_KEY_NUM_4;
	case XK_KP_5: return BTN_KEY_NUM_5;
	case XK_KP_6: return BTN_KEY_NUM_6;
	case XK_KP_7: return BTN_KEY_NUM_7;
	case XK_KP_8: return BTN_KEY_NUM_8;
	case XK_KP_9: return BTN_KEY_NUM_9;
	case XK_KP_Add: return BTN_KEY_NUM_PLUS;
	case XK_KP_Subtract: return BTN_KEY_NUM_MINUS;
	case XK_KP_Divide: return BTN_KEY_NUM_DIVIDE;
	case XK_KP_Multiply: return BTN_KEY_NUM_MULT;
	case XK_KP_Decimal: return BTN_KEY_NUM_DECIMAL;

	case XK_F1: return BTN_KEY_F1;
	case XK_F2: return BTN_KEY_F2;
	case XK_F3: return BTN_KEY_F3;
	case XK_F4: return BTN_KEY_F4;
	case XK_F5: return BTN_KEY_F5;
	case XK_F6: return BTN_KEY_F6;
	case XK_F7: return BTN_KEY_F7;
	case XK_F8: return BTN_KEY_F8;
	case XK_F9: return BTN_KEY_F9;
	case XK_F10: return BTN_KEY_F10;
	case XK_F11: return BTN_KEY_F11;
	case XK_F12: return BTN_KEY_F12;
	case XK_F13: return BTN_KEY_F13;
	case XK_F14: return BTN_KEY_F14;
	case XK_F15: return BTN_KEY_F15;
	case XK_F16: return BTN_KEY_F16;
	case XK_F17: return BTN_KEY_F17;
	case XK_F18: return BTN_KEY_F18;
	case XK_F19: return BTN_KEY_F19;
	case XK_F20: return BTN_KEY_F20;
	case XK_F21: return BTN_KEY_F21;
	case XK_F22: return BTN_KEY_F22;
	case XK_F23: return BTN_KEY_F23;
	case XK_F24: return BTN_KEY_F24;
	}

	return BTN_KEY_UNRECOGNIZED;
}*/

/* NekoEngine
 *
 * Input.c
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
