#include <Input/Input.h>
#include <Runtime/Runtime.h>
#include <Script/Interface.h>

SIF_FUNC(Button)
{
	lua_pushboolean(vm, In_Button((uint32_t)luaL_checkinteger(vm, 1)));
	return 1;
}

SIF_FUNC(ButtonUp)
{
	lua_pushboolean(vm, In_ButtonUp((uint32_t)luaL_checkinteger(vm, 1)));
	return 1;
}

SIF_FUNC(ButtonDown)
{
	lua_pushboolean(vm, In_ButtonDown((uint32_t)luaL_checkinteger(vm, 1)));
	return 1;
}

SIF_FUNC(Axis)
{
	lua_pushnumber(vm, In_Axis((uint32_t)luaL_checkinteger(vm, 1)));
	return 1;
}

SIF_FUNC(PointerPosition)
{
	if (lua_isinteger(vm, 1) && lua_isinteger(vm, 2)) {
		In_SetPointerPosition((uint16_t)lua_tointeger(vm, 1), (uint16_t)lua_tointeger(vm, 2));
		return 0;
	} else {
		uint16_t x, y;
		In_PointerPosition(&x, &y);
		lua_pushinteger(vm, x);
		lua_pushinteger(vm, y);
		return 2;
	}
}

SIF_FUNC(CapturePointer)
{
	if (!lua_isboolean(vm, 1))
		luaL_argerror(vm, 1, "");
	In_CapturePointer(lua_toboolean(vm, 1));
	return 0;
}

SIF_FUNC(ShowPointer)
{
	if (!lua_isboolean(vm, 1))
		luaL_argerror(vm, 1, "");
	In_ShowPointer(lua_toboolean(vm, 1));
	return 0;
}

SIF_FUNC(EnableMouseAxis)
{
	if (!lua_isboolean(vm, 1))
		luaL_argerror(vm, 1, "");
	In_EnableMouseAxis(lua_toboolean(vm, 1));
	return 0;
}

SIF_FUNC(UnmappedButton)
{
	lua_pushboolean(vm, In_UnmappedButton((enum NeButton)luaL_checkinteger(vm, 1), (uint8_t)luaL_optinteger(vm, 2, 0)));
	return 1;
}

SIF_FUNC(UnmappedButtonUp)
{
	lua_pushboolean(vm, In_UnmappedButtonUp((enum NeButton)luaL_checkinteger(vm, 1), (uint8_t)luaL_optinteger(vm, 2, 0)));
	return 1;
}

SIF_FUNC(UnmappedButtonDown)
{
	lua_pushboolean(vm, In_UnmappedButtonDown((enum NeButton)luaL_checkinteger(vm, 1), (uint8_t)luaL_optinteger(vm, 2, 0)));
	return 1;
}

SIF_FUNC(UnmappedAxis)
{
	lua_pushnumber(vm, In_UnmappedAxis((enum NeAxis)luaL_checkinteger(vm, 1), (uint8_t)luaL_optinteger(vm, 2, 0)));
	return 1;
}

SIF_FUNC(CreateVirtualAxis)
{
	lua_pushinteger(vm, In_CreateVirtualAxis(luaL_checkstring(vm, 1), (enum NeButton)luaL_checkinteger(vm, 2), (enum NeButton)luaL_checkinteger(vm, 3)));
	return 1;
}

SIF_FUNC(GetVirtualAxis)
{
	lua_pushinteger(vm, In_GetVirtualAxis(luaL_checkstring(vm, 1)));
	return 1;
}

SIF_FUNC(CreateMap)
{
	lua_pushinteger(vm, In_CreateMap(luaL_checkstring(vm, 1)));
	return 1;
}

SIF_FUNC(MapButton)
{
	const uint32_t map = (uint32_t)luaL_checkinteger(vm, 1);
	if (lua_istable(vm, 2))
		luaL_argerror(vm, 2, "Must be a table");

	int t = SIF_GETFIELD(2, "primary");
	if (lua_istable(vm, t))
		In_MapPrimaryButton(map, (enum NeButton)SIF_INTFIELD(t, "button"), SIF_OPTINTFIELD(t, "controller", 0));
	SIF_POPFIELD(t);

	t = SIF_GETFIELD(2, "secondary");
	if (lua_istable(vm, t))
		In_MapSecondaryButton(map, (enum NeButton)SIF_INTFIELD(t, "button"), SIF_OPTINTFIELD(t, "controller", 0));
	SIF_POPFIELD(t);

	return 0;
}

