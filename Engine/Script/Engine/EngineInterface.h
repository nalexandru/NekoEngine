#ifndef NE_SCRIPT_ENGINE_INTERFACE_MODULE_H
#define NE_SCRIPT_ENGINE_INTERFACE_MODULE_H

#include <Script/Script.h>
#include <System/System.h>

#ifdef __cplusplus
extern "C" {
#endif

extern struct NeArray ScEngineInterface_modules;

#ifdef __cplusplus
}
#endif

#define NE_ENGINE_IF_MOD(name)														\
	static void NeScEngineModule_ ## name(lua_State *vm);								\
	NE_INITIALIZER(NeScEngineModuleRegister_ ## name) {									\
		if (!ScEngineInterface_modules.data)											\
			Rt_InitPtrArray(&ScEngineInterface_modules, 2, MH_System);					\
		Rt_ArrayAddPtr(&ScEngineInterface_modules, (void *)NeScEngineModule_ ## name);	\
	}																					\
	static void NeScEngineModule_ ## name(lua_State *vm)

#endif /* NE_SCRIPT_ENGINE_INTERFACE_MODULE_H */