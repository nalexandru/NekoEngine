#include <Engine/IO.h>
#include <Engine/Job.h>
#include <Engine/Types.h>
#include <Engine/Entity.h>
#include <Engine/Events.h>
#include <Engine/Component.h>
#include <Scene/Scene.h>
#include <System/Log.h>
#include <Runtime/Runtime.h>

#include "ECS.h"

#define ENT_MOD	"Entity"

struct NeQueue *ECS_mboxes = NULL;

static struct NeArray f_entityTypes;
static struct NeAtomicLock f_entityTypeLock;

static inline bool AddComponent(struct NeScene *s, struct NeEntity *, NeCompTypeId, NeCompHandle);
static inline bool CreateComponent(struct NeScene *s, struct NeEntity *ent, NeCompTypeId type, const void **args);
static void LoadEntity(const char *path);
static void ProcessEntityMessages(int workerId, struct NeEntity *ent);

static inline bool
AddEntity(struct NeScene *s, struct NeEntity *ent, const char *name, bool broadcast)
{
	Rt_InitQueue(&ent->mbox, 10, sizeof(struct NeEntityMessage), MH_Scene);
	E_RenameEntity(ent, name ? name : "unnamed");

	Sys_AtomicLockWrite(&s->lock.newEntity);
	bool rc = Rt_ArrayAddPtr(&s->newEntities, ent);
	Sys_AtomicUnlockWrite(&s->lock.newEntity);

	if (!rc) {
		Sys_Free(ent);
		return false;
	}

	if (broadcast)
		E_Broadcast(EVT_ENTITY_CREATED, ent);

	return true;
}

static inline struct NeEntity *
AllocEntity(struct NeScene *s)
{
	struct NeEntity *ent = Sys_Alloc(1, sizeof(*ent), MH_Scene);
	if (!ent)
		return NULL;

	ent->sceneId = s->id;
	return ent;
}

NeEntityHandle
E_CreateEntityS(struct NeScene *s, const char *name, const char *typeName)
{
	uint64_t hash = 0;
	struct NeEntity *ent = NULL;
	struct NeEntityType *type = NULL;

	if (typeName) {
		Sys_AtomicLockRead(&f_entityTypeLock);

		hash = Rt_HashString(typeName);
		type = Rt_ArrayFind(&f_entityTypes, &hash, Rt_U64CmpFunc);
		if (!type) {
			Sys_LogEntry(ENT_MOD, LOG_CRITICAL, "Entity type %s not found");
			return ES_INVALID_ENTITY;
		}

		ent = E_CreateEntityWithArgArrayS(s, name, type->compTypes, type->initialArguments, type->compCount);

		Sys_AtomicUnlockRead(&f_entityTypeLock);
	} else {
		ent = AllocEntity(s);
		if (!ent || !AddEntity(s, ent, name, true))
			return ES_INVALID_ENTITY;
	}

	return ent;
}

NeEntityHandle
E_CreateEntityWithArgsS(struct NeScene *s, const char *name, const NeCompTypeId *compTypes, const void ***compArgs, uint8_t count)
{
	struct NeEntity *ent = NULL;

	if (count > MAX_ENTITY_COMPONENTS)
		return ES_INVALID_ENTITY;

	if (!(ent = AllocEntity(s)))
		return ES_INVALID_ENTITY;

	for (uint8_t i = 0; i < count; ++i) {
		if (!CreateComponent(s, ent, compTypes[i], compArgs ? compArgs[i] : NULL)) {
			Sys_Free(ent);
			return ES_INVALID_ENTITY;
		}
	}

	return AddEntity(s, ent, name, true) ? ent : ES_INVALID_ENTITY;
}

NeEntityHandle
E_CreateEntityWithArgArrayS(struct NeScene *s, const char *name, const NeCompTypeId *compTypes, const struct NeArray *compArgs, uint8_t count)
{
	struct NeEntity *ent = NULL;

	if (count > MAX_ENTITY_COMPONENTS)
		return ES_INVALID_ENTITY;

	if (!(ent = AllocEntity(s)))
		return ES_INVALID_ENTITY;

	for (uint8_t i = 0; i < count; ++i) {
		if (!CreateComponent(s, ent, compTypes[i], compArgs ? (const void **)compArgs[i].data : NULL)) {
			Sys_Free(ent);
			return ES_INVALID_ENTITY;
		}
	}

	return AddEntity(s, ent, name, true) ? ent : ES_INVALID_ENTITY;
}

