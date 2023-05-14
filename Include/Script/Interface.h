#ifndef NE_SCRIPT_INTERFACE_H
#define NE_SCRIPT_INTERFACE_H

#include <Engine/Types.h>
#include <Script/Script.h>
#include <Runtime/Runtime.h>
#include <Engine/Component.h>
#include <Render/Core.h>

#define SCRIPT_API_VERSION			10

#define SIF_NE_VEC2					"NeVec2"
#define SIF_NE_VEC3					"NeVec3"
#define SIF_NE_VEC4					"NeVec4"
#define SIF_NE_MATRIX				"NeMatrix"
#define SIF_NE_QUATERNION			"NeQuaternion"

#define SIF_NE_BUFFER				"NeBuffer"
#define SIF_NE_TEXTURE				"NeTexture"
#define SIF_NE_PIPELINE				"NePipeline"
#define SIF_NE_SEMAPHORE			"NeSemaphore"
#define SIF_NE_FRAMEBUFFER			"NeFramebuffer"
#define SIF_NE_RENDER_GRAPH			"NeRenderGraph"
#define SIF_NE_COMMAND_BUFFER		"NeCommandBuffer"
#define SIF_NE_RENDER_PASS_DESC		"NeRenderPassDesc"

#define SIF_NE_STREAM				"NeStream"

#define SIF_NE_SCENE				"NeScene"
#define SIF_NE_ENTITY				"NeEntity"
#define NE_SCRIPT_COMPONENT			"NeScriptComponent"

#define SIF_VOID(name, func)		\
static int							\
Sif_ ## name(lua_State *vm)			\
{									\
	func();							\
	return 0;						\
}

#define SIF_STR_VOID(name, func)	\
static int							\
Sif_ ## name(lua_State *vm)			\
{									\
	func(luaL_checkstring(vm, 1));	\
	return 0;						\
}

#define SIF_OBJ_VOID(name, func)	\
static int							\
Sif_ ## name(lua_State *vm)			\
{									\
	if (!lua_islightuserdata(vm, 1)) \
		luaL_argerror(vm, 1, "Must be light user data"); \
	func(lua_touserdata(vm, 1));	\
	return 0;						\
}

#define SIF_BOOL(name, val)			\
static int							\
Sif_ ## name(lua_State *vm)			\
{									\
	lua_pushboolean(vm, val);		\
	return 1;						\
}

#define SIF_STRING(name, val)		\
static int							\
Sif_ ## name(lua_State *vm)			\
{									\
	lua_pushstring(vm, val);		\
	return 1;						\
}

#define SIF_NUMBER(name, val)		\
static int							\
Sif_ ## name(lua_State *vm)			\
{									\
	lua_pushnumber(vm, val);		\
	return 1;						\
}

#define SIF_INTEGER(name, val)		\
static int							\
Sif_ ## name(lua_State *vm)			\
{									\
	lua_pushinteger(vm, val);		\
	return 1;						\
}

#define SIF_LUSRDATA(name, val)		\
static int							\
Sif_ ## name(lua_State *vm)			\
{									\
	lua_pushlightuserdata(vm, val);	\
	return 1;						\
}

#define SIF_FUNC(name)				\
static int							\
Sif_ ## name(lua_State *vm)			\

#define SIF_REG(name) { #name, Sif_ ## name }
#define SIF_ENDREG()  { NULL, NULL }

#define SIF_CHECKCOMPONENT(pos, name, udata, type)														\
struct NeScriptWrapper *name ## Wrapper = (struct NeScriptWrapper *)luaL_checkudata(vm, pos, udata);	\
type name = (type)name ## Wrapper->ptr

#define SIF_TESTCOMPONENT(pos, name, udata, type)														\
struct NeScriptWrapper *name ## Wrapper = (struct NeScriptWrapper *)luaL_testudata(vm, pos, udata);		\
type name = (type)name ## Wrapper ? (type)name ## Wrapper->ptr : NULL;

