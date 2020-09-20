#include <wrl.h>
#include <wrl/client.h>
#include <Xinput.h>

#include <math.h>

#include <System/Log.h>
#include <Input/Input.h>
#include <Engine/Engine.h>

#include "UWPPlatform.h"

using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::Foundation;
using namespace Windows::Devices::Input;

#define UWPINMOD	L"UWPInput"

#define READ_AXIS(cs, srcx, srcy, dstx, dsty, m, dz)	\
{														\
	float x = srcx;										\
	float y = srcy;										\
	_deadzone(&x, &y, m, dz);							\
	cs->axis[dstx] = x / m;								\
	cs->axis[dsty] = y / m;								\
}

static enum Button _keymap[256];
static DWORD _lastPacket[IN_MAX_CONTROLLERS];
static CoreCursor ^_cursor;

static void _KeyUp(KeyboardDeliveryInterceptor ^kdi, KeyEventArgs ^args);
static void _KeyDown(KeyboardDeliveryInterceptor ^kdi, KeyEventArgs ^args);
static void _MouseMoved(MouseDevice ^md, MouseEventArgs ^args);
static void _WheelChanged(CoreWindow ^win, PointerEventArgs ^args);
static inline void _deadzone(float *x, float *y, const float max, const float deadzone);
static inline enum Button _mapKey(const int key);

bool
In_SysInit(void)
{
	_cursor = reinterpret_cast<CoreWindow ^>(E_Screen)->PointerCursor;

	KeyboardDeliveryInterceptor ^kdi = KeyboardDeliveryInterceptor::GetForCurrentView();
	kdi->KeyUp += ref new TypedEventHandler<KeyboardDeliveryInterceptor ^, KeyEventArgs ^>(&_KeyUp);
	kdi->KeyDown += ref new TypedEventHandler<KeyboardDeliveryInterceptor ^, KeyEventArgs ^>(&_KeyDown);
	kdi->IsInterceptionEnabledWhenInForeground = true;

	MouseDevice::GetForCurrentView()->MouseMoved += ref new TypedEventHandler<MouseDevice ^, MouseEventArgs ^>(&_MouseMoved);
	reinterpret_cast<CoreWindow ^>(E_Screen)->PointerWheelChanged += ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(&_WheelChanged);

	UpdateControllers();

	return true;
}

void
In_SysTerm(void)
{
}