NeEntityHandle
E_CreateEntityVS(struct NeScene *s, const char *name, int count, const struct NeEntityCompInfo *info)
{
	struct NeEntity *ent = NULL;

	if (count > MAX_ENTITY_COMPONENTS)
		return ES_INVALID_ENTITY;

	if (!(ent = AllocEntity(s)))
		return ES_INVALID_ENTITY;

	for (int i = 0; i < count; ++i) {
		if (!CreateComponent(s, ent, E_ComponentTypeId(info[i].type), info[i].args)) {
			Sys_Free(ent);
			return ES_INVALID_ENTITY;
		}
	}

	return AddEntity(s, ent, name, true) ? ent : ES_INVALID_ENTITY;
}

NeEntityHandle
E_CreateEntityWithComponentsS(struct NeScene *s, const char *name, int count, ...)
{
	va_list va;
	struct NeEntity *ent = NULL;
	NeCompHandle handle;

	if (count > MAX_ENTITY_COMPONENTS)
		return ES_INVALID_ENTITY;

	if (!(ent = AllocEntity(s)))
		return ES_INVALID_ENTITY;

	va_start(va, count);
	for (int i = 0; i < count; ++i) {
		handle = va_arg(va, NeCompHandle);

		if (!AddComponent(s, ent, E_ComponentTypeS(s, handle), handle)) {
			Sys_Free(ent);
			return ES_INVALID_ENTITY;
		}
	}

	if (!AddEntity(s, ent, name, false))
		return ES_INVALID_ENTITY;

	for (uint32_t i = 0; i < ent->compCount; ++i)
		E_SetComponentOwnerS(s, ent->comp[i].handle, ent);

	E_Broadcast(EVT_ENTITY_CREATED, ent);

	return ent;
}

bool
E_AddComponent(NeEntityHandle handle, NeCompTypeId type, NeCompHandle comp)
{
	struct NeEntity *ent = handle;
	return AddComponent(Scn_GetScene(ent->sceneId), ent, type, comp);
}

bool
E_AddNewComponent(NeEntityHandle handle, NeCompTypeId type, const void **args)
{
	struct NeEntity *ent = handle;
	return CreateComponent(Scn_GetScene(ent->sceneId), ent, type, args);
}

void *
E_GetComponent(NeEntityHandle handle, NeCompTypeId type)
{
	struct NeEntity *ent = handle;
	struct NeScene *scn = Scn_GetScene(ent->sceneId);

	for (uint8_t i = 0; i < ent->compCount; ++i)
		if (ent->comp[i].type == type)
			return E_ComponentPtrS(scn, ent->comp[i].handle);

	return NULL;
}

NeCompHandle
E_GetComponentHandle(NeEntityHandle handle, NeCompTypeId type)
{
	struct NeEntity *ent = handle;

	for (uint8_t i = 0; i < ent->compCount; ++i)
		if (ent->comp[i].type == type)
			return ent->comp[i].handle;

	return NE_INVALID_HANDLE;
}

void
E_GetComponents(NeEntityHandle handle, struct NeArray *comp)
{
	struct NeEntity *ent = handle;
	Rt_InitArray(comp, ent->compCount, sizeof(struct NeEntityComp), MH_Frame);
	memcpy(comp->data, ent->comp, Rt_ArrayByteSize(comp));
	comp->count = comp->size;
}

void
E_RemoveComponent(NeEntityHandle handle, NeCompTypeId type)
{
	uint32_t id;
	struct NeEntity *ent = handle;
	struct NeScene *scn = Scn_GetScene(ent->sceneId);

	for (id = 0; id < ent->compCount; ++id)
		if (ent->comp[id].type == type)
			break;

	E_DestroyComponentS(scn, ent->comp[id].handle);
	--ent->compCount;

	if (id == ent->compCount)
		return;

	memcpy(&ent->comp[id], &ent->compCount, sizeof(struct NeEntityComp));
}

