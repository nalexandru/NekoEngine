#include <Engine/Types.h>
#include <Engine/Entity.h>
#include <Engine/Events.h>
#include <Engine/Component.h>
#include <Scene/Scene.h>
#include <System/Log.h>
#include <Runtime/Runtime.h>

#include "ECS.h"

#define ENT_MOD	"Entity"

static struct NeArray _entityTypes;

static int _TypeCmp(const void *, const void *);
static inline bool _AddComponent(struct NeScene *s, struct NeEntity *, NeCompTypeId, NeCompHandle);
static inline bool _CreateComponent(struct NeScene *s, struct NeEntity *ent, NeCompTypeId type, const void **args);

NeEntityHandle
E_CreateEntityS(struct NeScene *s, const char *typeName)
{
	uint64_t hash = 0;
	struct NeEntity *ent = NULL;
	struct NeEntityType *type = NULL;

	if (typeName) {
		hash = Rt_HashString(typeName);
		type = Rt_ArrayFind(&_entityTypes, &hash, _TypeCmp);
		ent = E_CreateEntityWithArgsS(s, type->comp_types, NULL, type->compCount);
	} else {
		ent = Sys_Alloc(1, sizeof(*ent), MH_Scene);

		if (!ent)
			return ES_INVALID_ENTITY;

		E_RenameEntity(ent, "unnamed");
		if (!Rt_ArrayAddPtr(&s->entities, ent)) {
			Sys_Free(ent);
			return ES_INVALID_ENTITY;
		}
	}

	E_Broadcast(EVT_ENTITY_CREATED, ent);

	return ent;
}

NeEntityHandle
E_CreateEntityWithArgsS(struct NeScene *s, const NeCompTypeId *compTypes, const void ***compArgs, uint8_t count)
{
	uint8_t i = 0;
	struct NeEntity *ent = NULL;

	if (count > MAX_ENTITY_COMPONENTS)
		return ES_INVALID_ENTITY;

	ent = Sys_Alloc(1, sizeof(*ent), MH_Scene);
	if (!ent)
		return ES_INVALID_ENTITY;

	for (i = 0; i < count; ++i) {
		if (!_CreateComponent(s, ent, compTypes[i], compArgs ? compArgs[i] : NULL)) {
			Sys_Free(ent);
			return ES_INVALID_ENTITY;
		}
	}

	E_RenameEntity(ent, "unnamed");
	if (!Rt_ArrayAddPtr(&s->entities, ent)) {
		Sys_Free(ent);
		return ES_INVALID_ENTITY;
	}

	E_Broadcast(EVT_ENTITY_CREATED, ent);

	return ent;
}

NeEntityHandle
E_CreateEntityVS(struct NeScene *s, int count, const struct NeEntityCompInfo *info)
{
	uint8_t i = 0;
	struct NeEntity *ent = NULL;

	if (count > MAX_ENTITY_COMPONENTS)
		return ES_INVALID_ENTITY;

	ent = Sys_Alloc(1, sizeof(*ent), MH_Scene);
	if (!ent)
		return ES_INVALID_ENTITY;

	for (i = 0; i < count; ++i) {
		if (!_CreateComponent(s, ent, E_ComponentTypeId(info[i].type), info[i].args)) {
			Sys_Free(ent);
			return ES_INVALID_ENTITY;
		}
	}

	E_RenameEntity(ent, "unnamed");
	if (!Rt_ArrayAddPtr(&s->entities, ent)) {
		Sys_Free(ent);
		return ES_INVALID_ENTITY;
	}

	E_Broadcast(EVT_ENTITY_CREATED, ent);

	return ent;
}

NeEntityHandle
E_CreateEntityWithComponentsS(struct NeScene *s, int count, ...)
{
	va_list va;
	struct NeEntity *ent = NULL;
	NeCompHandle handle;
	uint8_t i = 0;

	if (count > MAX_ENTITY_COMPONENTS)
		return ES_INVALID_ENTITY;

	ent = Sys_Alloc(1, sizeof(*ent), MH_Scene);
	if (!ent)
		return ES_INVALID_ENTITY;

	va_start(va, count);
	for (i = 0; i < count; ++i) {
		handle = va_arg(va, NeCompHandle);

		if (!_AddComponent(s, ent, E_ComponentTypeS(s, handle), handle)) {
			Sys_Free(ent);
			return ES_INVALID_ENTITY;
		}
	}

	E_RenameEntity(ent, "unnamed");
	if (!Rt_ArrayAddPtr(&s->entities, ent)) {
		Sys_Free(ent);
		return ES_INVALID_ENTITY;
	}

	for (i = 0; i < ent->compCount; ++i)
		E_SetComponentOwnerS(s, ent->comp[i].handle, ent);

	E_Broadcast(EVT_ENTITY_CREATED, ent);

	return ent;
}

bool
E_AddComponentS(struct NeScene *s, NeEntityHandle ent, NeCompTypeId type, NeCompHandle comp)
{
	return _AddComponent(s, ent, type, comp);
}

