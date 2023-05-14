#include <string.h>

#include <Scene/Scene.h>
#include <System/Memory.h>
#include <Engine/Config.h>
#include <Engine/Component.h>
#include <Script/Interface.h>

#include "../Engine/ECS.h"

static inline void
PushBuffer(lua_State *vm, struct ScBuffer *buff)
{
	struct ScBuffer *d = lua_newuserdatauv(vm, sizeof(*d), 0);
	luaL_setmetatable(vm, SIF_NE_BUFFER);
	memcpy(d, buff, sizeof(*d));
}

static inline void
PushTexture(lua_State *vm, struct ScTexture *tex)
{
	struct ScTexture *d = lua_newuserdatauv(vm, sizeof(*d), 0);
	luaL_setmetatable(vm, SIF_NE_TEXTURE);
	memcpy(d, tex, sizeof(*d));
}

SIF_FUNC(Index)
{
	SIF_GETCOMPONENT(1, sc, struct NeScriptComponent *);
	const uint64_t hash = Rt_HashString(luaL_checkstring(vm, 2));

	struct NeScriptField *f;
	Rt_ArrayForEach(f, &sc->fields) {
		if (f->hash != hash)
			continue;

		switch (f->type) {
		case SFT_Integer: lua_pushinteger(vm, f->integer); break;
		case SFT_Number: lua_pushnumber(vm, f->number); break;
		case SFT_Boolean: lua_pushboolean(vm, f->boolean); break;
		case SFT_Pointer: lua_pushlightuserdata(vm, f->usrdata); break;
		case SFT_String: lua_pushstring(vm, f->string); break;
		case SFT_Vec2: SIface_PushVec2(vm, f->vec2); break;
		case SFT_Vec3: SIface_PushVec3(vm, f->vec3); break;
		case SFT_Vec4: SIface_PushVec4(vm, f->vec4); break;
		case SFT_Quaternion: SIface_PushQuaternion(vm, f->quat); break;
		case SFT_Matrix: SIface_PushMatrix(vm, f->matrix); break;
		case SFT_Buffer: PushBuffer(vm, f->buffer); break;
		case SFT_Texture: PushTexture(vm, f->texture); break;
		case SFT_Entity: Sc_PushScriptWrapper(vm, f->usrdata, SIF_NE_ENTITY); break;
		}

		return 1;
	}

	luaL_argerror(vm, 2, "Field not found");
	return 0;
}