void
E_DestroyEntity(NeEntityHandle handle)
{
	struct NeEntity *ent = handle;
	struct NeScene *scn = Scn_GetScene(ent->sceneId);

	for (uint8_t i = 0; i < ent->compCount; ++i)
		E_DestroyComponentS(scn, ent->comp[i].handle);

	Rt_TermQueue(&ent->mbox);

	const size_t dst_id = ent->id;
	Sys_Free(ent);

	Sys_AtomicLockWrite(&scn->lock.entity);

	memcpy(Rt_ArrayGet(&scn->entities, dst_id), Rt_ArrayLast(&scn->entities), scn->entities.elemSize);

	ent = Rt_ArrayGetPtr(&scn->entities, dst_id);
	ent->id = dst_id;
	--scn->entities.count;

	Sys_AtomicUnlockWrite(&scn->lock.entity);

	E_Broadcast(EVT_ENTITY_DESTROYED, handle);
}

bool
E_RegisterEntityType(const char *typeName, const NeCompTypeId *compTypes, uint8_t typeCount)
{
	struct NeEntityType type;

	if (typeCount > MAX_ENTITY_COMPONENTS)
		return false;

	type.hash = Rt_HashString(typeName);
	type.compCount = typeCount;
	memcpy(type.compTypes, compTypes, sizeof(NeCompTypeId) * typeCount);

	Sys_AtomicLockWrite(&f_entityTypeLock);
	bool rc = Rt_ArrayAdd(&f_entityTypes, &type);
	Sys_AtomicUnlockWrite(&f_entityTypeLock);

	return rc;
}

void *
E_EntityPtr(NeEntityHandle handle)
{
	struct NeEntity *ent = handle;
	struct NeScene *scn = Scn_GetScene(ent->sceneId);

	Sys_AtomicLockRead(&scn->lock.entity);
	void *ptr = Rt_ArrayGetPtr(&scn->entities, ent->id);
	Sys_AtomicUnlockRead(&scn->lock.entity);

	return ptr;
}

uint32_t
E_EntityCountS(struct NeScene *s)
{
	Sys_AtomicLockRead(&s->lock.entity);
	int count = (uint32_t)s->entities.count;
	Sys_AtomicUnlockRead(&s->lock.entity);

	return count;
}

const char *
E_EntityName(NeEntityHandle handle)
{
	struct NeEntity *ent = handle;
	return ent->name;
}

void
E_RenameEntity(NeEntityHandle handle, const char *name)
{
	struct NeEntity *ent = handle;
	strlcpy(ent->name, name, MAX_ENTITY_NAME);
	ent->hash = Rt_HashString(ent->name);
}

NeEntityHandle
E_FindEntityS(struct NeScene *s, const char *name)
{
	struct NeEntity *ent = NULL;
	uint64_t hash = Rt_HashString(name);

	Sys_AtomicLockRead(&s->lock.entity);
	Rt_ArrayForEachPtr(ent, &s->entities) {
		if (ent->hash == hash)
			break;
		ent = NULL;
	}
	Sys_AtomicUnlockRead(&s->lock.entity);

	return ent;
}

void
E_SendMessage(NeEntityHandle dst, uint32_t msg, const void *data)
{
	struct NeEntityMessage em = { dst, msg, data };
	Rt_QueuePush(&ECS_mboxes[E_WorkerId()], &em);
}

bool
E_InitEntities(void)
{
	memset(&f_entityTypes, 0x0, sizeof(f_entityTypes));

	if (!Rt_InitArray(&f_entityTypes, 10, sizeof(struct NeEntityType), MH_System))
		return false;

	ECS_mboxes = Sys_Alloc(sizeof(*ECS_mboxes), E_JobWorkerThreads() + 1, MH_System);
	for (uint32_t i = 0; i < E_JobWorkerThreads() + 1; ++i)
		if (!Rt_InitQueue(&ECS_mboxes[i], 10, sizeof(struct NeEntityMessage), MH_System))
			return false;

	E_ProcessFiles("/Entities", "ent", true, LoadEntity);

	return true;
}

