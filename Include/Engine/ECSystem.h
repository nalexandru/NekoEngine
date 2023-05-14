#ifndef NE_ENGINE_ECSYSTEM_H
#define NE_ENGINE_ECSYSTEM_H

#include <Engine/Types.h>
#include <Runtime/Runtime.h>

#ifdef __cplusplus
extern "C" {
#endif

#define	ECSYS_GROUP_MANUAL				"manual"			// Executed only explicitly
#define	ECSYS_GROUP_LOGIC				"grp_logic"			// Main game logic
#define ECSYS_GROUP_POST_LOGIC			"grp_post_logic"	// After logic, before the image is acquired
#define	ECSYS_GROUP_PRE_RENDER			"grp_pre_render"	// After the image is acquired, before the render graph
#define ECSYS_GROUP_POST_RENDER			"grp_post_render"	// After the render graph

#define ECSYS_GROUP_MANUAL_HASH			0x0000000000000000llu
#define	ECSYS_GROUP_LOGIC_HASH			0x1c456b1988af8db6llu
#define ECSYS_GROUP_POST_LOGIC_HASH		0xd7113e29934084b6llu
#define	ECSYS_GROUP_PRE_RENDER_HASH		0x6abd3a4f6a170b48llu
#define ECSYS_GROUP_POST_RENDER_HASH	0xf5c258fb14ad3e94llu

#define ECSYS_PRI_TRANSFORM	-30000
#define	ECSYS_PRI_CULLING	-25000
#define	ECSYS_PRI_CAM_VIEW	-20000
#define ECSYS_PRI_SKINNING	-1000
#define ECSYS_PRI_MORPH		-900
#define ECSYS_PRI_DRAW		0

ENGINE_API extern struct NeScene *Scn_activeScene;

bool E_RegisterSystem(const char *name, uint64_t group, const char **comp, size_t numComp, NeECSysExecProc proc, int32_t priority, bool singleThread);
bool E_RegisterSystemId(const char *name, uint64_t group, const NeCompTypeId *comp, size_t numComp, NeECSysExecProc proc, int32_t priority, bool singleThread);

void E_ExecuteSystemS(struct NeScene *s, uint64_t hash, void *args);
static inline void E_ExecuteSystemByNameS(struct NeScene *s, const char *name, void *args) { E_ExecuteSystemS(s, Rt_HashString(name), args); }
static inline void E_ExecuteSystem(uint64_t hash, void *args) { E_ExecuteSystemS(Scn_activeScene, hash, args); }
static inline void E_ExecuteSystemByName(const char *name, void *args) { E_ExecuteSystemByNameS(Scn_activeScene, name, args); }

void E_ExecuteSystemGroupS(struct NeScene *s, uint64_t hash);
static inline void E_ExecuteSystemGroupByNameS(struct NeScene *s, const char *name) { E_ExecuteSystemGroupS(s, Rt_HashString(name)); }
static inline void E_ExecuteSystemGroup(uint64_t hash) { E_ExecuteSystemGroupS(Scn_activeScene, hash); }
static inline void E_ExecuteSystemGroupByName(const char *name) { E_ExecuteSystemGroupByNameS(Scn_activeScene, name); }

#define NE_REGISTER_SYSTEM(name, group, proc, priority, singleThread, compCount, ...)													\
	NE_INITIALIZER(NeSysRegister_ ## name) {																							\
		const char *components[compCount] = { __VA_ARGS__ };																			\
		E_RegisterSystem(name, Rt_HashLiteral(group), components, compCount, (NeECSysExecProc)proc, priority, singleThread);			\
	}

#define NE_SYSTEM(name, group, priority, singleThread, argsType, compCount, ...)														\
	static void NeSys_ ## name(void **comp, argsType *args);																			\
	NE_INITIALIZER(NeSysRegister_ ## name) {																							\
		const char *components[compCount] = { __VA_ARGS__ };																			\
		E_RegisterSystem(name, Rt_HashLiteral(group), components, compCount, (NeECSysExecProc)NeSys_ ## name, priority, singleThread);	\
	}																																	\
	static void NeSys_ ## name(void **comp, argsType *args)

#ifdef __cplusplus
}
#endif

#endif /* NE_ENGINE_ECSYSTEM_H */

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
