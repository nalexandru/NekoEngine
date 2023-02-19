#ifndef _NE_SCRIPT_INTERFACE_H_
#define _NE_SCRIPT_INTERFACE_H_

#include <Script/Script.h>

#define SCRIPT_API_VERSION			4

#define SIF_VOID(name, func)		\
static int							\
_S_ ## name(lua_State *vm)			\
{									\
	func();							\
	return 0;						\
}

#define SIF_OBJ_VOID(name, func)	\
static int							\
_S_ ## name(lua_State *vm)			\
{									\
	if (!lua_islightuserdata(vm, 1)) \
		luaL_argerror(vm, 1, "Must be light user data"); \
	func(lua_touserdata(vm, 1));	\
	return 0;						\
}

#define SIF_BOOL(name, val)			\
static int							\
_S_ ## name(lua_State *vm)			\
{									\
	lua_pushboolean(vm, val);		\
	return 1;						\
}

#define SIF_STRING(name, val)		\
static int							\
_S_ ## name(lua_State *vm)			\
{									\
	lua_pushstring(vm, val);		\
	return 1;						\
}

#define SIF_NUMBER(name, val)		\
static int							\
_S_ ## name(lua_State *vm)			\
{									\
	lua_pushnumber(vm, val);		\
	return 1;						\
}

#define SIF_INTEGER(name, val)		\
static int							\
_S_ ## name(lua_State *vm)			\
{									\
	lua_pushinteger(vm, val);		\
	return 1;						\
}

#define SIF_LUSRDATA(name, val)		\
static int							\
_S_ ## name(lua_State *vm)			\
{									\
	lua_pushlightuserdata(vm, val);	\
	return 1;						\
}

#define SIF_FUNC(name)				\
static int							\
_S_ ## name(lua_State *vm)			\

#define SIF_REG(name) { #name, _S_ ## name }
#define SIF_ENDREG()  { NULL, NULL }

static inline int
_sif_getfield(lua_State *vm, int arg, const char *name)
{
	lua_pushstring(vm, name);
	lua_rawget(vm, arg);
	return lua_gettop(vm);
}
#define SIF_GETFIELD(arg, name) _sif_getfield(vm, arg, name)
#define SIF_POPFIELD(field) lua_remove(vm, field)

static inline int
_sif_intfield(lua_State *vm, int arg, const char *name)
{
	const int f = SIF_GETFIELD(arg, name);
	const int r = (int)luaL_checkinteger(vm, f);
	SIF_POPFIELD(f);
	return r;
}
#define SIF_INTFIELD(arg, name) _sif_intfield(vm, arg, name)

static inline float
_sif_floatfield(lua_State *vm, int arg, const char *name)
{
	const int f = SIF_GETFIELD(arg, name);
	const float r = (float)luaL_checknumber(vm, f);
	SIF_POPFIELD(f);
	return r;
}
#define SIF_FLOATFIELD(arg, name) _sif_floatfield(vm, arg, name)

static inline const char *
_sif_stringfield(lua_State *vm, int arg, const char *name)
{
	const int f = SIF_GETFIELD(arg, name);
	const char *r = luaL_checkstring(vm, f);
	SIF_POPFIELD(f);
	return r;
}
#define SIF_STRINGFIELD(arg, name) _sif_stringfield(vm, arg, name)

static inline void *
_sif_lusrdatafield(lua_State *vm, int arg, const char *name)
{
	const int f = SIF_GETFIELD(arg, name);
	if (!lua_islightuserdata(vm, f))
		luaL_argerror(vm, f, "");
	void *r = lua_touserdata(vm, f);
	SIF_POPFIELD(f);
	return r;
}
#define SIF_LUSRDATAFIELD(arg, name) _sif_lusrdatafield(vm, arg, name)

static inline bool
_sif_boolfield(lua_State *vm, int arg, const char *name)
{
	const int f = SIF_GETFIELD(arg, name);
	if (!lua_isboolean(vm, f))
		luaL_argerror(vm, f, "");
	const bool r = lua_toboolean(vm, f);
	SIF_POPFIELD(f);
	return r;
}
#define SIF_BOOLFIELD(arg, name) _sif_boolfield(vm, arg, name)

#ifdef __cplusplus
extern "C" {
#endif

void SIface_OpenEngine(lua_State *vm);
void SIface_OpenSystem(lua_State *vm);
void SIface_OpenRender(lua_State *vm);
void SIface_OpenConsole(lua_State *vm);
void SIface_OpenUI(lua_State *vm);
void SIface_OpenScriptComponent(lua_State *vm);
void SIface_OpenResource(lua_State *vm);
void SIface_OpenIO(lua_State *vm);
void SIface_OpenEvent(lua_State *vm);
void SIface_OpenInput(lua_State *vm);
void SIface_OpenTransform(lua_State *vm);
void SIface_OpenCamera(lua_State *vm);
void SIface_OpenLight(lua_State *vm);
void SIface_OpenConfig(lua_State *vm);
void SIface_OpenVec3(lua_State *vm);
void SIface_OpenVec4(lua_State *vm);
void SIface_OpenMatrix(lua_State *vm);
void SIface_OpenAudio(lua_State *vm);

#ifdef __cplusplus
}
#endif

#endif /* _NE_SCRIPT_INTERFACE_H_ */

/* NekoEngine
 *
 * Interface.h
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