void
E_DistributeMessages(void)
{
	for (uint32_t i = 0; i < E_JobWorkerThreads() + 1; ++i) {
		struct NeQueue *mbox = &ECS_mboxes[i];

		while (mbox->count) {
			struct NeEntityMessage msg = *(struct NeEntityMessage *)Rt_QueuePop(mbox);
			struct NeEntity *ent = msg.dst;
			Rt_QueuePush(&ent->mbox, &msg);
		}
	}
}

void
E_ProcessMessages(struct NeScene *s)
{
	Sys_AtomicLockRead(&s->lock.entity);
	E_DispatchJobs(s->entities.count, (NeJobProc)ProcessEntityMessages, (void **)s->entities.data, NULL, NULL);

	struct NeEntity *ent = NULL;
	Rt_ArrayForEachPtr(ent, &s->entities)
		while (ent->mbox.count)
			Sys_Yield();

	Sys_AtomicUnlockRead(&s->lock.entity);
}

void
ProcessEntityMessages(int workerId, struct NeEntity *ent)
{
	int top = 0;
	lua_State *vm = NULL;
	uint64_t currentScript = 0;

	// TODO: The lua_state handling is messy. It should be moved to Entity and initialized when a script component
	// is attached, that way state will be kept.

	while (ent->mbox.count) {
		struct NeEntityMessage msg = *(struct NeEntityMessage *)Rt_QueuePop(&ent->mbox);
		for (uint32_t j = 0; j < ent->compCount; ++j) {
			const struct NeCompType *type = ECS_ComponentType(ent->comp[j].type);
			if (type->messageHandler) {
				type->messageHandler(E_ComponentPtr(ent->comp[j].handle), msg.msg, msg.data);
			} else if (type->scriptMessageHandler) {
				if (!vm) {
					vm = Sc_CreateVM();
					top = lua_gettop(vm);
				}

				const uint64_t hash = Rt_HashString(type->script);
				if (currentScript != hash) {
					lua_settop(vm, top);
					Sc_LoadScriptFile(vm, type->script);
				}

				lua_getglobal(vm, "MessageHandler");
				lua_pushlightuserdata(vm, E_ComponentPtr(ent->comp[j].handle));
				lua_pushinteger(vm, msg.msg);
				lua_pushlightuserdata(vm, (void *)msg.data);

				if (lua_pcall(vm, 3, 0, 0)) {
					Sys_LogEntry(ENT_MOD, LOG_CRITICAL, "Failed to execute MessageHandler for %s: %s", type->name, lua_tostring(vm, -1));
					Sc_LogStackDump(vm, LOG_CRITICAL);
				}
			}
		}
	}

	if (vm)
		Sc_DestroyVM(vm);
}

void
E_TermEntities(void)
{
	Sys_AtomicLockWrite(&f_entityTypeLock);

	for (uint32_t i = 0; i < E_JobWorkerThreads() + 1; ++i)
		Rt_TermQueue(&ECS_mboxes[i]);
	Sys_Free(ECS_mboxes);

	for (size_t i = 0; i < f_entityTypes.count; ++i) {
		struct NeEntityType *type = Rt_ArrayGet(&f_entityTypes, i);
		for (uint32_t j = 0; j < type->compCount; ++j) {
			if (!type->initialArguments[j].data)
				continue;

			for (size_t k = 0; k < type->initialArguments[j].count; ++k)
				Sys_Free(Rt_ArrayGetPtr(&type->initialArguments[j], k));

			Rt_TermArray(&type->initialArguments[j]);
		}
	}
	Rt_TermArray(&f_entityTypes);

	Sys_AtomicUnlockWrite(&f_entityTypeLock);
}

bool
E_InitSceneEntities(struct NeScene *s)
{
	return Rt_InitPtrArray(&s->entities, 100, MH_Scene) && Rt_InitPtrArray(&s->newEntities, 100, MH_Scene);
}

