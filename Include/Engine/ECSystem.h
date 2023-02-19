#ifndef _NE_ENGINE_ECSYSTEM_H_
#define _NE_ENGINE_ECSYSTEM_H_

#include <Engine/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define	ECSYS_GROUP_MANUAL			"manual"			// Executed only explicitly
#define	ECSYS_GROUP_LOGIC			"grp_logic"			// Main game logic
#define ECSYS_GROUP_POST_LOGIC		"grp_post_logic"	// After logic, before the image is acquired
#define	ECSYS_GROUP_PRE_RENDER		"grp_pre_render"	// After the image is acquired, before the render graph
#define ECSYS_GROUP_POST_RENDER		"grp_post_render"	// After the render graph

#define ECSYS_PRI_TRANSFORM	-30000
#define	ECSYS_PRI_CULLING	-25000
#define	ECSYS_PRI_CAM_VIEW	-20000
#define ECSYS_PRI_SKINNING	-1000
#define ECSYS_PRI_MORPH		-900
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

#ifdef __cplusplus
}
#endif

#endif /* _NE_ENGINE_ECSYSTEM_H_ */

/* NekoEngine
 *
 * ECSystem.h
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
