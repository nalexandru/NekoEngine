#ifndef _NE_SCRIPT_INTERFACE_H_
#define _NE_SCRIPT_INTERFACE_H_

#include <Script/Script.h>

#define SCRIPT_API_VERSION			2

#define SIF_VOID(name, func)		\
static int							\
_S_ ## name(lua_State *vm)			\
{									\
	func();							\
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

#endif /* _NE_SCRIPT_INTERFACE_H_ */
