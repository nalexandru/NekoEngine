#ifndef NE_INPUT_CODES_H
#define NE_INPUT_CODES_H

#ifdef __cplusplus
extern "C" {
#endif

enum NeButton 
{
	BTN_KEY_0 = 0x00,
	BTN_KEY_1 = 0x01,
	BTN_KEY_2 = 0x02,
	BTN_KEY_3 = 0x03,
	BTN_KEY_4 = 0x04,
	BTN_KEY_5 = 0x05,
	BTN_KEY_6 = 0x06,
	BTN_KEY_7 = 0x07,
	BTN_KEY_8 = 0x08,
	BTN_KEY_9 = 0x09,
	BTN_KEY_A = 0x0A,
	BTN_KEY_B = 0x0B,
	BTN_KEY_C = 0x0C,
	BTN_KEY_D = 0x0D,
	BTN_KEY_E = 0x0E,
	BTN_KEY_F = 0x0F,
	BTN_KEY_G = 0x10,
	BTN_KEY_H = 0x11,
	BTN_KEY_I = 0x12,
	BTN_KEY_J = 0x13,
	BTN_KEY_K = 0x14,
	BTN_KEY_L = 0x15,
	BTN_KEY_M = 0x16,
	BTN_KEY_N = 0x17,
	BTN_KEY_O = 0x18,
	BTN_KEY_P = 0x19,
	BTN_KEY_Q = 0x1A,
	BTN_KEY_R = 0x1B,
	BTN_KEY_S = 0x1C,
	BTN_KEY_T = 0x1D,
	BTN_KEY_U = 0x1E,
	BTN_KEY_V = 0x1F,
	BTN_KEY_W = 0x20,
	BTN_KEY_X = 0x21,
	BTN_KEY_Y = 0x22,
	BTN_KEY_Z = 0x23,
	BTN_KEY_UP = 0x24,
	BTN_KEY_DOWN = 0x25,
	BTN_KEY_LEFT = 0x26,
	BTN_KEY_RIGHT = 0x27,
	BTN_KEY_SPACE = 0x28,
	BTN_KEY_TILDE = 0x29,
	BTN_KEY_TAB = 0x2A,
	BTN_KEY_CAPS = 0x2B,
	BTN_KEY_RETURN = 0x2C,
	BTN_KEY_LSHIFT = 0x2D,
	BTN_KEY_RSHIFT = 0x2E,
	BTN_KEY_LALT = 0x2F,
	BTN_KEY_RALT = 0x30,
	BTN_KEY_LSUPER = 0x31,
	BTN_KEY_RSUPER = 0x32,
	BTN_KEY_LCTRL = 0x33,
	BTN_KEY_RCTRL = 0x34,
	BTN_KEY_SLASH = 0x35,
	BTN_KEY_BKSLASH = 0x36,
	BTN_KEY_COMMA = 0x37,
	BTN_KEY_PERIOD = 0x38,
	BTN_KEY_SEMICOLON = 0x39,
	BTN_KEY_QUOTE = 0x3A,
	BTN_KEY_DELETE = 0x3B,
	BTN_KEY_EQUAL = 0x3C,
	BTN_KEY_MINUS = 0x3D,
	BTN_KEY_BKSPACE = 0x3E,
	BTN_KEY_LBRACKET = 0x3F,
	BTN_KEY_RBRACKET = 0x40,
	BTN_KEY_ESCAPE = 0x41,
	BTN_KEY_PGUP = 0x42,
	BTN_KEY_PGDN = 0x43,
	BTN_KEY_HOME = 0x44,
	BTN_KEY_END = 0x45,
	BTN_KEY_INSERT = 0x46,
	BTN_KEY_SCROLL = 0x47,
	BTN_KEY_PRTSCRN = 0x48,
	BTN_KEY_PAUSE = 0x49,
	BTN_KEY_F1 = 0x4A,
	BTN_KEY_F2 = 0x4B,
	BTN_KEY_F3 = 0x4C,
	BTN_KEY_F4 = 0x4D,
	BTN_KEY_F5 = 0x4E,
	BTN_KEY_F6 = 0x4F,
	BTN_KEY_F7 = 0x50,
	BTN_KEY_F8 = 0x51,
	BTN_KEY_F9 = 0x52,
	BTN_KEY_F10 = 0x53,
	BTN_KEY_F11 = 0x54,
	BTN_KEY_F12 = 0x55,
	BTN_KEY_F13 = 0x56,
	BTN_KEY_F14 = 0x57,
	BTN_KEY_F15 = 0x58,
	BTN_KEY_F16 = 0x59,
	BTN_KEY_F17 = 0x5A,
	BTN_KEY_F18 = 0x5B,
	BTN_KEY_F19 = 0x5C,
	BTN_KEY_F20 = 0x5D,
	BTN_KEY_F21 = 0x5E,
	BTN_KEY_F22 = 0x5F,
	BTN_KEY_F23 = 0x60,
	BTN_KEY_F24 = 0x61,
	BTN_KEY_NUMLOCK = 0x62,
	BTN_KEY_NUM_0 = 0x63,
	BTN_KEY_NUM_1 = 0x64,
	BTN_KEY_NUM_2 = 0x65,
	BTN_KEY_NUM_3 = 0x66,
	BTN_KEY_NUM_4 = 0x67,
	BTN_KEY_NUM_5 = 0x68,
	BTN_KEY_NUM_6 = 0x69,
	BTN_KEY_NUM_7 = 0x6A,
	BTN_KEY_NUM_8 = 0x6B,
	BTN_KEY_NUM_9 = 0x6C,
	BTN_KEY_NUM_PLUS = 0x6D,
	BTN_KEY_NUM_MINUS = 0x6E,
	BTN_KEY_NUM_DECIMAL = 0x6F,
	BTN_KEY_NUM_DIVIDE = 0x70,
	BTN_KEY_NUM_MULT = 0x71,
	BTN_KEY_NUM_RETURN = 0x72,
	BTN_KEY_CLEAR = 0x73,

	// Mouse buttons
	BTN_MOUSE_LMB = 0x80,
	BTN_MOUSE_RMB = 0x81,
	BTN_MOUSE_MMB = 0x82,
	BTN_MOUSE_BTN4 = 0x83,
	BTN_MOUSE_BTN5 = 0x84,

	BTN_STATE_COUNT = 0x90,

	BTN_GPAD_BTN_D_UP = 0xA0,
	BTN_GPAD_BTN_D_DOWN = 0xA1,
	BTN_GPAD_BTN_D_LEFT = 0xA2,
	BTN_GPAD_BTN_D_RIGHT = 0xA3,
	BTN_GPAD_BTN_START = 0xA4,
	BTN_GPAD_BTN_BACK = 0xA5,
	BTN_GPAD_BTN_LTHUMB = 0xA6,
	BTN_GPAD_BTN_RTHUMB = 0xA7,
	BTN_GPAD_BTN_LBUMPER = 0xA8,
	BTN_GPAD_BTN_RBUMPER = 0xA9,
	BTN_GPAD_BTN_A = 0xAC,
	BTN_GPAD_BTN_B = 0xAD,
	BTN_GPAD_BTN_X = 0xAE,
	BTN_GPAD_BTN_Y = 0xAF,

	BTN_GPAD_BTN_BASE = BTN_GPAD_BTN_D_UP,

	BTN_KEY_UNRECOGNIZED = 0xFF,

	BTN_FORCE_UINT = 0xFFFFFFFF
};

enum NeAxis
{
	AXIS_LSTICK_X = 0,
	AXIS_LSTICK_Y = 1,
	AXIS_RSTICK_X = 2,
	AXIS_RSTICK_Y = 3,
	AXIS_LTRIGGER = 4,
	AXIS_RTRIGGER = 5,
	AXIS_GYRO_X = 6,
	AXIS_GYRO_Y = 7,
	AXIS_GYRO_Z = 8,

	AXIS_MOUSE_X = 10,
	AXIS_MOUSE_Y = 11,
	AXIS_MOUSE_WHEEL = 12,

	AXIS_VIRTUAL_0 = 20,
	AXIS_VIRTUAL_1 = 21,
	AXIS_VIRTUAL_2 = 22,
	AXIS_VIRTUAL_3 = 23,
	AXIS_VIRTUAL_4 = 24,
	AXIS_VIRTUAL_5 = 25,
	AXIS_VIRTUAL_6 = 26,
	AXIS_VIRTUAL_7 = 27,
	AXIS_VIRTUAL_8 = 28,
	AXIS_VIRTUAL_9 = 29,

	CONTROLLER_AXIS_COUNT = AXIS_GYRO_Z + 1,
	MOUSE_AXIS_START = 10,
	VIRTUAL_AXIS_START = 20,

	AXIS_UNRECOGNIZED = 0xFF
};

#ifdef __cplusplus
}
#endif

#endif /* NE_INPUT_CODES_H */

/* NekoEngine
 *
 * Codes.h
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
