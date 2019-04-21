/* NekoEngine
 *
 * input_mac.m
 * Author: Alexandru Naiman
 *
 * NekoEngine Mac (Cocoa) Input Subsystem
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

#import <Carbon/Carbon.h>

#include <system/log.h>

#include <engine/input.h>

#define INPUT_UNIX_MODULE		"Input_UNIX"

extern uint8_t cursor_state;

ne_key cocoa_to_ne_keycodes[256];

static inline uint16_t
_cocoakey_to_nekey(int code)
{
	if (code < 0 || code > 255)
		return NE_KEY_UNRECOGNIZED;

	switch (code) {
	case kVK_ANSI_0: return NE_KEY_0;
	case kVK_ANSI_1: return NE_KEY_1;
	case kVK_ANSI_2: return NE_KEY_2;
	case kVK_ANSI_3: return NE_KEY_3;
	case kVK_ANSI_4: return NE_KEY_4;
	case kVK_ANSI_5: return NE_KEY_5;
	case kVK_ANSI_6: return NE_KEY_6;
	case kVK_ANSI_7: return NE_KEY_7;
	case kVK_ANSI_8: return NE_KEY_8;
	case kVK_ANSI_9: return NE_KEY_9;

	case kVK_ANSI_A: return NE_KEY_A;
	case kVK_ANSI_B: return NE_KEY_B;
	case kVK_ANSI_C: return NE_KEY_C;
	case kVK_ANSI_D: return NE_KEY_D;
	case kVK_ANSI_E: return NE_KEY_E;
	case kVK_ANSI_F: return NE_KEY_F;
	case kVK_ANSI_G: return NE_KEY_G;
	case kVK_ANSI_H: return NE_KEY_H;
	case kVK_ANSI_I: return NE_KEY_I;
	case kVK_ANSI_J: return NE_KEY_J;
	case kVK_ANSI_K: return NE_KEY_K;
	case kVK_ANSI_L: return NE_KEY_L;
	case kVK_ANSI_M: return NE_KEY_M;
	case kVK_ANSI_N: return NE_KEY_N;
	case kVK_ANSI_O: return NE_KEY_O;
	case kVK_ANSI_P: return NE_KEY_P;
	case kVK_ANSI_Q: return NE_KEY_Q;
	case kVK_ANSI_R: return NE_KEY_R;
	case kVK_ANSI_S: return NE_KEY_S;
	case kVK_ANSI_T: return NE_KEY_T;
	case kVK_ANSI_U: return NE_KEY_U;
	case kVK_ANSI_V: return NE_KEY_V;
	case kVK_ANSI_W: return NE_KEY_W;
	case kVK_ANSI_X: return NE_KEY_X;
	case kVK_ANSI_Y: return NE_KEY_Y;
	case kVK_ANSI_Z: return NE_KEY_Z;

	case kVK_UpArrow: return NE_KEY_UP;
	case kVK_DownArrow: return NE_KEY_DOWN;
	case kVK_LeftArrow: return NE_KEY_LEFT;
	case kVK_RightArrow: return NE_KEY_RIGHT;
	case kVK_Space: return NE_KEY_SPACE;
	case kVK_ANSI_Equal: return NE_KEY_PLUS;
	case kVK_ANSI_Minus: return NE_KEY_MINUS;
	case kVK_ANSI_Comma: return NE_KEY_COMMA;
	case kVK_ANSI_Period: return NE_KEY_PERIOD;
	//case XK_Scroll_Lock: return NE_KEY_SCROLL;
	case kVK_Shift: return NE_KEY_LSHIFT;
	case kVK_RightShift: return NE_KEY_RSHIFT;
	case kVK_Option: return NE_KEY_LALT;
	case kVK_RightOption: return NE_KEY_RALT;
	case kVK_Command: return NE_KEY_LSUPER;
	case 0x36: return NE_KEY_RSUPER;
	case kVK_Control: return NE_KEY_LCTRL;
	case kVK_RightControl: return NE_KEY_RCTRL;
	case kVK_PageUp: return NE_KEY_PGUP;
	case kVK_PageDown: return NE_KEY_PGDN;
	case kVK_End: return NE_KEY_END;
	case kVK_Home: return NE_KEY_HOME;
	case kVK_Escape: return NE_KEY_ESCAPE;
	case kVK_Help: return NE_KEY_INSERT;
	case kVK_Return: return NE_KEY_RETURN;
	case kVK_CapsLock: return NE_KEY_CAPS;
	case kVK_ForwardDelete: return NE_KEY_DELETE;
	case kVK_Delete: return NE_KEY_BKSPACE;
	case kVK_Tab: return NE_KEY_TAB;
	//case XK_Print: return NE_KEY_PRTSCRN;
	//case XK_Pause: return NE_KEY_PAUSE;
	case kVK_ANSI_Semicolon: return NE_KEY_SEMICOLON;
	case kVK_ANSI_Slash: return NE_KEY_SLASH;
	case kVK_ANSI_Grave: return NE_KEY_TILDE;
	case kVK_ANSI_LeftBracket: return NE_KEY_LBRACKET;
	case kVK_ANSI_RightBracket: return NE_KEY_RBRACKET;
	case kVK_ANSI_Backslash: return NE_KEY_BKSLASH;
	case kVK_ANSI_Quote: return NE_KEY_QUOTE;

	case kVK_F1: return NE_KEY_F1;
	case kVK_F2: return NE_KEY_F2;
	case kVK_F3: return NE_KEY_F3;
	case kVK_F4: return NE_KEY_F4;
	case kVK_F5: return NE_KEY_F5;
	case kVK_F6: return NE_KEY_F6;
	case kVK_F7: return NE_KEY_F7;
	case kVK_F8: return NE_KEY_F8;
	case kVK_F9: return NE_KEY_F9;
	case kVK_F10: return NE_KEY_F10;
	case kVK_F11: return NE_KEY_F11;
	case kVK_F12: return NE_KEY_F12;
	case kVK_F13: return NE_KEY_F13;
	case kVK_F14: return NE_KEY_F14;
	case kVK_F15: return NE_KEY_F15;
	case kVK_F16: return NE_KEY_F16;
	case kVK_F17: return NE_KEY_F17;
	case kVK_F18: return NE_KEY_F18;
	case kVK_F19: return NE_KEY_F19;
	case kVK_F20: return NE_KEY_F20;
	/*case kVK_F21: return NE_KEY_F21;
	case kVK_F22: return NE_KEY_F22;
	case kVK_F23: return NE_KEY_F23;
	case kVK_F24: return NE_KEY_F24;*/

	case 0x47: return NE_KEY_NUMLOCK;
	case kVK_ANSI_Keypad0: return NE_KEY_NUM_0;
	case kVK_ANSI_Keypad1: return NE_KEY_NUM_1;
	case kVK_ANSI_Keypad2: return NE_KEY_NUM_2;
	case kVK_ANSI_Keypad3: return NE_KEY_NUM_3;
	case kVK_ANSI_Keypad4: return NE_KEY_NUM_4;
	case kVK_ANSI_Keypad5: return NE_KEY_NUM_5;
	case kVK_ANSI_Keypad6: return NE_KEY_NUM_6;
	case kVK_ANSI_Keypad7: return NE_KEY_NUM_7;
	case kVK_ANSI_Keypad8: return NE_KEY_NUM_8;
	case kVK_ANSI_Keypad9: return NE_KEY_NUM_9;
	case kVK_ANSI_KeypadPlus: return NE_KEY_NUM_PLUS;
	case kVK_ANSI_KeypadMinus: return NE_KEY_NUM_MINUS;
	case kVK_ANSI_KeypadDivide: return NE_KEY_NUM_DIVIDE;
	case kVK_ANSI_KeypadMultiply: return NE_KEY_NUM_MULT;
	case kVK_ANSI_KeypadDecimal: return NE_KEY_NUM_DECIMAL;
	}

	return NE_KEY_UNRECOGNIZED;
}

