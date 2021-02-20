#include "Win32Platform.h"
#include <Xinput.h>

#include <math.h>

#include <System/Log.h>
#include <Input/Input.h>
#include <Engine/Engine.h>

#include "Win32Platform.h"

#define W32INMOD	L"Win32Input"

#ifndef MAPVK_VSC_TO_VK_EX
#define MAPVK_VSC_TO_VK_EX	3
#endif

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

static inline void _deadzone(float *x, float *y, const float max, const float deadzone);
static inline enum Button _mapKey(const int key);

bool
In_SysInit(void)
{
	uint16_t i;
	RAWINPUTDEVICE rid[2];

	for (i = 0; i < 256; ++i)
		_keymap[i] = _mapKey(i);

	rid[0].usUsagePage = 0x01;
	rid[0].usUsage = 0x02;
	rid[0].dwFlags = 0; // set this only in fullscreen RIDEV_NOLEGACY;
	rid[0].hwndTarget = (HWND)E_screen;

	rid[1].usUsagePage = 0x01;
	rid[1].usUsage = 0x06;
	rid[1].dwFlags = RIDEV_NOLEGACY;
	rid[1].hwndTarget = (HWND)E_screen;

	if (!RegisterRawInputDevices(rid, 2, sizeof(rid[0]))) {
		Sys_LogEntry(W32INMOD, LOG_CRITICAL, L"Failed to register raw input devices");
		return false;
	}

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
	if (XInputGetState) {
		uint8_t i;
		XINPUT_STATE xi;
		struct ControllerState *cs;

		for (i = 0; i < In_connectedControllers; ++i) {
			XInputGetState(i, &xi);

			if (_lastPacket[i] == xi.dwPacketNumber)
				continue;

			cs = &In_controllerState[i];

			cs->buttons = xi.Gamepad.wButtons;

			READ_AXIS(cs, xi.Gamepad.sThumbLX, xi.Gamepad.sThumbLY, AXIS_LSTICK_X, AXIS_LSTICK_Y, 32767.f, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
			READ_AXIS(cs, xi.Gamepad.sThumbRX, xi.Gamepad.sThumbRY, AXIS_RSTICK_X, AXIS_RSTICK_Y, 32767.f, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
			READ_AXIS(cs, xi.Gamepad.bLeftTrigger, xi.Gamepad.bRightTrigger, AXIS_LTRIGGER, AXIS_RTRIGGER, 255.f, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);

			_lastPacket[i] = xi.dwPacketNumber;
		}
	}
}

void
In_PointerPosition(uint16_t *x, uint16_t *y)
{
	POINT pt;
	
	GetCursorPos(&pt);
	ScreenToClient((HWND)E_screen, &pt);

	*x = (uint16_t)pt.x;
	*y = (uint16_t)pt.y;
}

void
In_SetPointerPosition(uint16_t x, uint16_t y)
{
	POINT pt = { x, y };

	ClientToScreen((HWND)E_screen, &pt);
	SetCursorPos(pt.x, pt.y);
}

void
In_CapturePointer(bool capture)
{
	if (capture)
		SetCapture((HWND)E_screen);
	else
		ReleaseCapture();

	In_pointerCaptured = capture;
}

void
In_ShowPointer(bool show)
{
	PostMessage((HWND)E_screen, show ? WM_SHOWCURSOR : WM_HIDECURSOR, 0, 0);
	In_pointerVisible = show;
}

void
UpdateControllers(void)
{
	if (XInputGetState) {
		uint32_t i;
		XINPUT_STATE xi;

		In_connectedControllers = 0;

		for (i = 0; i < IN_MAX_CONTROLLERS; ++i)
			if (XInputGetState(i, &xi) == ERROR_SUCCESS)
				++In_connectedControllers;
	}
}

void
HandleInput(HWND wnd, LPARAM lParam, WPARAM wParam)
{
	LPBYTE lpb[sizeof(RAWINPUT)];
	UINT lpb_size = sizeof(RAWINPUT);
	uint8_t key_code;
	UINT scan;
	int ext;
	RAWINPUT *raw;

	// see:
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms645546(v=vs.85).aspx

	memset(lpb, 0x0, lpb_size);

	if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT,
		lpb, &lpb_size, sizeof(RAWINPUTHEADER)) > lpb_size) {
		lpb_size = sizeof(lpb);
	}

	raw = (RAWINPUT *)lpb;

	if (raw->header.dwType == RIM_TYPEKEYBOARD) {
		scan = (lParam & 0x00FF0000) >> 16;
		ext = (lParam & 0x01000000) != 0;

		if (raw->data.keyboard.VKey == 255)
			return;

		switch (raw->data.keyboard.VKey) {
		case VK_SHIFT: key_code = MapVirtualKey(raw->data.keyboard.MakeCode, MAPVK_VSC_TO_VK_EX) == VK_LSHIFT ? BTN_KEY_LSHIFT : BTN_KEY_RSHIFT; break;
		case VK_CONTROL: key_code = raw->data.keyboard.Flags & RI_KEY_E0 ? BTN_KEY_RCTRL : BTN_KEY_LCTRL; break;
		case VK_MENU: key_code = raw->data.keyboard.Flags & RI_KEY_E0 ? BTN_KEY_RALT : BTN_KEY_LALT; break;
		case VK_RETURN: key_code = raw->data.keyboard.Flags & RI_KEY_E0 ? BTN_KEY_NUM_RETURN : BTN_KEY_RETURN; break;
		case VK_INSERT: key_code = raw->data.keyboard.Flags & RI_KEY_E0 ? BTN_KEY_INSERT : BTN_KEY_NUM_0; break;
		case VK_DELETE: key_code = raw->data.keyboard.Flags & RI_KEY_E0 ? BTN_KEY_DELETE : BTN_KEY_NUM_DECIMAL; break;
		case VK_HOME: key_code = raw->data.keyboard.Flags & RI_KEY_E0 ? BTN_KEY_HOME : BTN_KEY_NUM_7; break;
		case VK_END: key_code = raw->data.keyboard.Flags & RI_KEY_E0 ? BTN_KEY_END : BTN_KEY_NUM_1; break;
		case VK_PRIOR: key_code = raw->data.keyboard.Flags & RI_KEY_E0 ? BTN_KEY_PGUP : BTN_KEY_NUM_9; break;
		case VK_NEXT: key_code = raw->data.keyboard.Flags & RI_KEY_E0 ? BTN_KEY_PGDN : BTN_KEY_NUM_3; break;
		case VK_LEFT: key_code = raw->data.keyboard.Flags & RI_KEY_E0 ? BTN_KEY_LEFT : BTN_KEY_NUM_4; break;
		case VK_RIGHT: key_code = raw->data.keyboard.Flags & RI_KEY_E0 ? BTN_KEY_RIGHT : BTN_KEY_NUM_6; break;
		case VK_UP: key_code = raw->data.keyboard.Flags & RI_KEY_E0 ? BTN_KEY_UP : BTN_KEY_NUM_8; break;
		case VK_DOWN: key_code = raw->data.keyboard.Flags & RI_KEY_E0 ? BTN_KEY_DOWN : BTN_KEY_NUM_2; break;
		case VK_CLEAR: key_code = raw->data.keyboard.Flags & RI_KEY_E0 ? BTN_KEY_CLEAR : BTN_KEY_NUM_5;
		break;
		case VK_NUMLOCK: key_code = MapVirtualKey(raw->data.keyboard.MakeCode, MAPVK_VSC_TO_VK_EX) == VK_NUMLOCK ? BTN_KEY_NUMLOCK : BTN_KEY_PAUSE; break;
		default: key_code = _keymap[raw->data.keyboard.VKey]; break;
		}

		In_buttonState[key_code] = !(raw->data.keyboard.Flags & RI_KEY_BREAK);
	} else if (raw->header.dwType == RIM_TYPEMOUSE) {
		uint16_t btn = raw->data.mouse.usButtonFlags;

		In_buttonState[BTN_MOUSE_LMB] = btn & RI_MOUSE_BUTTON_1_DOWN;
		In_buttonState[BTN_MOUSE_RMB] = btn & RI_MOUSE_BUTTON_2_DOWN;
		In_buttonState[BTN_MOUSE_MMB] = btn & RI_MOUSE_BUTTON_3_DOWN;
		In_buttonState[BTN_MOUSE_BTN4] = btn & RI_MOUSE_BUTTON_4_DOWN;
		In_buttonState[BTN_MOUSE_BTN5] = btn & RI_MOUSE_BUTTON_5_DOWN;

		In_mouseAxis[0] = (float)raw->data.mouse.lLastX / (float)(*E_screenWidth / 2);
		In_mouseAxis[1] = (float)raw->data.mouse.lLastY / (float)(*E_screenHeight / 2);

		if ((raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL) == RI_MOUSE_WHEEL)
			In_mouseAxis[2] = (float)(short)raw->data.mouse.usButtonData / WHEEL_DELTA;
	}

	DefRawInputProc(&raw, 1, sizeof(RAWINPUTHEADER));
}

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

