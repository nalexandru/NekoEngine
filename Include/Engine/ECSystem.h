#ifndef _NE_ENGINE_ECSYSTEM_H_
#define _NE_ENGINE_ECSYSTEM_H_

#include <Engine/Types.h>

#define	ECSYS_GROUP_MANUAL		L"manual"			// Executed only explicitly
#define	ECSYS_GROUP_PRE_LOGIC	L"grp_pre_logic"	// Executed 
#define	ECSYS_GROUP_LOGIC		L"grp_logic"		//
#define ECSYS_GROUP_POST_LOGIC	L"grp_post_logic"	//
#define ECSYS_GROUP_FIXED_LOGIC	L"grp_fixed_logic"	//
#define	ECSYS_GROUP_PRE_RENDER	L"grp_pre_render"	//
#define	ECSYS_GROUP_RENDER		L"grp_render"		//
#define ECSYS_GROUP_POST_RENDER	L"grp_post_render"	//

#define ECSYS_PRI_TRANSFORM	-30000
#define	ECSYS_PRI_CULLING	-25000
#define	ECSYS_PRI_CAM_VIEW	-20000
#define ECSYS_PRI_DRAW		0

ENGINE_API extern struct Scene *Scn_activeScene;

bool E_RegisterSystem(const wchar_t *name, const wchar_t *group, const wchar_t **comp, size_t num_comp, ECSysExecProc proc, int32_t priority);
bool E_RegisterSystemId(const wchar_t *name, const wchar_t *group, const CompTypeId *comp, size_t num_comp, ECSysExecProc proc, int32_t priority);

void E_ExecuteSystemS(struct Scene *s, const wchar_t *name, void *args);
static inline void E_ExecuteSystem(const wchar_t *name, void *args) { E_ExecuteSystemS(Scn_activeScene, name, args); }

void E_ExecuteSystemGroupS(struct Scene *s, const wchar_t *name);
static inline void E_ExecuteSystemGroup(const wchar_t *name) { E_ExecuteSystemGroupS(Scn_activeScene, name); }

#endif /* _NE_ENGINE_ECSYSTEM_H_ */
