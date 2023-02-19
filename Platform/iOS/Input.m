#include <math.h>

#include <System/Log.h>
#include <Input/Input.h>
#include <Engine/Engine.h>

#import <UIKit/UIKit.h>
#import <GameController/GameController.h>

#define IOSINMOD	L"iOSInput"

enum NeButton iOS_keymap[256];
bool __InSys_rawMouseAxis = false;

static inline enum NeButton _mapKey(const int key);

bool
In_SysInit(void)
{
	// TODO: iPadOS pointer support
	// [GCController startWirelessControllerDiscoveryWithCompletionHandler]

	#pragma unroll(256)
	for (uint16_t i = 0; i < 256; ++i)
		iOS_keymap[i] = _mapKey(i);
	
	In_connectedControllers = [GCController controllers].count;
	
	return true;
}

void
In_SysTerm(void)
{
}

void
In_SysPollControllers(void)
{
	NSArray<GCController *> *controllers = [GCController controllers];
	In_connectedControllers = controllers.count;
	
	for (NSUInteger i = 0; i < controllers.count; ++i) {
		GCController *ctl = [controllers objectAtIndex: i];
		GCExtendedGamepad *gpad = [ctl extendedGamepad];
		
		In_controllerState[i].buttons = 0;
		
		GCControllerDirectionPad *dpad = [gpad leftThumbstick];
		In_controllerState[i].axis[AXIS_LSTICK_X] = [[dpad xAxis] value];
		In_controllerState[i].axis[AXIS_LSTICK_Y] = [[dpad yAxis] value];
		if ([[dpad down] isPressed])
			In_controllerState[i].buttons |= 0x0040;

		dpad = [gpad rightThumbstick];
		In_controllerState[i].axis[AXIS_RSTICK_X] = [[dpad xAxis] value];
		In_controllerState[i].axis[AXIS_RSTICK_Y] = [[dpad yAxis] value];
		if ([[dpad down] isPressed])
			In_controllerState[i].buttons |= 0x0080;
		
		In_controllerState[i].axis[AXIS_LTRIGGER] = [[gpad leftTrigger] value];
		In_controllerState[i].axis[AXIS_RTRIGGER] = [[gpad rightTrigger] value];
		
		dpad = [gpad dpad];
		if ([[dpad up] isPressed])
			In_controllerState[i].buttons |= 0x0001;
		if ([[dpad down] isPressed])
			In_controllerState[i].buttons |= 0x0002;
		if ([[dpad left] isPressed])
			In_controllerState[i].buttons |= 0x0004;
		if ([[dpad right] isPressed])
			In_controllerState[i].buttons |= 0x0008;
		
		if ([[gpad buttonA] isPressed])
			In_controllerState[i].buttons |= 0x1000;
		if ([[gpad buttonB] isPressed])
			In_controllerState[i].buttons |= 0x2000;
		if ([[gpad buttonX] isPressed])
			In_controllerState[i].buttons |= 0x4000;
		if ([[gpad buttonY] isPressed])
			In_controllerState[i].buttons |= 0x8000;
		
		if ([[gpad leftShoulder] isPressed])
			In_controllerState[i].buttons |= 0x0100;
		if ([[gpad rightShoulder] isPressed])
			In_controllerState[i].buttons |= 0x0200;
		
		if ([[gpad buttonHome] isPressed])
			In_controllerState[i].buttons |= 0x0010;
		if ([[gpad buttonMenu] isPressed])
			In_controllerState[i].buttons |= 0x0020;
	}
}

void
In_PointerPosition(uint16_t *x, uint16_t *y)
{
	/*NSWindow *win = [[NSApplication sharedApplication] keyWindow];
	NSPoint pos = [win mouseLocationOutsideOfEventStream];
	
	*x = pos.x;
	*y = [[win contentView] frame].size.height - pos.y - 1;*/
}

