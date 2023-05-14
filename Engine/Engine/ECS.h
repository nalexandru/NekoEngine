#ifndef _NE_ENGINE_ECS_H_
#define _NE_ENGINE_ECS_H_

#include <Runtime/Array.h>

#include <Engine/Types.h>
#include <Engine/Entity.h>
#include <Engine/Component.h>
#include <Script/Script.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*NeScriptCompInitProc)(void *comp, const void **args, const char *type, const char *script);
typedef void (*NeScriptCompTermProc)(void *comp, const char *type, const char *script);

struct NeEntityMessage
{
	NeEntityHandle dst;
	uint32_t msg;
	const void *data;
};

struct NeEntity
{
	size_t id;
	uint32_t sceneId;
	uint32_t compCount;
	struct NeEntityComp comp[MAX_ENTITY_COMPONENTS];
	struct NeQueue mbox;
	uint64_t hash;
	char name[MAX_ENTITY_NAME];
};

struct NeEntityType
{
	uint64_t hash;
	uint32_t compCount;
	NeCompTypeId compTypes[MAX_ENTITY_COMPONENTS];
	struct NeArray initialArguments[MAX_ENTITY_COMPONENTS];
};

struct NeCompType
{
	uint64_t hash;
	size_t size, alignment;
	bool scriptMessageHandler;
	NeCompMessageHandlerProc messageHandler;
	union {
		NeCompInitProc init;
		NeScriptCompInitProc initScript;
	};
	union {
		NeCompTermProc term;
		NeScriptCompTermProc termScript;
	};
	char *script, name[MAX_ENTITY_NAME];
};

struct NeECSystem
{
	NeECSysExecProc exec;
	union {
		lua_State *vm;
		lua_State **vms;
	};
	uint64_t nameHash;
	uint64_t groupHash;
	size_t typeCount;
	bool singleThread, enabled;
	int32_t priority;
	NeCompTypeId compTypes[MAX_ENTITY_COMPONENTS];
	uint64_t scriptHash;
	char *reload;
};

typedef bool (*NeCompSysRegisterAllProc)(void);

extern struct NeQueue *ECS_mboxes;

const struct NeCompType *ECS_ComponentType(NeCompTypeId typeId);
void *ECS_CommitedComponentPtr(struct NeScene *s, NeCompHandle handle);
void *ECS_ComponentPtr(struct NeScene *s, NeCompHandle handle);
void *ECS_GetComponent(struct NeScene *s, NeEntityHandle handle, NeCompTypeId type);

void E_DistributeMessages(void);
void E_ProcessMessages(struct NeScene *s);

bool E_InitComponents(void);
void E_TermComponents(void);

bool E_InitEntities(void);
void E_TermEntities(void);

bool E_InitSceneComponents(struct NeScene *s);
void E_TermSceneComponents(struct NeScene *s);

bool E_InitSceneEntities(struct NeScene *s);
void E_TermSceneEntities(struct NeScene *s);

bool E_InitECSystems(void);
void E_ReloadSystemScripts(void);
void E_TermECSystems(void);

#ifdef __cplusplus
}
#endif

#endif /* _NE_ENGINE_ECS_H_ */

/* NekoEngine
 *
 * ECS.h
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