bool
E_AddNewComponentS(struct NeScene *s, NeEntityHandle ent, NeCompTypeId type, const void **args)
{
	return _CreateComponent(s, ent, type, args);
}

void *
E_GetComponentS(struct NeScene *s, NeEntityHandle handle, NeCompTypeId type)
{
	uint8_t i = 0;
	struct NeEntity *ent = handle;

	for (i = 0; i < ent->compCount; ++i)
		if (ent->comp[i].type == type)
			return E_ComponentPtrS(s, ent->comp[i].handle);

	return NULL;
}

void
E_GetComponentsS(struct NeScene *s, NeEntityHandle handle, struct NeArray *comp)
{
	struct NeEntity *ent = handle;
	Rt_InitArray(comp, ent->compCount, sizeof(struct NeEntityComp), MH_Frame);
	memcpy(comp->data, ent->comp, Rt_ArrayByteSize(comp));
}

void
E_RemoveComponentS(struct NeScene *s, NeEntityHandle handle, NeCompTypeId type)
{
	struct NeEntity *ent = handle;
	uint8_t id = 0;

	for (id = 0; id < ent->compCount; ++id)
		if (ent->comp[id].type == type)
			break;

	E_DestroyComponentS(s, ent->comp[id].handle);
	--ent->compCount;

	if (id == ent->compCount)
		return;

	memcpy(&ent->comp[id], &ent->compCount, sizeof(struct NeEntityComp));
}

void
E_DestroyEntityS(struct NeScene *s, NeEntityHandle handle)
{
	struct NeEntity *ent = handle;
	size_t dst_id = 0;
	uint8_t i = 0;

	for (i = 0; i < ent->compCount; ++i)
		E_DestroyComponentS(s, ent->comp[i].handle);

	dst_id = ent->id;
	Sys_Free(ent);

	memcpy(Rt_ArrayGet(&s->entities, dst_id), Rt_ArrayLast(&s->entities), s->entities.elemSize);

	ent = Rt_ArrayGetPtr(&s->entities, dst_id);
	ent->id = dst_id;
	--s->entities.count;

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
	memcpy(type.comp_types, compTypes, sizeof(NeCompTypeId) * typeCount);

	return Rt_ArrayAdd(&_entityTypes, &type);
}

void *
E_EntityPtrS(struct NeScene *s, NeEntityHandle handle)
{
	struct NeEntity *ent = handle;
	return Rt_ArrayGetPtr(&s->entities, ent->id);
}

uint32_t
E_EntityCountS(struct NeScene *s)
{
	return (uint32_t)s->entities.count;
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
	strncpy(ent->name, name, MAX_ENTITY_NAME);
	ent->hash = Rt_HashString(ent->name);
}

NeEntityHandle
E_FindEntityS(struct NeScene *s, const char *name)
{
	struct NeEntity *ent = NULL;
	uint64_t hash = Rt_HashString(name);

	Rt_ArrayForEachPtr(ent, &s->entities)
		if (ent->hash == hash)
			return ent;

	return NULL;
}

bool
E_InitEntities(void)
{
	memset(&_entityTypes, 0x0, sizeof(_entityTypes));

	if (!Rt_InitArray(&_entityTypes, 10, sizeof(struct NeEntityType), MH_System))
		return false;

	return true;
}

void
E_TermEntities(void)
{
	Rt_TermArray(&_entityTypes);
}

bool
E_InitSceneEntities(struct NeScene *s)
{
	return Rt_InitArray(&s->entities, 100, sizeof(struct NeEntity *), MH_Scene);
}

void
E_TermSceneEntities(struct NeScene *s)
{
	size_t i = 0;
	for (i = 0; i < s->entities.count; ++i)
		Sys_Free(Rt_ArrayGetPtr(&s->entities, i));
	Rt_TermArray(&s->entities);
}

static int
_TypeCmp(const void *item, const void *data)
{
	return !(((struct NeEntityType *)item)->hash == *((uint64_t *)data));
}

bool
_AddComponent(struct NeScene *s, struct NeEntity *ent, NeCompTypeId type, NeCompHandle handle)
{
	uint8_t i = 0;
	struct NeEntityComp *comp = NULL;

	for (i = 0; i < ent->compCount; ++i)
		if (ent->comp[i].type == type)
			return false;

	comp = &ent->comp[ent->compCount++];
	comp->type = type;
	comp->handle = handle;
	E_SetComponentOwnerS(s, handle, ent);

	return true;
}

bool
_CreateComponent(struct NeScene *s, struct NeEntity *ent, NeCompTypeId type, const void **args)
{
	NeCompHandle handle = E_CreateComponentIdS(s, type, ent, args);

	if (handle == ES_INVALID_COMPONENT)
		return false;

	if (!_AddComponent(s, ent, type, handle)) {
		Sys_LogEntry(ENT_MOD, LOG_CRITICAL, "Failed to add component of type [%d]", type);
		return false;
	}

	return true;
}
