#include <Audio/Audio.h>
#include <Scene/Components.h>
#include <Script/Interface.h>

#include "EngineInterface.h"

SIF_FUNC(DistanceModel)
{
	Au_DistanceModel((enum NeDistanceModel)luaL_checkinteger(vm, 1));
	return 0;
};

SIF_FUNC(ListenerPosition)
{
	luaL_checkudata(vm, 1, NE_AUDIO_LISTENER);
	float *f = luaL_testudata(vm, 2, SIF_NE_VEC3);
	if (f) {
		Au_ListenerPosition((struct NeVec3 *)f);
	} else if (luaL_checknumber(vm, 2)) {
		Au_ListenerPositionF(
			luaL_checknumber(vm, 2),
			luaL_checknumber(vm, 3),
			luaL_checknumber(vm, 4)
		);
	} else {

	}
	return 0;
}

SIF_FUNC(ListenerVelocity)
{
	luaL_checkudata(vm, 1, NE_AUDIO_LISTENER);
	float *f = luaL_testudata(vm, 2, SIF_NE_VEC3);
	if (f) {
		Au_ListenerVelocity((struct NeVec3 *)f);
	} else if (luaL_checknumber(vm, 2)) {
		Au_ListenerVelocityF(
			luaL_checknumber(vm, 2),
			luaL_checknumber(vm, 3),
			luaL_checknumber(vm, 4)
		);
	}
	return 0;
}

SIF_FUNC(ListenerOrientation)
{
	float orientation[6] = { 0 };
	luaL_checkudata(vm, 1, NE_AUDIO_LISTENER);
	float *at = luaL_testudata(vm, 2, SIF_NE_VEC3);
	if (at) {
		float *to = luaL_checkudata(vm, 3, SIF_NE_VEC3);
		orientation[0] = at[0];
		orientation[1] = at[1];
		orientation[2] = at[2];
		orientation[3] = to[0];
		orientation[4] = to[1];
		orientation[5] = to[2];
	} else if (luaL_checknumber(vm, 2)) {
		orientation[0] = luaL_checknumber(vm, 1);
		orientation[1] = luaL_checknumber(vm, 2);
		orientation[2] = luaL_checknumber(vm, 3);
		orientation[3] = luaL_checknumber(vm, 4);
		orientation[4] = luaL_checknumber(vm, 5);
		orientation[5] = luaL_checknumber(vm, 6);
	}
	Au_ListenerOrientation(orientation);
	return 0;
}

SIF_FUNC(Position)
{
	SIF_CHECKCOMPONENT(1, src, NE_AUDIO_SOURCE, struct NeAudioSource *);
	float *f = luaL_testudata(vm, 2, SIF_NE_VEC3);
	if (f)
		Au_Position(src, (struct NeVec3 *)f);
	else
		Au_PositionF(
			src,
			luaL_checknumber(vm, 2),
			luaL_checknumber(vm, 3),
			luaL_checknumber(vm, 4)
		);
	return 0;
}

SIF_FUNC(Velocity)
{
	SIF_CHECKCOMPONENT(1, src, NE_AUDIO_SOURCE, struct NeAudioSource *);
	float *f = luaL_testudata(vm, 2, SIF_NE_VEC3);
	if (f)
		Au_Velocity(src, (struct NeVec3 *)f);
	else
		Au_VelocityF(
			src,
			luaL_checknumber(vm, 2),
			luaL_checknumber(vm, 3),
			luaL_checknumber(vm, 4)
		);
	return 0;
}

SIF_FUNC(Direction)
{
	SIF_CHECKCOMPONENT(1, src, NE_AUDIO_SOURCE, struct NeAudioSource *);
	float *f = luaL_testudata(vm, 2, SIF_NE_VEC3);
	if (f)
		Au_Direction(src, (struct NeVec3 *)f);
	else
		Au_DirectionF(
			src,
			luaL_checknumber(vm, 2),
			luaL_checknumber(vm, 3),
			luaL_checknumber(vm, 4)
		);
	return 0;
}

SIF_FUNC(Play)
{
	SIF_CHECKCOMPONENT(1, src, NE_AUDIO_SOURCE, struct NeAudioSource *);
	Au_Play(src);
	return 0;
}

SIF_FUNC(Pause)
{
	SIF_CHECKCOMPONENT(1, src, NE_AUDIO_SOURCE, struct NeAudioSource *);
	Au_Pause(src);
	return 0;
}

SIF_FUNC(Stop)
{
	SIF_CHECKCOMPONENT(1, src, NE_AUDIO_SOURCE, struct NeAudioSource *);
	Au_Stop(src);
	return 0;
}

SIF_FUNC(Rewind)
{
	SIF_CHECKCOMPONENT(1, src, NE_AUDIO_SOURCE, struct NeAudioSource *);
	Au_Rewind(src);
	return 0;
}

