#include <math.h>

#define Handle __EngineHandle

#include "macOSPlatform.h"

#include <System/Log.h>
#include <Input/Input.h>
#include <Engine/Engine.h>

#undef Handle

#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>
#import <GameController/GameController.h>

#define MACXINMOD	L"macOSInput"

enum Button macOS_keymap[256];

static inline enum Button _mapKey(const int key);

bool
In_SysInit(void)
{
	//[GCController startWirelessControllerDiscoveryWithCompletionHandler]
	
	#pragma unroll(256)
	for (uint16_t i = 0; i < 256; ++i)
		macOS_keymap[i] = _mapKey(i);

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
	NSWindow *win = [[NSApplication sharedApplication] keyWindow];
	NSPoint pos = [win mouseLocationOutsideOfEventStream];
	
	*x = pos.x;
	*y = [[win contentView] frame].size.height - pos.y - 1;
}

void
In_SetPointerPosition(uint16_t x, uint16_t y)
{
	NSWindow *win = [[NSApplication sharedApplication] keyWindow];
	NSPoint pos = [win convertPointToScreen:NSMakePoint((float)x,
						(float)([[win contentView] frame].size.height - y - 1))];
	
	CGAssociateMouseAndMouseCursorPosition(false);
	CGWarpMouseCursorPosition(CGPointMake(pos.x, 
		CGDisplayBounds(CGMainDisplayID()).size.height - pos.y));
	CGAssociateMouseAndMouseCursorPosition(true);
}

void
In_CapturePointer(bool capture)
{
	(void)capture;
}

void
In_ShowPointer(bool show)
{
	if (show)
		CGDisplayShowCursor(kCGNullDirectDisplay);
	else
		CGDisplayHideCursor(kCGNullDirectDisplay);
}

