/* NekoEngine
 *
 * input_win32.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Win32 Input Subsystem
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

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <system/log.h>

#include <engine/input.h>

#define INPUT_WIN32_MODULE		"Input_Win32"

extern uint8_t cursor_state;
extern UINT NWM_SHOWCURSOR;
extern UINT NWM_HIDECURSOR;

ne_key win32_to_ne_keycodes[256];
UINT (*win32_GetRawInputData)(HRAWINPUT, UINT, LPVOID, PUINT, UINT) = NULL;
LRESULT (*win32_DefRawInputProc)(PRAWINPUT *, INT, UINT) = NULL;

static HANDLE _input_user32;

static inline uint16_t
_win32key_to_nekey(int code)
{
	switch (code) {
		case VK_BACK: return NE_KEY_BKSPACE;
		case VK_TAB: return NE_KEY_TAB;
		case VK_CLEAR: return NE_KEY_CLEAR;
		case VK_RETURN: return NE_KEY_RETURN;
		case VK_LSHIFT: return NE_KEY_LSHIFT;
		case VK_RSHIFT: return NE_KEY_RSHIFT;
		case VK_LCONTROL: return NE_KEY_LCTRL;
		case VK_RCONTROL: return NE_KEY_RCTRL;
		case VK_CAPITAL: return NE_KEY_CAPS;
		case VK_PAUSE: return NE_KEY_PAUSE;
		case VK_LMENU: return NE_KEY_LALT;
		case VK_RMENU: return NE_KEY_RALT;
		case VK_ESCAPE: return NE_KEY_ESCAPE;
		case VK_SPACE: return NE_KEY_SPACE;
		case VK_PRIOR: return NE_KEY_PGUP;
		case VK_NEXT: return NE_KEY_PGDN;
		case VK_END: return NE_KEY_END;
		case VK_HOME: return NE_KEY_HOME;
		case VK_LEFT: return NE_KEY_LEFT;
		case VK_UP: return NE_KEY_UP;
		case VK_RIGHT: return NE_KEY_RIGHT;
		case VK_DOWN: return NE_KEY_DOWN;
		case VK_SNAPSHOT: return NE_KEY_PRTSCRN;
		case VK_INSERT: return NE_KEY_INSERT;
		case VK_DELETE: return NE_KEY_DELETE;
		case VK_SCROLL: return NE_KEY_SCROLL;

		case 0x30: return NE_KEY_0;
		case 0x31: return NE_KEY_1;
		case 0x32: return NE_KEY_2;
		case 0x33: return NE_KEY_3;
		case 0x34: return NE_KEY_4;
		case 0x35: return NE_KEY_5;
		case 0x36: return NE_KEY_6;
		case 0x37: return NE_KEY_7;
		case 0x38: return NE_KEY_8;
		case 0x39: return NE_KEY_9;

		case 0x41: return NE_KEY_A;
		case 0x42: return NE_KEY_B;
		case 0x43: return NE_KEY_C;
		case 0x44: return NE_KEY_D;
		case 0x45: return NE_KEY_E;
		case 0x46: return NE_KEY_F;
		case 0x47: return NE_KEY_G;
		case 0x48: return NE_KEY_H;
		case 0x49: return NE_KEY_I;
		case 0x4A: return NE_KEY_J;
		case 0x4B: return NE_KEY_K;
		case 0x4C: return NE_KEY_L;
		case 0x4D: return NE_KEY_M;
		case 0x4E: return NE_KEY_N;
		case 0x4F: return NE_KEY_O;
		case 0x50: return NE_KEY_P;
		case 0x51: return NE_KEY_Q;
		case 0x52: return NE_KEY_R;
		case 0x53: return NE_KEY_S;
		case 0x54: return NE_KEY_T;
		case 0x55: return NE_KEY_U;
		case 0x56: return NE_KEY_V;
		case 0x57: return NE_KEY_W;
		case 0x58: return NE_KEY_X;
		case 0x59: return NE_KEY_Y;
		case 0x5A: return NE_KEY_Z;

		case VK_LWIN: return NE_KEY_LSUPER;
		case VK_RWIN: return NE_KEY_RSUPER;

		case VK_NUMLOCK: return NE_KEY_NUMLOCK;
		case VK_NUMPAD0: return NE_KEY_NUM_0;
		case VK_NUMPAD1: return NE_KEY_NUM_1;
		case VK_NUMPAD2: return NE_KEY_NUM_2;
		case VK_NUMPAD3: return NE_KEY_NUM_3;
		case VK_NUMPAD4: return NE_KEY_NUM_4;
		case VK_NUMPAD5: return NE_KEY_NUM_5;
		case VK_NUMPAD6: return NE_KEY_NUM_6;
		case VK_NUMPAD7: return NE_KEY_NUM_7;
		case VK_NUMPAD8: return NE_KEY_NUM_8;
		case VK_NUMPAD9: return NE_KEY_NUM_9;
		case VK_MULTIPLY: return NE_KEY_NUM_MULT;
		case VK_ADD: return NE_KEY_NUM_PLUS;
		case VK_SUBTRACT: return NE_KEY_MINUS;
		case VK_DECIMAL: return NE_KEY_NUM_DECIMAL;
		case VK_DIVIDE: return NE_KEY_NUM_DIVIDE;

		case VK_F1: return NE_KEY_F1;
		case VK_F2: return NE_KEY_F2;
		case VK_F3: return NE_KEY_F3;
		case VK_F4: return NE_KEY_F4;
		case VK_F5: return NE_KEY_F5;
		case VK_F6: return NE_KEY_F6;
		case VK_F7: return NE_KEY_F7;
		case VK_F8: return NE_KEY_F8;
		case VK_F9: return NE_KEY_F9;
		case VK_F10: return NE_KEY_F10;
		case VK_F11: return NE_KEY_F11;
		case VK_F12: return NE_KEY_F12;
		case VK_F13: return NE_KEY_F13;
		case VK_F14: return NE_KEY_F14;
		case VK_F15: return NE_KEY_F15;
		case VK_F16: return NE_KEY_F16;
		case VK_F17: return NE_KEY_F17;
		case VK_F18: return NE_KEY_F18;
		case VK_F19: return NE_KEY_F19;
		case VK_F20: return NE_KEY_F20;
		case VK_F21: return NE_KEY_F21;
		case VK_F22: return NE_KEY_F22;
		case VK_F23: return NE_KEY_F23;
		case VK_F24: return NE_KEY_F24;
	}

	return NE_KEY_UNRECOGNIZED;
}

ne_status
sys_input_init(void)
{
	RAWINPUTDEVICE rid[2];
	BOOL (WINAPI *_RegisterRawInputDevices)(PCRAWINPUTDEVICE, UINT, UINT);

	memset(win32_to_ne_keycodes, 0x0, sizeof(win32_to_ne_keycodes));

	for (uint16_t i = 0; i < 256; ++i) {
		if (win32_to_ne_keycodes[i] == 0)
			win32_to_ne_keycodes[i] = _win32key_to_nekey(i);
	}


	rid[0].usUsagePage = 0x01;
	rid[0].usUsage = 0x02;
	rid[0].dwFlags = 0; // set this only in fullscreen RIDEV_NOLEGACY;
	rid[0].hwndTarget = 0;

	rid[1].usUsagePage = 0x01;
	rid[1].usUsage = 0x06;
	rid[1].dwFlags = RIDEV_NOLEGACY;
	rid[1].hwndTarget = 0;

	_input_user32 = LoadLibraryA("user32.dll");
	if (!_input_user32)
		return NE_OK;

	_RegisterRawInputDevices = GetProcAddress(_input_user32,
			"RegisterRawInputDevices");
	win32_GetRawInputData = GetProcAddress(_input_user32,
			"GetRawInputData");
	win32_DefRawInputProc = GetProcAddress(_input_user32,
			"DefRawInputProc");

	if (!_RegisterRawInputDevices ||
		!win32_GetRawInputData ||
		!win32_DefRawInputProc) {
		log_entry(INPUT_WIN32_MODULE, LOG_DEBUG,
				"Raw input not available");
		return NE_OK;
	}

	if (!_RegisterRawInputDevices(rid, 2, sizeof(rid[0]))) {
		log_entry(INPUT_WIN32_MODULE, LOG_CRITICAL,
				"Failed to register raw input devices");
		return NE_FAIL;
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
	if (_input_user32)
		FreeLibrary(_input_user32);
}

void
input_show_cursor(bool show)
{
	if (show) {
		PostMessage(GetActiveWindow(), NWM_SHOWCURSOR, 0, 0);
		cursor_state |= (uint8_t)1 << CURSOR_VISIBLE;
	} else {
		PostMessage(GetActiveWindow(), NWM_HIDECURSOR, 0, 0);
		cursor_state &= ~((uint8_t)1 << CURSOR_VISIBLE);
	}
}

void
input_capture_cursor(bool capture)
{
	if (capture) {
		SetCapture(GetActiveWindow());
		cursor_state |= (uint8_t)1 << CURSOR_CAPTURED;
	} else {
		ReleaseCapture();
		cursor_state &= ~((uint8_t)1 << CURSOR_CAPTURED);
	}
}