SIF_FUNC(Playing)
{
	SIF_CHECKCOMPONENT(1, src, NE_AUDIO_SOURCE, struct NeAudioSource *);
	lua_pushboolean(vm, Au_IsPlaying(src));
	return 1;
}

SIF_FUNC(Gain)
{
	SIF_CHECKCOMPONENT(1, src, NE_AUDIO_SOURCE, struct NeAudioSource *);
	Au_Gain(src, luaL_checknumber(vm, 2));
	return 0;
}

SIF_FUNC(Pitch)
{
	SIF_CHECKCOMPONENT(1, src, NE_AUDIO_SOURCE, struct NeAudioSource *);
	Au_Pitch(src, luaL_checknumber(vm, 2));
	return 0;
}

SIF_FUNC(Cone)
{
	SIF_CHECKCOMPONENT(1, src, NE_AUDIO_SOURCE, struct NeAudioSource *);
	float *f = luaL_testudata(vm, 2, SIF_NE_VEC3);
	if (f)
		Au_Cone(src, f[0], f[1], f[2]);
	else
		Au_Cone(
			src,
			luaL_checknumber(vm, 2),
			luaL_checknumber(vm, 3),
			luaL_checknumber(vm, 4)
		);
	return 0;
}

SIF_FUNC(MaxDistance)
{
	SIF_CHECKCOMPONENT(1, src, NE_AUDIO_SOURCE, struct NeAudioSource *);
	Au_MaxDistance(src, luaL_checknumber(vm, 2));
	return 0;
}

SIF_FUNC(RefDistance)
{
	SIF_CHECKCOMPONENT(1, src, NE_AUDIO_SOURCE, struct NeAudioSource *);
	Au_RefDistance(src, luaL_checknumber(vm, 2));
	return 0;
}

SIF_FUNC(Clip)
{
	SIF_CHECKCOMPONENT(1, src, NE_AUDIO_SOURCE, struct NeAudioSource *);

	if (!lua_islightuserdata(vm, 2))
		luaL_argerror(vm, 2, "Must be light user data");

	Au_SetClip(src, (NeHandle)lua_touserdata(vm, 2));
	return 0;
}

SIF_FUNC(__tostring)
{
	if (luaL_testudata(vm, 1, NE_AUDIO_SOURCE))
		lua_pushliteral(vm, "NeAudioSource");
	else
		lua_pushliteral(vm, "NeAudioListener");
	return 1;
}

NE_ENGINE_IF_MOD(Audio)
{
	luaL_Reg reg[] =
	{
		SIF_REG(DistanceModel),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Audio");

	luaL_Reg lMeta[] = {
		{ "__index",     NULL },
		SIF_REG(__tostring),
		SIF_ENDREG()
	};

	luaL_Reg lMeth[] = {
		{ "Position", Sif_ListenerPosition },
		{ "Velocity", Sif_ListenerVelocity },
		{ "Orientation", Sif_ListenerOrientation },
		SIF_ENDREG()
	};

	luaL_newmetatable(vm, NE_AUDIO_LISTENER);
	luaL_setfuncs(vm, lMeta, 0);
	luaL_newlibtable(vm, lMeth);
	luaL_setfuncs(vm, lMeth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);

	luaL_Reg sMeta[] = {
		{ "__index",     NULL },
		SIF_REG(__tostring),
		SIF_ENDREG()
	};

	luaL_Reg sMeth[] = {
		SIF_REG(Playing),
		SIF_REG(Play),
		SIF_REG(Pause),
		SIF_REG(Stop),
		SIF_REG(Rewind),
		SIF_REG(Position),
		SIF_REG(Velocity),
		SIF_REG(Direction),
		SIF_REG(Gain),
		SIF_REG(Pitch),
		SIF_REG(Cone),
		SIF_REG(Clip),
		SIF_REG(MaxDistance),
		SIF_REG(RefDistance),
		SIF_ENDREG()
	};

	luaL_newmetatable(vm, NE_AUDIO_SOURCE);
	luaL_setfuncs(vm, sMeta, 0);
	luaL_newlibtable(vm, sMeth);
	luaL_setfuncs(vm, sMeth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);

	lua_newtable(vm);
	{
		lua_pushinteger(vm, DM_InverseDistance);
		lua_setfield(vm, -2, "Inverse");

		lua_pushinteger(vm, DM_InverseDistanceClamped);
		lua_setfield(vm, -2, "InverseClamped");

		lua_pushinteger(vm, DM_LinearDistance);
		lua_setfield(vm, -2, "Linear");

		lua_pushinteger(vm, DM_LinearDistanceClamped);
		lua_setfield(vm, -2, "LinearClamped");

		lua_pushinteger(vm, DM_ExponentDistance);
		lua_setfield(vm, -2, "Exponent");

		lua_pushinteger(vm, DM_ExponentDistanceClamped);
		lua_setfield(vm, -2, "ExponentClamped");
	}
	lua_setglobal(vm, "DistanceModel");
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