SIF_FUNC(MapAxis)
{
	const uint32_t map = (uint32_t)luaL_checkinteger(vm, 1);
	if (lua_istable(vm, 2))
		luaL_argerror(vm, 2, "Must be a table");

	int t = SIF_GETFIELD(2, "primary");
	if (lua_istable(vm, t))
		In_MapPrimaryAxis(map, (enum NeAxis)SIF_INTFIELD(t, "axis"), SIF_OPTINTFIELD(t, "controller", 0));
	SIF_POPFIELD(t);

	t = SIF_GETFIELD(2, "secondary");
	if (lua_istable(vm, t))
		In_MapSecondaryAxis(map, (enum NeAxis)SIF_INTFIELD(t, "axis"), SIF_OPTINTFIELD(t, "controller", 0));
	SIF_POPFIELD(t);

	return 0;
}

NE_SCRIPT_INTEFACE(NeInput)
{
	luaL_Reg reg[] =
	{
		SIF_REG(Button),
		SIF_REG(ButtonUp),
		SIF_REG(ButtonDown),
		SIF_REG(Axis),
		SIF_REG(PointerPosition),
		SIF_REG(CapturePointer),
		SIF_REG(ShowPointer),
		SIF_REG(UnmappedButton),
		SIF_REG(UnmappedButtonUp),
		SIF_REG(UnmappedButtonDown),
		SIF_REG(UnmappedAxis),
		SIF_REG(EnableMouseAxis),
		SIF_REG(CreateVirtualAxis),
		SIF_REG(GetVirtualAxis),
		SIF_REG(CreateMap),
		SIF_REG(MapButton),
		SIF_REG(MapAxis),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Input");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, BTN_KEY_0);
		lua_setfield(vm, -2, "0");
		lua_pushinteger(vm, BTN_KEY_1);
		lua_setfield(vm, -2, "1");
		lua_pushinteger(vm, BTN_KEY_2);
		lua_setfield(vm, -2, "2");
		lua_pushinteger(vm, BTN_KEY_3);
		lua_setfield(vm, -2, "3");
		lua_pushinteger(vm, BTN_KEY_4);
		lua_setfield(vm, -2, "4");
		lua_pushinteger(vm, BTN_KEY_5);
		lua_setfield(vm, -2, "5");
		lua_pushinteger(vm, BTN_KEY_6);
		lua_setfield(vm, -2, "6");
		lua_pushinteger(vm, BTN_KEY_7);
		lua_setfield(vm, -2, "7");
		lua_pushinteger(vm, BTN_KEY_8);
		lua_setfield(vm, -2, "8");
		lua_pushinteger(vm, BTN_KEY_9);
		lua_setfield(vm, -2, "9");
		lua_pushinteger(vm, BTN_KEY_A);
		lua_setfield(vm, -2, "A");
		lua_pushinteger(vm, BTN_KEY_B);
		lua_setfield(vm, -2, "B");
		lua_pushinteger(vm, BTN_KEY_C);
		lua_setfield(vm, -2, "C");
		lua_pushinteger(vm, BTN_KEY_D);
		lua_setfield(vm, -2, "D");
		lua_pushinteger(vm, BTN_KEY_E);
		lua_setfield(vm, -2, "E");
		lua_pushinteger(vm, BTN_KEY_F);
		lua_setfield(vm, -2, "F");
		lua_pushinteger(vm, BTN_KEY_G);
		lua_setfield(vm, -2, "G");
		lua_pushinteger(vm, BTN_KEY_H);
		lua_setfield(vm, -2, "H");
		lua_pushinteger(vm, BTN_KEY_I);
		lua_setfield(vm, -2, "I");
		lua_pushinteger(vm, BTN_KEY_J);
		lua_setfield(vm, -2, "J");
		lua_pushinteger(vm, BTN_KEY_K);
		lua_setfield(vm, -2, "K");
		lua_pushinteger(vm, BTN_KEY_L);
		lua_setfield(vm, -2, "L");
		lua_pushinteger(vm, BTN_KEY_M);
		lua_setfield(vm, -2, "M");
		lua_pushinteger(vm, BTN_KEY_N);
		lua_setfield(vm, -2, "N");
		lua_pushinteger(vm, BTN_KEY_O);
		lua_setfield(vm, -2, "O");
		lua_pushinteger(vm, BTN_KEY_P);
		lua_setfield(vm, -2, "P");
		lua_pushinteger(vm, BTN_KEY_Q);
		lua_setfield(vm, -2, "Q");
		lua_pushinteger(vm, BTN_KEY_R);
		lua_setfield(vm, -2, "R");
		lua_pushinteger(vm, BTN_KEY_S);
		lua_setfield(vm, -2, "S");
		lua_pushinteger(vm, BTN_KEY_T);
		lua_setfield(vm, -2, "T");
		lua_pushinteger(vm, BTN_KEY_U);
		lua_setfield(vm, -2, "U");
		lua_pushinteger(vm, BTN_KEY_V);
		lua_setfield(vm, -2, "V");
		lua_pushinteger(vm, BTN_KEY_W);
		lua_setfield(vm, -2, "W");
		lua_pushinteger(vm, BTN_KEY_X);
		lua_setfield(vm, -2, "X");
		lua_pushinteger(vm, BTN_KEY_Y);
		lua_setfield(vm, -2, "Y");
		lua_pushinteger(vm, BTN_KEY_Z);
		lua_setfield(vm, -2, "Z");
		lua_pushinteger(vm, BTN_KEY_UP);
		lua_setfield(vm, -2, "Up");
		lua_pushinteger(vm, BTN_KEY_DOWN);
		lua_setfield(vm, -2, "Down");
		lua_pushinteger(vm, BTN_KEY_LEFT);
		lua_setfield(vm, -2, "Left");
		lua_pushinteger(vm, BTN_KEY_RIGHT);
		lua_setfield(vm, -2, "Right");
		lua_pushinteger(vm, BTN_KEY_SPACE);
		lua_setfield(vm, -2, "Space");
		lua_pushinteger(vm, BTN_KEY_TILDE);
		lua_setfield(vm, -2, "Tilde");
		lua_pushinteger(vm, BTN_KEY_TAB);
		lua_setfield(vm, -2, "Tab");
		lua_pushinteger(vm, BTN_KEY_CAPS);
		lua_setfield(vm, -2, "CapsLock");
		lua_pushinteger(vm, BTN_KEY_RETURN);
		lua_setfield(vm, -2, "Return");
		lua_pushinteger(vm, BTN_KEY_LSHIFT);
		lua_setfield(vm, -2, "LeftShift");
		lua_pushinteger(vm, BTN_KEY_RSHIFT);
		lua_setfield(vm, -2, "RightShift");
		lua_pushinteger(vm, BTN_KEY_LALT);
		lua_setfield(vm, -2, "LeftAlt");
		lua_pushinteger(vm, BTN_KEY_RALT);
		lua_setfield(vm, -2, "RightAlt");
		lua_pushinteger(vm, BTN_KEY_LSUPER);
		lua_setfield(vm, -2, "LeftSuper");
		lua_pushinteger(vm, BTN_KEY_RSUPER);
		lua_setfield(vm, -2, "RightSuper");
		lua_pushinteger(vm, BTN_KEY_LCTRL);
		lua_setfield(vm, -2, "LeftControl");
		lua_pushinteger(vm, BTN_KEY_RCTRL);
		lua_setfield(vm, -2, "RightControl");
		lua_pushinteger(vm, BTN_KEY_SLASH);
		lua_setfield(vm, -2, "Slash");
		lua_pushinteger(vm, BTN_KEY_BKSLASH);
		lua_setfield(vm, -2, "Backslash");
		lua_pushinteger(vm, BTN_KEY_COMMA);
		lua_setfield(vm, -2, "Comma");
		lua_pushinteger(vm, BTN_KEY_PERIOD);
		lua_setfield(vm, -2, "Period");
		lua_pushinteger(vm, BTN_KEY_SEMICOLON);
		lua_setfield(vm, -2, "Semicolon");
		lua_pushinteger(vm, BTN_KEY_QUOTE);
		lua_setfield(vm, -2, "Quote");
		lua_pushinteger(vm, BTN_KEY_DELETE);
		lua_setfield(vm, -2, "Delete");
		lua_pushinteger(vm, BTN_KEY_EQUAL);
		lua_setfield(vm, -2, "Equal");
		lua_pushinteger(vm, BTN_KEY_MINUS);
		lua_setfield(vm, -2, "Minus");
		lua_pushinteger(vm, BTN_KEY_BKSPACE);
		lua_setfield(vm, -2, "Backspace");
		lua_pushinteger(vm, BTN_KEY_LBRACKET);
		lua_setfield(vm, -2, "LeftBracket");
		lua_pushinteger(vm, BTN_KEY_RBRACKET);
		lua_setfield(vm, -2, "RightBracket");
		lua_pushinteger(vm, BTN_KEY_ESCAPE);
		lua_setfield(vm, -2, "Escape");
		lua_pushinteger(vm, BTN_KEY_PGUP);
		lua_setfield(vm, -2, "PageUp");
		lua_pushinteger(vm, BTN_KEY_PGDN);
		lua_setfield(vm, -2, "PageDown");
		lua_pushinteger(vm, BTN_KEY_HOME);
		lua_setfield(vm, -2, "Home");
		lua_pushinteger(vm, BTN_KEY_END);
		lua_setfield(vm, -2, "End");
		lua_pushinteger(vm, BTN_KEY_INSERT);
		lua_setfield(vm, -2, "Insert");
		lua_pushinteger(vm, BTN_KEY_SCROLL);
		lua_setfield(vm, -2, "Scroll");
		lua_pushinteger(vm, BTN_KEY_PRTSCRN);
		lua_setfield(vm, -2, "PrintScreen");
		lua_pushinteger(vm, BTN_KEY_PAUSE);
		lua_setfield(vm, -2, "Pause");
		lua_pushinteger(vm, BTN_KEY_F1);
		lua_setfield(vm, -2, "F1");
		lua_pushinteger(vm, BTN_KEY_F2);
		lua_setfield(vm, -2, "F2");
		lua_pushinteger(vm, BTN_KEY_F3);
		lua_setfield(vm, -2, "F3");
		lua_pushinteger(vm, BTN_KEY_F4);
		lua_setfield(vm, -2, "F4");
		lua_pushinteger(vm, BTN_KEY_F5);
		lua_setfield(vm, -2, "F5");
		lua_pushinteger(vm, BTN_KEY_F6);
		lua_setfield(vm, -2, "F6");
		lua_pushinteger(vm, BTN_KEY_F7);
		lua_setfield(vm, -2, "F7");
		lua_pushinteger(vm, BTN_KEY_F8);
		lua_setfield(vm, -2, "F8");
		lua_pushinteger(vm, BTN_KEY_F9);
		lua_setfield(vm, -2, "F9");
		lua_pushinteger(vm, BTN_KEY_F10);
		lua_setfield(vm, -2, "F10");
		lua_pushinteger(vm, BTN_KEY_F11);
		lua_setfield(vm, -2, "F11");
		lua_pushinteger(vm, BTN_KEY_F12);
		lua_setfield(vm, -2, "F12");
		lua_pushinteger(vm, BTN_KEY_F13);
		lua_setfield(vm, -2, "F13");
		lua_pushinteger(vm, BTN_KEY_F14);
		lua_setfield(vm, -2, "F14");
		lua_pushinteger(vm, BTN_KEY_F15);
		lua_setfield(vm, -2, "F15");
		lua_pushinteger(vm, BTN_KEY_F16);
		lua_setfield(vm, -2, "F16");
		lua_pushinteger(vm, BTN_KEY_F17);
		lua_setfield(vm, -2, "F17");
		lua_pushinteger(vm, BTN_KEY_F18);
		lua_setfield(vm, -2, "F18");
		lua_pushinteger(vm, BTN_KEY_F19);
		lua_setfield(vm, -2, "F19");
		lua_pushinteger(vm, BTN_KEY_F20);
		lua_setfield(vm, -2, "F20");
		lua_pushinteger(vm, BTN_KEY_F21);
		lua_setfield(vm, -2, "F21");
		lua_pushinteger(vm, BTN_KEY_F22);
		lua_setfield(vm, -2, "F22");
		lua_pushinteger(vm, BTN_KEY_F23);
		lua_setfield(vm, -2, "F23");
		lua_pushinteger(vm, BTN_KEY_F24);
		lua_setfield(vm, -2, "F24");
		lua_pushinteger(vm, BTN_KEY_NUMLOCK);
		lua_setfield(vm, -2, "NumLock");
		lua_pushinteger(vm, BTN_KEY_NUM_0);
		lua_setfield(vm, -2, "Numpad0");
		lua_pushinteger(vm, BTN_KEY_NUM_1);
		lua_setfield(vm, -2, "Numpad1");
		lua_pushinteger(vm, BTN_KEY_NUM_2);
		lua_setfield(vm, -2, "Numpad2");
		lua_pushinteger(vm, BTN_KEY_NUM_3);
		lua_setfield(vm, -2, "Numpad3");
		lua_pushinteger(vm, BTN_KEY_NUM_4);
		lua_setfield(vm, -2, "Numpad4");
		lua_pushinteger(vm, BTN_KEY_NUM_5);
		lua_setfield(vm, -2, "Numpad5");
		lua_pushinteger(vm, BTN_KEY_NUM_6);
		lua_setfield(vm, -2, "Numpad6");
		lua_pushinteger(vm, BTN_KEY_NUM_7);
		lua_setfield(vm, -2, "Numpad7");
		lua_pushinteger(vm, BTN_KEY_NUM_8);
		lua_setfield(vm, -2, "Numpad8");
		lua_pushinteger(vm, BTN_KEY_NUM_9);
		lua_setfield(vm, -2, "Numpad9");
		lua_pushinteger(vm, BTN_KEY_NUM_PLUS);
		lua_setfield(vm, -2, "NumpadPlus");
		lua_pushinteger(vm, BTN_KEY_NUM_MINUS);
		lua_setfield(vm, -2, "NumpadMinus");
		lua_pushinteger(vm, BTN_KEY_NUM_DECIMAL);
		lua_setfield(vm, -2, "NumpadDecimal");
		lua_pushinteger(vm, BTN_KEY_NUM_DIVIDE);
		lua_setfield(vm, -2, "NumpadDivide");
		lua_pushinteger(vm, BTN_KEY_NUM_MULT);
		lua_setfield(vm, -2, "NumpadMultiply");
		lua_pushinteger(vm, BTN_KEY_NUM_RETURN);
		lua_setfield(vm, -2, "NumpadReturn");
		lua_pushinteger(vm, BTN_KEY_CLEAR);
		lua_setfield(vm, -2, "NumpadClear");
		lua_pushinteger(vm, BTN_MOUSE_LMB);
		lua_setfield(vm, -2, "LeftMouse");
		lua_pushinteger(vm, BTN_MOUSE_RMB);
		lua_setfield(vm, -2, "RightMouse");
		lua_pushinteger(vm, BTN_MOUSE_MMB);
		lua_setfield(vm, -2, "MiddleMouse");
		lua_pushinteger(vm, BTN_MOUSE_BTN4);
		lua_setfield(vm, -2, "Mouse4");
		lua_pushinteger(vm, BTN_MOUSE_BTN5);
		lua_setfield(vm, -2, "Mouse5");
		lua_pushinteger(vm, BTN_GPAD_BTN_D_UP);
		lua_setfield(vm, -2, "Gpad_Up");
		lua_pushinteger(vm, BTN_GPAD_BTN_D_DOWN);
		lua_setfield(vm, -2, "Gpad_Down");
		lua_pushinteger(vm, BTN_GPAD_BTN_D_LEFT);
		lua_setfield(vm, -2, "Gpad_Left");
		lua_pushinteger(vm, BTN_GPAD_BTN_D_RIGHT);
		lua_setfield(vm, -2, "Gpad_Right");
		lua_pushinteger(vm, BTN_GPAD_BTN_START);
		lua_setfield(vm, -2, "Gpad_Start");
		lua_pushinteger(vm, BTN_GPAD_BTN_BACK);
		lua_setfield(vm, -2, "Gpad_Back");
		lua_pushinteger(vm, BTN_GPAD_BTN_LTHUMB);
		lua_setfield(vm, -2, "Gpad_LeftThumb");
		lua_pushinteger(vm, BTN_GPAD_BTN_RTHUMB);
		lua_setfield(vm, -2, "Gpad_RightThumb");
		lua_pushinteger(vm, BTN_GPAD_BTN_LBUMPER);
		lua_setfield(vm, -2, "Gpad_LeftBumper");
		lua_pushinteger(vm, BTN_GPAD_BTN_RBUMPER);
		lua_setfield(vm, -2, "Gpad_RightBumper");
		lua_pushinteger(vm, BTN_GPAD_BTN_A);
		lua_setfield(vm, -2, "Gpad_A");
		lua_pushinteger(vm, BTN_GPAD_BTN_B);
		lua_setfield(vm, -2, "Gpad_B");
		lua_pushinteger(vm, BTN_GPAD_BTN_X);
		lua_setfield(vm, -2, "Gpad_X");
		lua_pushinteger(vm, BTN_GPAD_BTN_Y);
		lua_setfield(vm, -2, "Gpad_Y");
	}
	lua_setglobal(vm, "KeyCode");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, AXIS_LSTICK_X);
		lua_setfield(vm, -2, "LeftStickX");
		lua_pushinteger(vm, AXIS_LSTICK_Y);
		lua_setfield(vm, -2, "LeftStickY");
		lua_pushinteger(vm, AXIS_RSTICK_X);
		lua_setfield(vm, -2, "RightStickX");
		lua_pushinteger(vm, AXIS_RSTICK_Y);
		lua_setfield(vm, -2, "RightStickY");
		lua_pushinteger(vm, AXIS_LTRIGGER);
		lua_setfield(vm, -2, "LeftTrigger");
		lua_pushinteger(vm, AXIS_RTRIGGER);
		lua_setfield(vm, -2, "RightTrigger");
		lua_pushinteger(vm, AXIS_GYRO_X);
		lua_setfield(vm, -2, "GyroscopeX");
		lua_pushinteger(vm, AXIS_GYRO_Y);
		lua_setfield(vm, -2, "GyroscopeY");
		lua_pushinteger(vm, AXIS_GYRO_Z);
		lua_setfield(vm, -2, "GyroscopeZ");
		lua_pushinteger(vm, AXIS_MOUSE_X);
		lua_setfield(vm, -2, "MouseX");
		lua_pushinteger(vm, AXIS_MOUSE_Y);
		lua_setfield(vm, -2, "MouseY");
		lua_pushinteger(vm, AXIS_MOUSE_WHEEL);
		lua_setfield(vm, -2, "MouseWheel");
		lua_pushinteger(vm, AXIS_VIRTUAL_0);
		lua_setfield(vm, -2, "Virtual0");
		lua_pushinteger(vm, AXIS_VIRTUAL_1);
		lua_setfield(vm, -2, "Virtual1");
		lua_pushinteger(vm, AXIS_VIRTUAL_2);
		lua_setfield(vm, -2, "Virtual2");
		lua_pushinteger(vm, AXIS_VIRTUAL_3);
		lua_setfield(vm, -2, "Virtual3");
		lua_pushinteger(vm, AXIS_VIRTUAL_4);
		lua_setfield(vm, -2, "Virtual4");
		lua_pushinteger(vm, AXIS_VIRTUAL_5);
		lua_setfield(vm, -2, "Virtual5");
		lua_pushinteger(vm, AXIS_VIRTUAL_6);
		lua_setfield(vm, -2, "Virtual6");
		lua_pushinteger(vm, AXIS_VIRTUAL_7);
		lua_setfield(vm, -2, "Virtual7");
		lua_pushinteger(vm, AXIS_VIRTUAL_8);
		lua_setfield(vm, -2, "Virtual8");
		lua_pushinteger(vm, AXIS_VIRTUAL_9);
		lua_setfield(vm, -2, "Virtual9");
	}
	lua_setglobal(vm, "Axis");

	return 1;
}

/* NekoEngine
 *
 * l_Input.c
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
