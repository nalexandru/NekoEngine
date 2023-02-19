#include <Audio/Audio.h>
#include <Script/Script.h>
#include <Runtime/Runtime.h>

#include "Interface.h"

SIF_FUNC(SetDistanceModel)
{
	Au_DistanceModel((enum NeDistanceModel)luaL_checkinteger(vm, 1));
	return 0;
};

SIF_FUNC(SetListenerPosition)
{
	Au_ListenerPositionF(
		(float)luaL_checknumber(vm, 1),
		(float)luaL_checknumber(vm, 2),
		(float)luaL_checknumber(vm, 3)
	);
	return 0;
}

SIF_FUNC(SetListenerVelocity)
{
	Au_ListenerVelocityF(
		(float)luaL_checknumber(vm, 1),
		(float)luaL_checknumber(vm, 2),
		(float)luaL_checknumber(vm, 3)
	);
	return 0;
}

SIF_FUNC(SetListenerOrientation)
{
	const float orientation[6] =
	{
		(float)luaL_checknumber(vm, 1),
		(float)luaL_checknumber(vm, 2),
		(float)luaL_checknumber(vm, 3),
		(float)luaL_checknumber(vm, 4),
		(float)luaL_checknumber(vm, 5),
		(float)luaL_checknumber(vm, 6)
	};
	Au_ListenerOrientation(orientation);
	return 0;
}

SIF_FUNC(SetPosition)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	Au_SetPositionF(
		lua_touserdata(vm, 1),
		(float)luaL_checknumber(vm, 2),
		(float)luaL_checknumber(vm, 3),
		(float)luaL_checknumber(vm, 4)
	);

	return 0;
}

SIF_FUNC(SetVelocity)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	Au_SetVelocityF(
		lua_touserdata(vm, 1),
		(float)luaL_checknumber(vm, 2),
		(float)luaL_checknumber(vm, 3),
		(float)luaL_checknumber(vm, 4)
	);

	return 0;
}

SIF_FUNC(SetDirection)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	Au_SetDirectionF(
		lua_touserdata(vm, 1),
		(float)luaL_checknumber(vm, 2),
		(float)luaL_checknumber(vm, 3),
		(float)luaL_checknumber(vm, 4)
	);

	return 0;
}

SIF_OBJ_VOID(Play, Au_Play);
SIF_OBJ_VOID(Pause, Au_Pause);
SIF_OBJ_VOID(Stop, Au_Stop);
SIF_OBJ_VOID(Rewind, Au_Rewind);

SIF_FUNC(IsPlaying)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	lua_pushboolean(vm, Au_IsPlaying(lua_touserdata(vm, 1)));

	return 1;
}

SIF_FUNC(SetGain)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	Au_Gain(lua_touserdata(vm, 1), (float)luaL_checknumber(vm, 2));

	return 0;
}

SIF_FUNC(SetPitch)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	Au_Pitch(lua_touserdata(vm, 1), (float)luaL_checknumber(vm, 2));

	return 0;
}

SIF_FUNC(SetCone)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	Au_Cone(lua_touserdata(vm, 1), (float)luaL_checknumber(vm, 2),
		(float)luaL_checknumber(vm, 3), (float)luaL_checknumber(vm, 4));

	return 0;
}

SIF_FUNC(SetMaxDistance)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	Au_MaxDistance(lua_touserdata(vm, 1), (float)luaL_checknumber(vm, 2));

	return 0;
}

SIF_FUNC(SetRefDistance)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	Au_RefDistance(lua_touserdata(vm, 1), (float)luaL_checknumber(vm, 2));

	return 0;
}

SIF_FUNC(SetClip)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	if (!lua_islightuserdata(vm, 2))
		luaL_argerror(vm, 2, "Must be light user data");

	Au_SetClip(lua_touserdata(vm, 1), (NeHandle)lua_touserdata(vm, 2));

	return 0;
}

void
SIface_OpenAudio(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		SIF_REG(IsPlaying),
		SIF_REG(Play),
		SIF_REG(Pause),
		SIF_REG(Stop),
		SIF_REG(Rewind),
		SIF_REG(SetPosition),
		SIF_REG(SetVelocity),
		SIF_REG(SetDirection),
		SIF_REG(SetGain),
		SIF_REG(SetPitch),
		SIF_REG(SetCone),
		SIF_REG(SetClip),
		SIF_REG(SetMaxDistance),
		SIF_REG(SetRefDistance),
		SIF_REG(SetDistanceModel),
		SIF_REG(SetListenerPosition),
		SIF_REG(SetListenerVelocity),
		SIF_REG(SetListenerOrientation),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Audio");

	lua_pushinteger(vm, DM_INVERSE_DISTANCE);
	lua_setglobal(vm, "DM_INVERSE_DISTANCE");

	lua_pushinteger(vm, DM_INVERSE_DISTANCE_CLAMPED);
	lua_setglobal(vm, "DM_INVERSE_DISTANCE_CLAMPED");

	lua_pushinteger(vm, DM_LINEAR_DISTANCE);
	lua_setglobal(vm, "DM_LINEAR_DISTANCE");

	lua_pushinteger(vm, DM_LINEAR_DISTANCE_CLAMPED);
	lua_setglobal(vm, "DM_LINEAR_DISTANCE_CLAMPED");

	lua_pushinteger(vm, DM_EXPONENT_DISTANCE);
	lua_setglobal(vm, "DM_EXPONENT_DISTANCE");

	lua_pushinteger(vm, DM_EXPONENT_DISTANCE_CLAMPED);
	lua_setglobal(vm, "DM_EXPONENT_DISTANCE_CLAMPED");
}

/* NekoEngine
 *
 * l_Audio.c
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
