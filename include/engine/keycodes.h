/* NekoEngine
 *
 * keycodes.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Keycodes
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

#ifndef _NE_ENGINE_KEYCODES_H_
#define _NE_ENGINE_KEYCODES_H_

typedef enum ne_key
{
	NE_KEY_0			= 0x00,
	NE_KEY_1			= 0x01,
	NE_KEY_2			= 0x02,
	NE_KEY_3			= 0x03,
	NE_KEY_4			= 0x04,
	NE_KEY_5			= 0x05,
	NE_KEY_6			= 0x06,
	NE_KEY_7			= 0x07,
	NE_KEY_8			= 0x08,
	NE_KEY_9			= 0x09,
	NE_KEY_A			= 0x0A,
	NE_KEY_B			= 0x0B,
	NE_KEY_C			= 0x0C,
	NE_KEY_D			= 0x0D,
	NE_KEY_E			= 0x0E,
	NE_KEY_F			= 0x0F,
	NE_KEY_G			= 0x10,
	NE_KEY_H			= 0x11,
	NE_KEY_I			= 0x12,
	NE_KEY_J			= 0x13,
	NE_KEY_K			= 0x14,
	NE_KEY_L			= 0x15,
	NE_KEY_M			= 0x16,
	NE_KEY_N			= 0x17,
	NE_KEY_O			= 0x18,
	NE_KEY_P			= 0x19,
	NE_KEY_Q			= 0x1A,
	NE_KEY_R			= 0x1B,
	NE_KEY_S			= 0x1C,
	NE_KEY_T			= 0x1D,
	NE_KEY_U			= 0x1E,
	NE_KEY_V			= 0x1F,
	NE_KEY_W			= 0x20,
	NE_KEY_X			= 0x21,
	NE_KEY_Y			= 0x22,
	NE_KEY_Z			= 0x23,
	NE_KEY_UP			= 0x24,
	NE_KEY_DOWN			= 0x25,
	NE_KEY_LEFT			= 0x26,
	NE_KEY_RIGHT			= 0x27,
	NE_KEY_SPACE			= 0x28,
	NE_KEY_TILDE			= 0x29,
	NE_KEY_TAB			= 0x2A,
	NE_KEY_CAPS			= 0x2B,
	NE_KEY_RETURN			= 0x2C,
	NE_KEY_LSHIFT			= 0x2D,
	NE_KEY_RSHIFT			= 0x2E,
	NE_KEY_LALT			= 0x2F,
	NE_KEY_RALT			= 0x30,
	NE_KEY_LSUPER			= 0x31,
	NE_KEY_RSUPER			= 0x32,
	NE_KEY_LCTRL			= 0x33,
	NE_KEY_RCTRL			= 0x34,
	NE_KEY_SLASH			= 0x35,
	NE_KEY_BKSLASH			= 0x36,
	NE_KEY_COMMA			= 0x37,
	NE_KEY_PERIOD			= 0x38,
	NE_KEY_SEMICOLON		= 0x39,
	NE_KEY_QUOTE			= 0x3A,
	NE_KEY_DELETE			= 0x3B,
	NE_KEY_PLUS			= 0x3C,
	NE_KEY_MINUS			= 0x3D,
	NE_KEY_BKSPACE			= 0x3E,
	NE_KEY_LBRACKET			= 0x3F,
	NE_KEY_RBRACKET			= 0x40,
	NE_KEY_ESCAPE			= 0x41,
	NE_KEY_PGUP			= 0x42,
	NE_KEY_PGDN			= 0x43,
	NE_KEY_HOME			= 0x44,
	NE_KEY_END			= 0x45,
	NE_KEY_INSERT			= 0x46,
	NE_KEY_SCROLL			= 0x47,
	NE_KEY_PRTSCRN			= 0x48,
	NE_KEY_PAUSE			= 0x49,
	NE_KEY_F1			= 0x4A,
	NE_KEY_F2			= 0x4B,
	NE_KEY_F3			= 0x4C,
	NE_KEY_F4			= 0x4D,
	NE_KEY_F5			= 0x4E,
	NE_KEY_F6			= 0x4F,
	NE_KEY_F7			= 0x50,
	NE_KEY_F8			= 0x51,
	NE_KEY_F9			= 0x52,
	NE_KEY_F10			= 0x53,
	NE_KEY_F11			= 0x54,
	NE_KEY_F12			= 0x55,
	NE_KEY_F13			= 0x56,
	NE_KEY_F14			= 0x57,
	NE_KEY_F15			= 0x58,
	NE_KEY_F16			= 0x59,
	NE_KEY_F17			= 0x5A,
	NE_KEY_F18			= 0x5B,
	NE_KEY_F19			= 0x5C,
	NE_KEY_F20			= 0x5D,
	NE_KEY_F21			= 0x5E,
	NE_KEY_F22			= 0x5F,
	NE_KEY_F23			= 0x60,
	NE_KEY_F24			= 0x61,
	NE_KEY_NUMLOCK			= 0x62,
	NE_KEY_NUM_0			= 0x63,
	NE_KEY_NUM_1			= 0x64,
	NE_KEY_NUM_2			= 0x65,
	NE_KEY_NUM_3			= 0x66,
	NE_KEY_NUM_4			= 0x67,
	NE_KEY_NUM_5			= 0x68,
	NE_KEY_NUM_6			= 0x69,
	NE_KEY_NUM_7			= 0x6A,
	NE_KEY_NUM_8			= 0x6B,
	NE_KEY_NUM_9			= 0x6C,
	NE_KEY_NUM_PLUS			= 0x6D,
	NE_KEY_NUM_MINUS		= 0x6E,
	NE_KEY_NUM_DECIMAL		= 0x6F,
	NE_KEY_NUM_DIVIDE		= 0x70,
	NE_KEY_NUM_MULT			= 0x71,
	NE_KEY_NUM_RETURN		= 0x72,
	NE_KEY_CLEAR			= 0x73,

	// Gamepad buttons	
	NE_GPAD_A			= 0x72,
	NE_GPAD_B			= 0x73,
	NE_GPAD_X			= 0x74,
	NE_GPAD_Y			= 0x75,
	NE_GPAD_LSHOULDER		= 0x76,
	NE_GPAD_RSHOULDER		= 0x77,
	NE_GPAD_LTHUMB			= 0x78,
	NE_GPAD_RTHUMB			= 0x79,
	NE_GPAD_BACK			= 0x7A,
	NE_GPAD_START			= 0x7B,
	NE_GPAD_D_RIGHT			= 0x7C,
	NE_GPAD_D_LEFT			= 0x7D,
	NE_GPAD_D_UP			= 0x7E,
	NE_GPAD_D_DOWN			= 0x7F,

	// Mouse buttons		
	NE_MOUSE_LMB			= 0x80,
	NE_MOUSE_RMB			= 0x81,
	NE_MOUSE_MMB			= 0x82,
	NE_MOUSE_BTN4			= 0x83,
	NE_MOUSE_BTN5			= 0x84,

	NE_KEY_UNRECOGNIZED		= 0xFF
} ne_key;

#endif /* _NE_ENGINE_KEYCODES_H_ */