#define SIF_GETCOMPONENT(pos, name, type)																\
struct NeScriptWrapper *name ## Wrapper = (struct NeScriptWrapper *)lua_touserdata(vm, pos);			\
type name = (type)name ## Wrapper->ptr

struct ScBuffer
{
	NeBufferHandle handle;
	struct NeBuffer *buff;
	uint64_t address;
};

struct ScTexture
{
	struct NeTexture *tex;
	uint32_t location;
	struct NeTextureDesc desc;
};

struct NeScriptWrapper
{
	union {
		struct NeCompBase *comp;
		void *ptr;
	};
};

enum NeScriptFieldType
{
	SFT_Integer,
	SFT_Number,
	SFT_Boolean,
	SFT_Pointer,
	SFT_String,
	SFT_Vec2,
	SFT_Vec3,
	SFT_Vec4,
	SFT_Quaternion,
	SFT_Matrix,
	SFT_Buffer,
	SFT_Texture,
	SFT_Entity
};

struct NeScriptField
{
	uint64_t hash;
	enum NeScriptFieldType type;
	union {
		lua_Integer integer;
		lua_Number number;
		bool boolean;
		void *usrdata;
		char *string;
		struct NeVec2 *vec2;
		struct NeVec3 *vec3;
		struct NeVec4 *vec4;
		struct NeQuaternion *quat;
		struct NeMatrix *matrix;
		struct ScBuffer *buffer;
		struct ScTexture *texture;
	};
	bool readOnly;
};

struct NeScriptComponent
{
	NE_COMPONENT_BASE;

	struct NeArray fields;
};

static inline int
Sif_GetField(lua_State *vm, int arg, const char *name)
{
	lua_pushstring(vm, name);
	lua_rawget(vm, arg);
	return lua_gettop(vm);
}
#define SIF_GETFIELD(arg, name) Sif_GetField(vm, arg, name)
#define SIF_POPFIELD(field) lua_remove(vm, field)

static inline int
Sif_IntField(lua_State *vm, int arg, const char *name)
{
	const int f = SIF_GETFIELD(arg, name);
	const int r = (int)luaL_checkinteger(vm, f);
	SIF_POPFIELD(f);
	return r;
}
#define SIF_INTFIELD(arg, name) Sif_IntField(vm, arg, name)

static inline int
Sif_OptIntField(lua_State *vm, int arg, const char *name, int val)
{
	const int f = SIF_GETFIELD(arg, name);
	const int r = (int)luaL_optinteger(vm, f, val);
	SIF_POPFIELD(f);
	return r;
}
#define SIF_OPTINTFIELD(arg, name, val) Sif_OptIntField(vm, arg, name, val)

static inline uint64_t
Sif_U64Field(lua_State *vm, int arg, const char *name)
{
	const int f = SIF_GETFIELD(arg, name);
	const uint64_t r = (uint64_t)luaL_checkinteger(vm, f);
	SIF_POPFIELD(f);
	return r;
}
#define SIF_U64FIELD(arg, name) Sif_U64Field(vm, arg, name)

static inline uint64_t
Sif_OptU64Field(lua_State *vm, int arg, const char *name, uint64_t val)
{
	const int f = SIF_GETFIELD(arg, name);
	const uint64_t r = (uint64_t)luaL_optinteger(vm, f, val);
	SIF_POPFIELD(f);
	return r;
}
#define SIF_OPTU64FIELD(arg, name, val) Sif_OptU64Field(vm, arg, name, val)

static inline float
Sif_FloatField(lua_State *vm, int arg, const char *name)
{
	const int f = SIF_GETFIELD(arg, name);
	const float r = luaL_checknumber(vm, f);
	SIF_POPFIELD(f);
	return r;
}
#define SIF_FLOATFIELD(arg, name) Sif_FloatField(vm, arg, name)

static inline float
Sif_OptFloatField(lua_State *vm, int arg, const char *name, float val)
{
	const int f = SIF_GETFIELD(arg, name);
	const float r = (float)luaL_optnumber(vm, f, val);
	SIF_POPFIELD(f);
	return r;
}
#define SIF_OPTFLOATFIELD(arg, name, val) Sif_OptFloatField(vm, arg, name, val)