void
In_SysPollControllers(void)
{
	uint8_t i;
	XINPUT_STATE xi;
	struct ControllerState *cs;

	for (i = 0; i < In_ConnectedControllers; ++i) {
		XInputGetState(i, &xi);

		if (_lastPacket[i] == xi.dwPacketNumber)
			continue;

		cs = &In_ControllerState[i];

		cs->buttons = xi.Gamepad.wButtons;

		READ_AXIS(cs, xi.Gamepad.sThumbLX, xi.Gamepad.sThumbLY, AXIS_LSTICK_X, AXIS_LSTICK_Y, 32767.f, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		READ_AXIS(cs, xi.Gamepad.sThumbRX, xi.Gamepad.sThumbRY, AXIS_RSTICK_X, AXIS_RSTICK_Y, 32767.f, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		READ_AXIS(cs, xi.Gamepad.bLeftTrigger, xi.Gamepad.bRightTrigger, AXIS_LTRIGGER, AXIS_RTRIGGER, 255.f, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

		_lastPacket[i] = xi.dwPacketNumber;
	}
}

void
In_PointerPosition(uint16_t *x, uint16_t *y)
{
	Point pt = CoreWindow::GetForCurrentThread()->PointerPosition;
	*x = (uint16_t)(pt.X - reinterpret_cast<CoreWindow ^>(E_Screen)->Bounds.X);
	*y = (uint16_t)(pt.Y - reinterpret_cast<CoreWindow ^>(E_Screen)->Bounds.Y);
}

void
In_SetPointerPosition(uint16_t x, uint16_t y)
{
	POINT pt = { x, y };

//	CoreWindow::GetForCurrentThread()->Pointer

/*	ClientToScreen((HWND)E_Screen, &pt);
	SetCursorPos(pt.x, pt.y);*/
}

void
In_CapturePointer(bool capture)
{
	if (capture)
		reinterpret_cast<CoreWindow ^>(E_Screen)->SetPointerCapture();
	else
		reinterpret_cast<CoreWindow ^>(E_Screen)->ReleasePointerCapture();

	In_PointerCaptured = capture;
}

void
In_ShowPointer(bool show)
{
	reinterpret_cast<CoreWindow ^>(E_Screen)->PointerCursor = show ? _cursor : nullptr;
	In_PointerVisible = show;
}

void
UpdateControllers(void)
{
	XINPUT_STATE xi;

	In_ConnectedControllers = 0;

	for (uint32_t i = 0; i < IN_MAX_CONTROLLERS; ++i)
		if (XInputGetState(i, &xi) == ERROR_SUCCESS)
			++In_ConnectedControllers;
}

void
_KeyUp(KeyboardDeliveryInterceptor ^kdi, KeyEventArgs ^args)
{
	In_ButtonState[_keymap[(uint16_t)args->VirtualKey]] = false;
}

void
_KeyDown(KeyboardDeliveryInterceptor ^kdi, KeyEventArgs ^args)
{
	In_ButtonState[_keymap[(uint16_t)args->VirtualKey]] = true;
}

void _MouseMoved(MouseDevice ^md, MouseEventArgs ^args)
{
	In_MouseAxis[0] = (float)args->MouseDelta.X;
	In_MouseAxis[1] = (float)args->MouseDelta.Y;
}

void
_WheelChanged(CoreWindow ^win, PointerEventArgs ^args)
{
// TODO
//	In_MouseAxis[2] = (float)(short)raw->data.mouse.usButtonData / WHEEL_DELTA;
}

/* TODO: Mouse buttons; this platform is mostly for Xbox so not very important

	In_ButtonState[BTN_MOUSE_LMB] = btn & RI_MOUSE_BUTTON_1_DOWN;
	In_ButtonState[BTN_MOUSE_RMB] = btn & RI_MOUSE_BUTTON_2_DOWN;
	In_ButtonState[BTN_MOUSE_MMB] = btn & RI_MOUSE_BUTTON_3_DOWN;
	In_ButtonState[BTN_MOUSE_BTN4] = btn & RI_MOUSE_BUTTON_4_DOWN;
	In_ButtonState[BTN_MOUSE_BTN5] = btn & RI_MOUSE_BUTTON_5_DOWN;

 */

static inline void
_deadzone(float *x, float *y, const float max, const float deadzone)
{
	const float x_val = *x, y_val = *y;
	float mag = sqrtf(x_val * x_val + y_val * y_val);

	if (mag < deadzone) {
		*x = 0.f;
		*y = 0.f;
	} else {
		const float n_x = x_val / mag;
		const float n_y = y_val / mag;

		if (mag > max)
			mag = max;

		mag -= deadzone;
		mag /= (max - deadzone);

		*x = x_val * mag;
		*y = y_val * mag;
	}
}

enum Button
_mapKey(const int key)
{
	switch (key) {
	case 0x30: return BTN_KEY_0;
	case 0x31: return BTN_KEY_1;
	case 0x32: return BTN_KEY_2;
	case 0x33: return BTN_KEY_3;
	case 0x34: return BTN_KEY_4;
	case 0x35: return BTN_KEY_5;
	case 0x36: return BTN_KEY_6;
	case 0x37: return BTN_KEY_7;
	case 0x38: return BTN_KEY_8;
	case 0x39: return BTN_KEY_9;

	case 0x41: return BTN_KEY_A;
	case 0x42: return BTN_KEY_B;
	case 0x43: return BTN_KEY_C;
	case 0x44: return BTN_KEY_D;
	case 0x45: return BTN_KEY_E;
	case 0x46: return BTN_KEY_F;
	case 0x47: return BTN_KEY_G;
	case 0x48: return BTN_KEY_H;
	case 0x49: return BTN_KEY_I;
	case 0x4A: return BTN_KEY_J;
	case 0x4B: return BTN_KEY_K;
	case 0x4C: return BTN_KEY_L;
	case 0x4D: return BTN_KEY_M;
	case 0x4E: return BTN_KEY_N;
	case 0x4F: return BTN_KEY_O;
	case 0x50: return BTN_KEY_P;
	case 0x51: return BTN_KEY_Q;
	case 0x52: return BTN_KEY_R;
	case 0x53: return BTN_KEY_S;
	case 0x54: return BTN_KEY_T;
	case 0x55: return BTN_KEY_U;
	case 0x56: return BTN_KEY_V;
	case 0x57: return BTN_KEY_W;
	case 0x58: return BTN_KEY_X;
	case 0x59: return BTN_KEY_Y;
	case 0x5A: return BTN_KEY_Z;

	case VK_UP: return BTN_KEY_UP;
	case VK_DOWN: return BTN_KEY_DOWN;
	case VK_LEFT: return BTN_KEY_LEFT;
	case VK_RIGHT: return BTN_KEY_RIGHT;
	case VK_SPACE: return BTN_KEY_SPACE;
	case VK_OEM_PLUS: return BTN_KEY_PLUS;
	case VK_OEM_MINUS: return BTN_KEY_MINUS;
	case VK_OEM_PERIOD: return BTN_KEY_PERIOD;
	case VK_SCROLL: return BTN_KEY_SCROLL;
	case VK_LSHIFT: return BTN_KEY_LSHIFT;
	case VK_RSHIFT: return BTN_KEY_RSHIFT;
	case VK_LMENU: return BTN_KEY_LALT;
	case VK_RMENU: return BTN_KEY_RALT;
	case VK_LWIN: return BTN_KEY_LSUPER;
	case VK_RWIN: return BTN_KEY_RSUPER;
	case VK_LCONTROL: return BTN_KEY_LCTRL;
	case VK_RCONTROL: return BTN_KEY_RCTRL;
	case VK_PRIOR: return BTN_KEY_PGUP;
	case VK_NEXT: return BTN_KEY_PGDN;
	case VK_END: return BTN_KEY_END;
	case VK_HOME: return BTN_KEY_HOME;
	case VK_ESCAPE: return BTN_KEY_ESCAPE;
	case VK_INSERT: return BTN_KEY_INSERT;
	case VK_RETURN: return BTN_KEY_RETURN;
	case VK_CAPITAL: return BTN_KEY_CAPS;
	case VK_DELETE: return BTN_KEY_DELETE;
	case VK_BACK: return BTN_KEY_BKSPACE;
	case VK_TAB: return BTN_KEY_TAB;
	case VK_SNAPSHOT: return BTN_KEY_PRTSCRN;
	case VK_PAUSE: return BTN_KEY_PAUSE;
	case VK_CLEAR: return BTN_KEY_CLEAR;
		// semicolon
		// slash
		// tilde
		// lbracket
		// rbracket
		// backslash
		// quotedbl

	case VK_NUMLOCK: return BTN_KEY_NUMLOCK;
	case VK_NUMPAD0: return BTN_KEY_NUM_0;
	case VK_NUMPAD1: return BTN_KEY_NUM_1;
	case VK_NUMPAD2: return BTN_KEY_NUM_2;
	case VK_NUMPAD3: return BTN_KEY_NUM_3;
	case VK_NUMPAD4: return BTN_KEY_NUM_4;
	case VK_NUMPAD5: return BTN_KEY_NUM_5;
	case VK_NUMPAD6: return BTN_KEY_NUM_6;
	case VK_NUMPAD7: return BTN_KEY_NUM_7;
	case VK_NUMPAD8: return BTN_KEY_NUM_8;
	case VK_NUMPAD9: return BTN_KEY_NUM_9;
	case VK_MULTIPLY: return BTN_KEY_NUM_MULT;
	case VK_ADD: return BTN_KEY_NUM_PLUS;
	case VK_SUBTRACT: return BTN_KEY_MINUS;
	case VK_DECIMAL: return BTN_KEY_NUM_DECIMAL;
	case VK_DIVIDE: return BTN_KEY_NUM_DIVIDE;

	case VK_F1: return BTN_KEY_F1;
	case VK_F2: return BTN_KEY_F2;
	case VK_F3: return BTN_KEY_F3;
	case VK_F4: return BTN_KEY_F4;
	case VK_F5: return BTN_KEY_F5;
	case VK_F6: return BTN_KEY_F6;
	case VK_F7: return BTN_KEY_F7;
	case VK_F8: return BTN_KEY_F8;
	case VK_F9: return BTN_KEY_F9;
	case VK_F10: return BTN_KEY_F10;
	case VK_F11: return BTN_KEY_F11;
	case VK_F12: return BTN_KEY_F12;
	case VK_F13: return BTN_KEY_F13;
	case VK_F14: return BTN_KEY_F14;
	case VK_F15: return BTN_KEY_F15;
	case VK_F16: return BTN_KEY_F16;
	case VK_F17: return BTN_KEY_F17;
	case VK_F18: return BTN_KEY_F18;
	case VK_F19: return BTN_KEY_F19;
	case VK_F20: return BTN_KEY_F20;
	case VK_F21: return BTN_KEY_F21;
	case VK_F22: return BTN_KEY_F22;
	case VK_F23: return BTN_KEY_F23;
	case VK_F24: return BTN_KEY_F24;
	}

	return BTN_KEY_UNRECOGNIZED;
}