void
In_SetPointerPosition(uint16_t x, uint16_t y)
{
	/*NSWindow *win = [[NSApplication sharedApplication] keyWindow];
	NSPoint pos = [win convertPointToScreen:NSMakePoint((float)x,
						(float)([[win contentView] frame].size.height - y - 1))];
	
	CGAssociateMouseAndMouseCursorPosition(false);
	CGWarpMouseCursorPosition(CGPointMake(pos.x,
		CGDisplayBounds(CGMainDisplayID()).size.height - pos.y));
	CGAssociateMouseAndMouseCursorPosition(true);*/
}

void
In_CapturePointer(bool capture)
{
	(void)capture;
}

void
In_ShowPointer(bool show)
{
	/*if (show)
		CGDisplayShowCursor(kCGNullDirectDisplay);
	else
		CGDisplayHideCursor(kCGNullDirectDisplay);*/
}

enum NeButton
_mapKey(int key)
{
	switch (key) {
	case UIKeyboardHIDUsageKeyboard0: return BTN_KEY_0;
	case UIKeyboardHIDUsageKeyboard1: return BTN_KEY_1;
	case UIKeyboardHIDUsageKeyboard2: return BTN_KEY_2;
	case UIKeyboardHIDUsageKeyboard3: return BTN_KEY_3;
	case UIKeyboardHIDUsageKeyboard4: return BTN_KEY_4;
	case UIKeyboardHIDUsageKeyboard5: return BTN_KEY_5;
	case UIKeyboardHIDUsageKeyboard6: return BTN_KEY_6;
	case UIKeyboardHIDUsageKeyboard7: return BTN_KEY_7;
	case UIKeyboardHIDUsageKeyboard8: return BTN_KEY_8;
	case UIKeyboardHIDUsageKeyboard9: return BTN_KEY_9;

	case UIKeyboardHIDUsageKeyboardA: return BTN_KEY_A;
	case UIKeyboardHIDUsageKeyboardB: return BTN_KEY_B;
	case UIKeyboardHIDUsageKeyboardC: return BTN_KEY_C;
	case UIKeyboardHIDUsageKeyboardD: return BTN_KEY_D;
	case UIKeyboardHIDUsageKeyboardE: return BTN_KEY_E;
	case UIKeyboardHIDUsageKeyboardF: return BTN_KEY_F;
	case UIKeyboardHIDUsageKeyboardG: return BTN_KEY_G;
	case UIKeyboardHIDUsageKeyboardH: return BTN_KEY_H;
	case UIKeyboardHIDUsageKeyboardI: return BTN_KEY_I;
	case UIKeyboardHIDUsageKeyboardJ: return BTN_KEY_J;
	case UIKeyboardHIDUsageKeyboardK: return BTN_KEY_K;
	case UIKeyboardHIDUsageKeyboardL: return BTN_KEY_L;
	case UIKeyboardHIDUsageKeyboardM: return BTN_KEY_M;
	case UIKeyboardHIDUsageKeyboardN: return BTN_KEY_N;
	case UIKeyboardHIDUsageKeyboardO: return BTN_KEY_O;
	case UIKeyboardHIDUsageKeyboardP: return BTN_KEY_P;
	case UIKeyboardHIDUsageKeyboardQ: return BTN_KEY_Q;
	case UIKeyboardHIDUsageKeyboardR: return BTN_KEY_R;
	case UIKeyboardHIDUsageKeyboardS: return BTN_KEY_S;
	case UIKeyboardHIDUsageKeyboardT: return BTN_KEY_T;
	case UIKeyboardHIDUsageKeyboardU: return BTN_KEY_U;
	case UIKeyboardHIDUsageKeyboardV: return BTN_KEY_V;
	case UIKeyboardHIDUsageKeyboardW: return BTN_KEY_W;
	case UIKeyboardHIDUsageKeyboardX: return BTN_KEY_X;
	case UIKeyboardHIDUsageKeyboardY: return BTN_KEY_Y;
	case UIKeyboardHIDUsageKeyboardZ: return BTN_KEY_Z;

	case UIKeyboardHIDUsageKeyboardUpArrow: return BTN_KEY_UP;
	case UIKeyboardHIDUsageKeyboardDownArrow: return BTN_KEY_DOWN;
	case UIKeyboardHIDUsageKeyboardLeftArrow: return BTN_KEY_LEFT;
	case UIKeyboardHIDUsageKeyboardRightArrow: return BTN_KEY_RIGHT;
	case UIKeyboardHIDUsageKeyboardSpacebar: return BTN_KEY_SPACE;
	case UIKeyboardHIDUsageKeyboardEqualSign: return BTN_KEY_EQUAL;
	case UIKeyboardHIDUsageKeyboardHyphen: return BTN_KEY_MINUS;
	case UIKeyboardHIDUsageKeyboardComma: return BTN_KEY_COMMA;
	case UIKeyboardHIDUsageKeyboardPeriod: return BTN_KEY_PERIOD;
	case UIKeyboardHIDUsageKeyboardScrollLock: return BTN_KEY_SCROLL;
	case UIKeyboardHIDUsageKeyboardLeftShift: return BTN_KEY_LSHIFT;
	case UIKeyboardHIDUsageKeyboardRightShift: return BTN_KEY_RSHIFT;
	case UIKeyboardHIDUsageKeyboardLeftAlt: return BTN_KEY_LALT;
	case UIKeyboardHIDUsageKeyboardRightAlt: return BTN_KEY_RALT;
	case UIKeyboardHIDUsageKeyboardLeftGUI: return BTN_KEY_LSUPER;
	case UIKeyboardHIDUsageKeyboardRightGUI: return BTN_KEY_RSUPER;
	case UIKeyboardHIDUsageKeyboardLeftControl: return BTN_KEY_LCTRL;
	case UIKeyboardHIDUsageKeyboardRightControl: return BTN_KEY_RCTRL;
	case UIKeyboardHIDUsageKeyboardPageUp: return BTN_KEY_PGUP;
	case UIKeyboardHIDUsageKeyboardPageDown: return BTN_KEY_PGDN;
	case UIKeyboardHIDUsageKeyboardEnd: return BTN_KEY_END;
	case UIKeyboardHIDUsageKeyboardHome: return BTN_KEY_HOME;
	case UIKeyboardHIDUsageKeyboardEscape: return BTN_KEY_ESCAPE;
	case UIKeyboardHIDUsageKeyboardHelp: return BTN_KEY_INSERT;
	case UIKeyboardHIDUsageKeyboardReturn: return BTN_KEY_RETURN;
	case UIKeyboardHIDUsageKeyboardCapsLock: return BTN_KEY_CAPS;
	case UIKeyboardHIDUsageKeyboardDeleteForward: return BTN_KEY_DELETE;
	case UIKeyboardHIDUsageKeyboardDeleteOrBackspace: return BTN_KEY_BKSPACE;
	case UIKeyboardHIDUsageKeyboardTab: return BTN_KEY_TAB;
	case UIKeyboardHIDUsageKeyboardPrintScreen: return BTN_KEY_PRTSCRN;
	case UIKeyboardHIDUsageKeyboardPause: return BTN_KEY_PAUSE;
	case UIKeyboardHIDUsageKeyboardSemicolon: return BTN_KEY_SEMICOLON;
	case UIKeyboardHIDUsageKeyboardSlash: return BTN_KEY_SLASH;
	case UIKeyboardHIDUsageKeyboardGraveAccentAndTilde: return BTN_KEY_TILDE;
	case UIKeyboardHIDUsageKeyboardOpenBracket: return BTN_KEY_LBRACKET;
	case UIKeyboardHIDUsageKeyboardCloseBracket: return BTN_KEY_RBRACKET;
	case UIKeyboardHIDUsageKeyboardBackslash: return BTN_KEY_BKSLASH;
	case UIKeyboardHIDUsageKeyboardQuote: return BTN_KEY_QUOTE;

	case UIKeyboardHIDUsageKeypadNumLock: return BTN_KEY_NUMLOCK;
	case UIKeyboardHIDUsageKeypad0: return BTN_KEY_NUM_0;
	case UIKeyboardHIDUsageKeypad1: return BTN_KEY_NUM_1;
	case UIKeyboardHIDUsageKeypad2: return BTN_KEY_NUM_2;
	case UIKeyboardHIDUsageKeypad3: return BTN_KEY_NUM_3;
	case UIKeyboardHIDUsageKeypad4: return BTN_KEY_NUM_4;
	case UIKeyboardHIDUsageKeypad5: return BTN_KEY_NUM_5;
	case UIKeyboardHIDUsageKeypad6: return BTN_KEY_NUM_6;
	case UIKeyboardHIDUsageKeypad7: return BTN_KEY_NUM_7;
	case UIKeyboardHIDUsageKeypad8: return BTN_KEY_NUM_8;
	case UIKeyboardHIDUsageKeypad9: return BTN_KEY_NUM_9;
	case UIKeyboardHIDUsageKeypadPlus: return BTN_KEY_NUM_PLUS;
	case UIKeyboardHIDUsageKeypadHyphen: return BTN_KEY_NUM_MINUS;
	case UIKeyboardHIDUsageKeypadSlash: return BTN_KEY_NUM_DIVIDE;
	case UIKeyboardHIDUsageKeypadAsterisk: return BTN_KEY_NUM_MULT;
	case UIKeyboardHIDUsageKeypadPeriod: return BTN_KEY_NUM_DECIMAL;
	
	case UIKeyboardHIDUsageKeyboardF1: return BTN_KEY_F1;
	case UIKeyboardHIDUsageKeyboardF2: return BTN_KEY_F2;
	case UIKeyboardHIDUsageKeyboardF3: return BTN_KEY_F3;
	case UIKeyboardHIDUsageKeyboardF4: return BTN_KEY_F4;
	case UIKeyboardHIDUsageKeyboardF5: return BTN_KEY_F5;
	case UIKeyboardHIDUsageKeyboardF6: return BTN_KEY_F6;
	case UIKeyboardHIDUsageKeyboardF7: return BTN_KEY_F7;
	case UIKeyboardHIDUsageKeyboardF8: return BTN_KEY_F8;
	case UIKeyboardHIDUsageKeyboardF9: return BTN_KEY_F9;
	case UIKeyboardHIDUsageKeyboardF10: return BTN_KEY_F10;
	case UIKeyboardHIDUsageKeyboardF11: return BTN_KEY_F11;
	case UIKeyboardHIDUsageKeyboardF12: return BTN_KEY_F12;
	case UIKeyboardHIDUsageKeyboardF13: return BTN_KEY_F13;
	case UIKeyboardHIDUsageKeyboardF14: return BTN_KEY_F14;
	case UIKeyboardHIDUsageKeyboardF15: return BTN_KEY_F15;
	case UIKeyboardHIDUsageKeyboardF16: return BTN_KEY_F16;
	case UIKeyboardHIDUsageKeyboardF17: return BTN_KEY_F17;
	case UIKeyboardHIDUsageKeyboardF18: return BTN_KEY_F18;
	case UIKeyboardHIDUsageKeyboardF19: return BTN_KEY_F19;
	case UIKeyboardHIDUsageKeyboardF20: return BTN_KEY_F20;
	case UIKeyboardHIDUsageKeyboardF21: return BTN_KEY_F21;
	case UIKeyboardHIDUsageKeyboardF22: return BTN_KEY_F22;
	case UIKeyboardHIDUsageKeyboardF23: return BTN_KEY_F23;
	case UIKeyboardHIDUsageKeyboardF24: return BTN_KEY_F24;
	}

	return BTN_KEY_UNRECOGNIZED;
}

/* NekoEngine
 *
 * Input.m
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
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
 * -----------------------------------------------------------------------------
 */
