#ifndef _NE_ENGINE_ECSYSTEM_H_
#define _NE_ENGINE_ECSYSTEM_H_

#include <Engine/Types.h>

#define	ECSYS_GROUP_MANUAL			"manual"			// Executed only explicitly
#define	ECSYS_GROUP_PRE_LOGIC		"grp_pre_logic"		// Executed 
#define	ECSYS_GROUP_LOGIC			"grp_logic"			//
#define ECSYS_GROUP_POST_LOGIC		"grp_post_logic"	//
#define ECSYS_GROUP_FIXED_LOGIC		"grp_fixed_logic"	//
#define	ECSYS_GROUP_PRE_RENDER		"grp_pre_render"	//
#define	ECSYS_GROUP_RENDER			"grp_render"		//
#define ECSYS_GROUP_POST_RENDER		"grp_post_render"	//

#define ECSYS_PRI_TRANSFORM	-30000
#define	ECSYS_PRI_CULLING	-25000
#define	ECSYS_PRI_CAM_VIEW	-20000
#define ECSYS_PRI_SKINNING	-100
#define ECSYS_PRI_DRAW		0

ENGINE_API extern struct NeScene *Scn_activeScene;

bool E_RegisterSystem(const char *name, const char *group, const char **comp, size_t numComp, NeECSysExecProc proc, int32_t priority, bool singleThread);
bool E_RegisterSystemId(const char *name, const char *group, const NeCompTypeId *comp, size_t numComp, NeECSysExecProc proc, int32_t priority, bool singleThread);

bool E_RegisterScriptSystem(const char *name, const char *group, const char **comp, size_t numComp, const char *script, int32_t priority, bool singleThread);
bool E_RegisterScriptSystemId(const char *name, const char *group, const NeCompTypeId *comp, size_t numComp, const char *script, int32_t priority, bool singleThread);

void E_ExecuteSystemS(struct NeScene *s, const char *name, void *args);
static inline void E_ExecuteSystem(const char *name, void *args) { E_ExecuteSystemS(Scn_activeScene, name, args); }

void E_ExecuteSystemGroupS(struct NeScene *s, const char *name);
static inline void E_ExecuteSystemGroup(const char *name) { E_ExecuteSystemGroupS(Scn_activeScene, name); }

#define E_REGISTER_SYSTEM(name, group, proc, priority, singleThread, compCount, ...)							\
	E_INITIALIZER(_NeSysRegister_ ## name) {																	\
		const char *components[compCount] = { __VA_ARGS__ };													\
		E_RegisterSystem(name, group, components, compCount, (NeECSysExecProc)proc, priority, singleThread);	\
	}

#define E_SYSTEM(name, group, priority, singleThread, argsType, compCount, ...)										\
	static void _ ## name(void **comp, argsType *args);																\
	E_INITIALIZER(_NeSysRegister_ ## name) {																		\
		const char *components[compCount] = { __VA_ARGS__ };														\
		E_RegisterSystem(name, group, components, compCount, (NeECSysExecProc)_ ## name, priority, singleThread);	\
	}																												\
	static void _ ## name(void **comp, argsType *args)

#endif /* _NE_ENGINE_ECSYSTEM_H_ */