enum Button
_mapKey(int key)
{
	switch (key) {
	case kVK_ANSI_0: return BTN_KEY_0;
	case kVK_ANSI_1: return BTN_KEY_1;
	case kVK_ANSI_2: return BTN_KEY_2;
	case kVK_ANSI_3: return BTN_KEY_3;
	case kVK_ANSI_4: return BTN_KEY_4;
	case kVK_ANSI_5: return BTN_KEY_5;
	case kVK_ANSI_6: return BTN_KEY_6;
	case kVK_ANSI_7: return BTN_KEY_7;
	case kVK_ANSI_8: return BTN_KEY_8;
	case kVK_ANSI_9: return BTN_KEY_9;

	case kVK_ANSI_A: return BTN_KEY_A;
	case kVK_ANSI_B: return BTN_KEY_B;
	case kVK_ANSI_C: return BTN_KEY_C;
	case kVK_ANSI_D: return BTN_KEY_D;
	case kVK_ANSI_E: return BTN_KEY_E;
	case kVK_ANSI_F: return BTN_KEY_F;
	case kVK_ANSI_G: return BTN_KEY_G;
	case kVK_ANSI_H: return BTN_KEY_H;
	case kVK_ANSI_I: return BTN_KEY_I;
	case kVK_ANSI_J: return BTN_KEY_J;
	case kVK_ANSI_K: return BTN_KEY_K;
	case kVK_ANSI_L: return BTN_KEY_L;
	case kVK_ANSI_M: return BTN_KEY_M;
	case kVK_ANSI_N: return BTN_KEY_N;
	case kVK_ANSI_O: return BTN_KEY_O;
	case kVK_ANSI_P: return BTN_KEY_P;
	case kVK_ANSI_Q: return BTN_KEY_Q;
	case kVK_ANSI_R: return BTN_KEY_R;
	case kVK_ANSI_S: return BTN_KEY_S;
	case kVK_ANSI_T: return BTN_KEY_T;
	case kVK_ANSI_U: return BTN_KEY_U;
	case kVK_ANSI_V: return BTN_KEY_V;
	case kVK_ANSI_W: return BTN_KEY_W;
	case kVK_ANSI_X: return BTN_KEY_X;
	case kVK_ANSI_Y: return BTN_KEY_Y;
	case kVK_ANSI_Z: return BTN_KEY_Z;

	case kVK_UpArrow: return BTN_KEY_UP;
	case kVK_DownArrow: return BTN_KEY_DOWN;
	case kVK_LeftArrow: return BTN_KEY_LEFT;
	case kVK_RightArrow: return BTN_KEY_RIGHT;
	case kVK_Space: return BTN_KEY_SPACE;
	case kVK_ANSI_Equal: return BTN_KEY_PLUS;
	case kVK_ANSI_Minus: return BTN_KEY_MINUS;
	case kVK_ANSI_Comma: return BTN_KEY_COMMA;
	case kVK_ANSI_Period: return BTN_KEY_PERIOD;
	//case XK_Scroll_Lock: return BTN_KEY_SCROLL;
	case kVK_Shift: return BTN_KEY_LSHIFT;
	case kVK_RightShift: return BTN_KEY_RSHIFT;
	case kVK_Option: return BTN_KEY_LALT;
	case kVK_RightOption: return BTN_KEY_RALT;
	case kVK_Command: return BTN_KEY_LSUPER;
	case 0x36: return BTN_KEY_RSUPER;
	case kVK_Control: return BTN_KEY_LCTRL;
	case kVK_RightControl: return BTN_KEY_RCTRL;
	case kVK_PageUp: return BTN_KEY_PGUP;
	case kVK_PageDown: return BTN_KEY_PGDN;
	case kVK_End: return BTN_KEY_END;
	case kVK_Home: return BTN_KEY_HOME;
	case kVK_Escape: return BTN_KEY_ESCAPE;
	case kVK_Help: return BTN_KEY_INSERT;
	case kVK_Return: return BTN_KEY_RETURN;
	case kVK_CapsLock: return BTN_KEY_CAPS;
	case kVK_ForwardDelete: return BTN_KEY_DELETE;
	case kVK_Delete: return BTN_KEY_BKSPACE;
	case kVK_Tab: return BTN_KEY_TAB;
	//case XK_Print: return BTN_KEY_PRTSCRN;
	//case XK_Pause: return BTN_KEY_PAUSE;
	case kVK_ANSI_Semicolon: return BTN_KEY_SEMICOLON;
	case kVK_ANSI_Slash: return BTN_KEY_SLASH;
	case kVK_ANSI_Grave: return BTN_KEY_TILDE;
	case kVK_ANSI_LeftBracket: return BTN_KEY_LBRACKET;
	case kVK_ANSI_RightBracket: return BTN_KEY_RBRACKET;
	case kVK_ANSI_Backslash: return BTN_KEY_BKSLASH;
	case kVK_ANSI_Quote: return BTN_KEY_QUOTE;

	case 0x47: return BTN_KEY_NUMLOCK;
	case kVK_ANSI_Keypad0: return BTN_KEY_NUM_0;
	case kVK_ANSI_Keypad1: return BTN_KEY_NUM_1;
	case kVK_ANSI_Keypad2: return BTN_KEY_NUM_2;
	case kVK_ANSI_Keypad3: return BTN_KEY_NUM_3;
	case kVK_ANSI_Keypad4: return BTN_KEY_NUM_4;
	case kVK_ANSI_Keypad5: return BTN_KEY_NUM_5;
	case kVK_ANSI_Keypad6: return BTN_KEY_NUM_6;
	case kVK_ANSI_Keypad7: return BTN_KEY_NUM_7;
	case kVK_ANSI_Keypad8: return BTN_KEY_NUM_8;
	case kVK_ANSI_Keypad9: return BTN_KEY_NUM_9;
	case kVK_ANSI_KeypadPlus: return BTN_KEY_NUM_PLUS;
	case kVK_ANSI_KeypadMinus: return BTN_KEY_NUM_MINUS;
	case kVK_ANSI_KeypadDivide: return BTN_KEY_NUM_DIVIDE;
	case kVK_ANSI_KeypadMultiply: return BTN_KEY_NUM_MULT;
	case kVK_ANSI_KeypadDecimal: return BTN_KEY_NUM_DECIMAL;
	
	case kVK_F1: return BTN_KEY_F1;
	case kVK_F2: return BTN_KEY_F2;
	case kVK_F3: return BTN_KEY_F3;
	case kVK_F4: return BTN_KEY_F4;
	case kVK_F5: return BTN_KEY_F5;
	case kVK_F6: return BTN_KEY_F6;
	case kVK_F7: return BTN_KEY_F7;
	case kVK_F8: return BTN_KEY_F8;
	case kVK_F9: return BTN_KEY_F9;
	case kVK_F10: return BTN_KEY_F10;
	case kVK_F11: return BTN_KEY_F11;
	case kVK_F12: return BTN_KEY_F12;
	case kVK_F13: return BTN_KEY_F13;
	case kVK_F14: return BTN_KEY_F14;
	case kVK_F15: return BTN_KEY_F15;
	case kVK_F16: return BTN_KEY_F16;
	case kVK_F17: return BTN_KEY_F17;
	case kVK_F18: return BTN_KEY_F18;
	case kVK_F19: return BTN_KEY_F19;
	case kVK_F20: return BTN_KEY_F20;
	/*case kVK_F21: return BTN_KEY_F21;
	case kVK_F22: return BTN_KEY_F22;
	case kVK_F23: return BTN_KEY_F23;
	case kVK_F24: return BTN_KEY_F24;*/
	}

	return BTN_KEY_UNRECOGNIZED;
}