SIF_FUNC(NewIndex)
{
	SIF_GETCOMPONENT(1, sc, struct NeScriptComponent *);
	const uint64_t hash = Rt_HashString(luaL_checkstring(vm, 2));

	struct NeScriptField *f = NULL;
	Rt_ArrayForEach(f, &sc->fields) {
		if (f->hash == hash)
			break;

		f = NULL;
	}

	if (!f) {
		f = Rt_ArrayAllocate(&sc->fields);
		f->hash = hash;
	}

	if (f->readOnly)
		luaL_argerror(vm, 2, "The field is read only.");

#define CHECK_FIELD_TYPE(field, fieldType)						\
	if (f->type > SFT_Pointer && f->type != fieldType) {		\
		Sys_Free(f->usrdata);									\
		f->field = Sys_Alloc(sizeof(*f->field), 1, MH_Script);	\
		f->type = fieldType;									\
	}

	switch (lua_type(vm, 3)) {
		case LUA_TNIL: {
			f->type = SFT_Pointer;
			f->usrdata = NULL;
		} break;
		case LUA_TBOOLEAN: {
			f->type = SFT_Boolean;
			f->boolean = lua_toboolean(vm, 3);
		} break;
		case LUA_TUSERDATA: {
			union {
				float *p;
				struct ScBuffer *sb;
				struct ScTexture *st;
				NeEntityHandle ent;
			} u;
			if ((u.p = luaL_testudata(vm, 3, SIF_NE_VEC3))) {
				CHECK_FIELD_TYPE(vec3, SFT_Vec3);
				memcpy(f->vec3, u.p, sizeof(float) * 3);
			} else if ((u.p = luaL_testudata(vm, 3, SIF_NE_VEC4))) {
				CHECK_FIELD_TYPE(vec4, SFT_Vec4);
				memcpy(f->vec4, u.p, sizeof(*f->vec4));
			} else if ((u.p = luaL_testudata(vm, 3, SIF_NE_QUATERNION))) {
				CHECK_FIELD_TYPE(quat, SFT_Quaternion);
				memcpy(f->quat, u.p, sizeof(*f->quat));
			} else if ((u.p = luaL_testudata(vm, 3, SIF_NE_MATRIX))) {
				CHECK_FIELD_TYPE(matrix, SFT_Matrix);
				memcpy(f->matrix, u.p, sizeof(*f->matrix));
			} else if ((u.sb = luaL_testudata(vm, 3, SIF_NE_BUFFER))) {
				CHECK_FIELD_TYPE(buffer, SFT_Buffer);
				memcpy(f->buffer, u.sb, sizeof(*f->buffer));
			} else if ((u.st = luaL_testudata(vm, 3, SIF_NE_TEXTURE))) {
				CHECK_FIELD_TYPE(texture, SFT_Texture);
				memcpy(f->texture, u.st, sizeof(*f->texture));
			} else if ((u.p = luaL_testudata(vm, 3, SIF_NE_VEC2))) {
				CHECK_FIELD_TYPE(vec2, SFT_Vec2);
				memcpy(f->vec2, u.p, sizeof(float) * 2);
			} else if ((u.ent = luaL_testudata(vm, 3, SIF_NE_ENTITY))) {
				f->type = SFT_Entity;
				f->usrdata = lua_touserdata(vm, 3);
			} else {
				f->type = SFT_Pointer;
				f->usrdata = lua_touserdata(vm, 3);
			}
		} break;
		case LUA_TLIGHTUSERDATA: {
			f->type = SFT_Pointer;
			f->usrdata = lua_touserdata(vm, 3);
		} break;
		case LUA_TNUMBER: {
			if (lua_isinteger(vm, 3)) {
				f->type = SFT_Integer;
				f->integer = lua_tointeger(vm, 3);
			} else {
				f->type = SFT_Number;
				f->number = lua_tonumber(vm, 3);
			}
		} break;
		case LUA_TSTRING: {
			size_t len;
			const char *str = luaL_checklstring(vm, 3, &len); ++len;

			if (f->type > SFT_Pointer)
				Sys_Free(f->usrdata);

			f->type = SFT_String;
			f->string = Sys_Alloc(sizeof(*f->string), len, MH_Script);
			strlcpy(f->string, str, len);
		} break;
		case LUA_TTABLE:
		case LUA_TFUNCTION:
		case LUA_TTHREAD: {
			luaL_argerror(vm, 3, "Type not supported");
		} break;
	}

#undef CHECK_FIELD_TYPE

	return 0;
}

SIF_FUNC(ToString)
{
	void *p = lua_touserdata(vm, 1);
	if (!p || !lua_getmetatable(vm, 1))
		return 0;

	int mt = lua_gettop(vm);

	lua_pushstring(vm, "__name");
	lua_rawget(vm, mt);

	int f = lua_gettop(vm);
	char *str = lua_isstring(vm, f) ? Rt_TransientStrDup(lua_tostring(vm, f)) : "ScriptComponent";
	lua_remove(vm, f);

	lua_pop(vm, 1);

	lua_pushstring(vm, str);
	return 1;
}

int
SIface_ScriptComponent(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		SIF_REG(Index),
		SIF_REG(NewIndex),
		SIF_REG(ToString),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Component");

	return 1;
}

/* NekoEngine
 *
 * l_ScriptComponent.c
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