// Taken from:
// http://stackoverflow.com/questions/1918841/how-to-convert-ascii-character-to-cgkeycode/1971027#1971027
CFStringRef
create_string_for_key(CGKeyCode keyCode)
{
	/*TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();
	CFDataRef layoutData = (CFDataRef)TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);
	const UCKeyboardLayout *keyboardLayout = (const UCKeyboardLayout *)CFDataGetBytePtr(layoutData);
	
	UInt32 keysDown = 0;
	UniChar chars[4];
	UniCharCount realLength;
	
	UCKeyTranslate(keyboardLayout,
				   keyCode,
				   kUCKeyActionDisplay,
				   0,
				   LMGetKbdType(),
				   kUCKeyTranslateNoDeadKeysBit,
				   &keysDown,
				   sizeof(chars) / sizeof(chars[0]),
				   &realLength,
				   chars);
	CFRelease(currentKeyboard);
	
	return CFStringCreateWithCharacters(kCFAllocatorDefault, chars, 1);*/
	return NULL;
}

ne_status
sys_input_init(void)
{
	memset(cocoa_to_ne_keycodes, 0x0, sizeof(cocoa_to_ne_keycodes));

	for (uint16_t i = 0; i < 256; ++i) {
		if (cocoa_to_ne_keycodes[i] == 0)
			cocoa_to_ne_keycodes[i] = _cocoakey_to_nekey(i);
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

}

void
input_capture_cursor(bool capture)
{

}