static inline const char *
Sif_StringField(lua_State *vm, int arg, const char *name)
{
	const int f = SIF_GETFIELD(arg, name);
	const char *r = luaL_checkstring(vm, f);
	SIF_POPFIELD(f);
	return r;
}
#define SIF_STRINGFIELD(arg, name) Sif_StringField(vm, arg, name)

static inline const char *
Sif_OptStringField(lua_State *vm, int arg, const char *name, const char *val)
{
	const int f = SIF_GETFIELD(arg, name);
	const char *r = lua_isstring(vm, f) ? luaL_checkstring(vm, f) : val;
	SIF_POPFIELD(f);
	return r;
}
#define SIF_OPTSTRINGFIELD(arg, name, val) Sif_OptStringField(vm, arg, name, val)

static inline void *
Sif_LUsrDataField(lua_State *vm, int arg, const char *name)
{
	const int f = SIF_GETFIELD(arg, name);
	if (!lua_islightuserdata(vm, f) && !lua_isnil(vm, f))
		luaL_argerror(vm, f, "Must be light user data");
	void *r = lua_touserdata(vm, f);
	SIF_POPFIELD(f);
	return r;
}
#define SIF_LUSRDATAFIELD(arg, name) Sif_LUsrDataField(vm, arg, name)

static inline void *
Sif_UsrDataField(lua_State *vm, int arg, const char *name, const char *tname)
{
	const int f = SIF_GETFIELD(arg, name);
	void *r = luaL_checkudata(vm, f, tname);
	SIF_POPFIELD(f);
	return r;
}
#define SIF_USRDATAFIELD(arg, name, tname) Sif_UsrDataField(vm, arg, name, tname)

static inline bool
Sif_BoolField(lua_State *vm, int arg, const char *name)
{
	const int f = SIF_GETFIELD(arg, name);
	if (!lua_isboolean(vm, f))
		luaL_argerror(vm, f, "");
	const bool r = lua_toboolean(vm, f);
	SIF_POPFIELD(f);
	return r;
}
#define SIF_BOOLFIELD(arg, name) Sif_BoolField(vm, arg, name)

static inline bool
Sif_OptBoolField(lua_State *vm, int arg, const char *name, bool val)
{
	const int f = SIF_GETFIELD(arg, name);
	const bool r = lua_isboolean(vm, f) ? lua_toboolean(vm, f) : val;
	SIF_POPFIELD(f);
	return r;
}
#define SIF_OPTBOOLFIELD(arg, name, val) Sif_OptBoolField(vm, arg, name, val)

static inline void
Sc_PushScriptWrapper(lua_State *vm, void *ptr, const char *udata)
{
	struct NeScriptWrapper *sw = (struct NeScriptWrapper *)lua_newuserdatauv(vm, sizeof(*sw), 0);
	luaL_setmetatable(vm, udata);
	sw->comp = (struct NeCompBase *)ptr;
}

#define NE_SCRIPT_INTEFACE(name)																						\
	static int NeScriptInterface_ ## name(lua_State *vm);																\
	NE_INITIALIZER(NeScriptInterfaceRegister_ ## name) {																	\
		Sc_RegisterInterface(#name, NeScriptInterface_ ## name);														\
	}																													\
	static int NeScriptInterface_ ## name(lua_State *vm)

#ifdef __cplusplus
extern "C" {
#endif

int SIface_ScriptComponent(lua_State *vm);

void SIface_PushVec2(lua_State *vm, const struct NeVec2 *v2);
void SIface_PushVec3(lua_State *vm, const struct NeVec3 *v3);
void SIface_PushVec4(lua_State *vm, const struct NeVec4 *v4);
void SIface_PushQuaternion(lua_State *vm, const struct NeQuaternion *q);
void SIface_PushMatrix(lua_State *vm, const struct NeMatrix *m);

#ifdef __cplusplus
}
#endif

#endif /* NE_SCRIPT_INTERFACE_H */

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
