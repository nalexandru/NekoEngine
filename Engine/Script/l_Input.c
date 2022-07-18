#include <Input/Input.h>
#include <Script/Script.h>
#include <Runtime/Runtime.h>

#include "Interface.h"

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
	uint16_t x, y;
	In_PointerPosition(&x, &y);
	lua_pushinteger(vm, x);
	lua_pushinteger(vm, y);
	return 2;
}

SIF_FUNC(SetPointerPosition)
{
	In_SetPointerPosition((uint16_t)luaL_checkinteger(vm, 1), (uint16_t)luaL_checkinteger(vm, 2));
	return 0;
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
	lua_pushboolean(vm, In_UnmappedButton((enum NeButton)luaL_checkinteger(vm, 1), (uint8_t)luaL_checkinteger(vm, 2)));
	return 1;
}

SIF_FUNC(UnmappedButtonUp)
{
	lua_pushboolean(vm, In_UnmappedButtonUp((enum NeButton)luaL_checkinteger(vm, 1), (uint8_t)luaL_checkinteger(vm, 2)));
	return 1;
}

SIF_FUNC(UnmappedButtonDown)
{
	lua_pushboolean(vm, In_UnmappedButtonDown((enum NeButton)luaL_checkinteger(vm, 1), (uint8_t)luaL_checkinteger(vm, 2)));
	return 1;
}

SIF_FUNC(UnmappedAxis)
{
	lua_pushnumber(vm, In_UnmappedAxis((enum NeAxis)luaL_checkinteger(vm, 1), (uint8_t)luaL_checkinteger(vm, 2)));
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

SIF_FUNC(MapPrimaryButton)
{
	In_MapPrimaryButton((uint32_t)luaL_checkinteger(vm, 1), (enum NeButton)luaL_checkinteger(vm, 2), (uint8_t)luaL_checkinteger(vm, 3));
	return 0;
}

SIF_FUNC(MapSecondaryButton)
{
	In_MapSecondaryButton((uint32_t)luaL_checkinteger(vm, 1), (enum NeButton)luaL_checkinteger(vm, 2), (uint8_t)luaL_checkinteger(vm, 3));
	return 0;
}

SIF_FUNC(MapPrimaryAxis)
{
	In_MapPrimaryAxis((uint32_t)luaL_checkinteger(vm, 1), (enum NeAxis)luaL_checkinteger(vm, 2), (uint8_t)luaL_checkinteger(vm, 3));
	return 0;
}

SIF_FUNC(MapSecondaryAxis)
{
	In_MapSecondaryAxis((uint32_t)luaL_checkinteger(vm, 1), (enum NeAxis)luaL_checkinteger(vm, 2), (uint8_t)luaL_checkinteger(vm, 3));
	return 0;
}

void
SIface_OpenInput(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		SIF_REG(Button),
		SIF_REG(ButtonUp),
		SIF_REG(ButtonDown),
		SIF_REG(Axis),
		SIF_REG(PointerPosition),
		SIF_REG(SetPointerPosition),
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
		SIF_REG(MapPrimaryButton),
		SIF_REG(MapSecondaryButton),
		SIF_REG(MapPrimaryAxis),
		SIF_REG(MapSecondaryAxis),
		
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "In");
}