void
E_TermSceneEntities(struct NeScene *s)
{
	struct NeEntity* ent;
	Rt_ArrayForEachPtr(ent, &s->entities) {
		Rt_TermQueue(&ent->mbox);
		Sys_Free(ent);
	}
	Rt_TermArray(&s->entities);

	for (size_t i = 0; i < s->newEntities.count; ++i)
		Sys_Free(Rt_ArrayGetPtr(&s->newEntities, i));
	Rt_TermArray(&s->newEntities);
}

void *
ECS_GetComponent(struct NeScene *s, NeEntityHandle handle, NeCompTypeId type)
{
	struct NeEntity *ent = handle;
	struct NeArray *compData = (struct NeArray *)s->compData.data;
	for (uint8_t i = 0; i < ent->compCount; ++i)
		if (ent->comp[i].type == type)
			return Rt_ArrayGet(&compData[E_HANDLE_TYPE(ent->comp[i].handle)], E_HANDLE_ID(ent->comp[i].handle));
	return NULL;
}

bool
AddComponent(struct NeScene *s, struct NeEntity *ent, NeCompTypeId type, NeCompHandle handle)
{
	struct NeEntityComp *comp = NULL;

	for (uint32_t i = 0; i < ent->compCount; ++i)
		if (ent->comp[i].type == type)
			return false;

	comp = &ent->comp[ent->compCount++];
	comp->type = type;
	comp->handle = handle;
	E_SetComponentOwnerS(s, handle, ent);

	return true;
}

bool
CreateComponent(struct NeScene *s, struct NeEntity *ent, NeCompTypeId type, const void **args)
{
	NeCompHandle handle = E_CreateComponentIdS(s, type, ent, args);

	if (handle == NE_INVALID_HANDLE)
		return false;

	if (!AddComponent(s, ent, type, handle)) {
		Sys_LogEntry(ENT_MOD, LOG_CRITICAL, "Failed to add component of type [%d]", type);
		return false;
	}

	return true;
}

static void
LoadEntity(const char *path)
{
	char buff[512];
	struct NeStream stm = { 0 };
	struct NeEntityType type = { 0 };
	struct NeArray *args = NULL;

	if (!E_FileStream(path, IO_READ, &stm))
		return;

	E_ReadStreamLine(&stm, buff, sizeof(buff));
	if (strnlen(buff, sizeof(buff)) <= 10 || strncmp(buff, "EntityDef=", 10))
		goto exit;

	type.hash = Rt_HashString(&buff[10]);

	while (!E_EndOfStream(&stm)) {
		char *line = E_ReadStreamLine(&stm, buff, sizeof(buff));
		size_t len;

		if (!*(line = Rt_SkipWhitespace(line)) || line[0] == '#')
			continue;

		len = strnlen(line, sizeof(buff));

		if (!strncmp(line, "Component=", 10)) {
			char *typeName = strchr(line, '=') + 1;
			if (!typeName)
				continue;

			if (type.compCount == MAX_ENTITY_COMPONENTS)
				break;

			args = &type.initialArguments[type.compCount];
			type.compTypes[type.compCount++] = E_ComponentTypeId(typeName);
		} else if (!strncmp(line, "EndDefinition", len)) {
			break;
		} else if (!strncmp(line, "EndComponent", len)) {
			if (args && args->data) {
				void *guard = NULL;
				Rt_ArrayAddPtr(args, guard);
			}
			args = NULL;
		} else {
			char *arg = line;
			char *val = strchr(line, '=');
			if (!args || !val)
				continue;

			*val++ = 0x0;

			if (!args->data)
				Rt_InitPtrArray(args, 2, MH_Asset);

			Rt_ArrayAddPtr(args, Rt_StrNDup(arg, sizeof(buff), MH_Asset));
			Rt_ArrayAddPtr(args, Rt_StrNDup(val, sizeof(buff), MH_Asset));
		}
	}

	Sys_AtomicLockWrite(&f_entityTypeLock);
	Rt_ArrayAdd(&f_entityTypes, &type);
	Sys_AtomicUnlockWrite(&f_entityTypeLock);

exit:
	E_CloseStream(&stm);
}

/* NekoEngine
 *
 * Entity.c
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
