#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <Script/Script.h>

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

void SIface_OpenEngine(lua_State *vm);
void SIface_OpenSystem(lua_State *vm);
void SIface_OpenRender(lua_State *vm);

#endif /* _INTERFACE_H_ */
