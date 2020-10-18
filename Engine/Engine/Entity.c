#include <Engine/Types.h>
#include <Engine/Entity.h>
#include <Engine/Component.h>
#include <Scene/Scene.h>
#include <System/Log.h>
#include <Runtime/Runtime.h>

#include "ECS.h"

#define ENT_MOD	L"Entity"

static Array _entity_types;

static int _TypeCmp(const void *, const void *);
static inline bool _AddComponent(struct Scene *s, struct Entity *, CompTypeId, CompHandle);
static inline bool _CreateComponent(struct Scene *s, struct Entity *ent, CompTypeId type, const void **args);

EntityHandle
E_CreateEntityS(struct Scene *s, const wchar_t *typeName)
{
	uint64_t hash = 0;
	struct Entity *ent = NULL;
	struct EntityType *type = NULL;

	if (typeName) {
		hash = Rt_HashStringW(typeName);
		type = Rt_ArrayFind(&_entity_types, &hash, _TypeCmp);
		ent = E_CreateEntityWithArgsS(s, type->comp_types, NULL, type->comp_count);
	} else {
		ent = calloc(1, sizeof(*ent));
	}

	if (!ent)
		return ES_INVALID_ENTITY;

	if (!Rt_ArrayAddPtr(&s->entities, ent)) {
		free(ent);
		return ES_INVALID_ENTITY;
	}

	return ent;
}

EntityHandle
E_CreateEntityWithArgsS(struct Scene *s, const CompTypeId *compTypes, const void ***compArgs, uint8_t count)
{
	uint8_t i = 0;
	struct Entity *ent = NULL;

	if (count > MAX_ENTITY_COMPONENTS)
		return ES_INVALID_ENTITY;

	ent = calloc(1, sizeof(*ent));
	if (!ent)
		return ES_INVALID_ENTITY;

	for (i = 0; i < count; ++i) {
		if (!_CreateComponent(s, ent, compTypes[i], compArgs ? compArgs[i] : NULL)) {
			free(ent);
			return ES_INVALID_ENTITY;
		}
	}

	if (!Rt_ArrayAddPtr(&s->entities, ent)) {
		free(ent);
		return ES_INVALID_ENTITY;
	}

	return ent;
}

EntityHandle
E_CreateEntityVS(struct Scene *s, int count, const struct EntityCompInfo *info)
{
	uint8_t i = 0;
	struct Entity *ent = NULL;

	if (count > MAX_ENTITY_COMPONENTS)
		return ES_INVALID_ENTITY;

	ent = calloc(1, sizeof(*ent));
	if (!ent)
		return ES_INVALID_ENTITY;

	for (i = 0; i < count; ++i) {
		if (!_CreateComponent(s, ent, E_ComponentTypeId(info[i].type), info[i].args)) {
			free(ent);
			return ES_INVALID_ENTITY;
		}
	}

	if (!Rt_ArrayAddPtr(&s->entities, ent)) {
		free(ent);
		return ES_INVALID_ENTITY;
	}

	return ent;
}

EntityHandle
E_CreateEntityWithComponentsS(struct Scene *s, int count, ...)
{
	va_list va;
	struct Entity *ent = NULL;
	CompHandle handle;
	uint8_t i = 0;

	if (count > MAX_ENTITY_COMPONENTS)
		return ES_INVALID_ENTITY;

	ent = calloc(1, sizeof(*ent));
	if (!ent)
		return ES_INVALID_ENTITY;

	va_start(va, count);
	for (i = 0; i < count; ++i) {
		handle = va_arg(va, CompHandle);

		if (!_AddComponent(s, ent, E_ComponentTypeS(s, handle), handle)) {
			free(ent);
			return ES_INVALID_ENTITY;
		}
	}

	if (!Rt_ArrayAddPtr(&s->entities, ent)) {
		free(ent);
		return ES_INVALID_ENTITY;
	}

	for (i = 0; i < ent->comp_count; ++i)
		E_SetComponentOwnerS(s, ent->comp[i].handle, ent);

	return ent;
}

bool
E_AddComponentS(struct Scene *s, EntityHandle ent, CompTypeId type, CompHandle comp)
{
	return _AddComponent(s, ent, type, comp);
}

bool
E_AddNewComponentS(struct Scene *s, EntityHandle ent, CompTypeId type, const void **args)
{
	return _CreateComponent(s, ent, type, args);
}

void *
E_GetComponentS(struct Scene *s, EntityHandle handle, CompTypeId type)
{
	uint8_t i = 0;
	struct Entity *ent = handle;

	for (i = 0; i < ent->comp_count; ++i)
		if (ent->comp[i].type == type)
			return E_ComponentPtrS(s, ent->comp[i].handle);

	return NULL;
}

void
E_RemoveComponentS(struct Scene *s, EntityHandle handle, CompTypeId type)
{
	struct Entity *ent = handle;
	uint8_t id = 0;

	for (id = 0; id < ent->comp_count; ++id)
		if (ent->comp[id].type == type)
			break;

	E_DestroyComponentS(s, ent->comp[id].handle);
	--ent->comp_count;

	if (id == ent->comp_count)
		return;

	memcpy(&ent->comp[id], &ent->comp_count, sizeof(struct EntityComp));
}

void
E_DestroyEntityS(struct Scene *s, EntityHandle handle)
{
	struct Entity *ent = handle;
	size_t dst_id = 0;
	uint8_t i = 0;

	for (i = 0; i < ent->comp_count; ++i)
		E_DestroyComponentS(s, ent->comp[i].handle);

	dst_id = ent->id;
	free(ent);

	memcpy(Rt_ArrayGet(&s->entities, dst_id), Rt_ArrayLast(&s->entities),
		sizeof(void *));

	ent = Rt_ArrayGetPtr(&s->entities, dst_id);
	ent->id = dst_id;
	--s->entities.count;
}

bool
E_RegisterEntityType(const wchar_t *typeName, const CompTypeId *compTypes, uint8_t typeCount)
{
	struct EntityType type;

	if (typeCount > MAX_ENTITY_COMPONENTS)
		return false;

	type.hash = Rt_HashStringW(typeName);
	type.comp_count = typeCount;
	memcpy(type.comp_types, compTypes, sizeof(CompTypeId) * typeCount);

	return Rt_ArrayAdd(&_entity_types, &type);
}

void *
E_EntityPtrS(struct Scene *s, EntityHandle handle)
{
	struct Entity *ent = handle;
	return Rt_ArrayGetPtr(&s->entities, ent->id);
}

bool
E_InitEntities(void)
{
	memset(&_entity_types, 0x0, sizeof(_entity_types));

	if (Rt_InitArray(&_entity_types, 10, sizeof(struct EntityType)))
		return false;

	return true;
}

void
E_TermEntities(void)
{
	Rt_TermArray(&_entity_types);
}

bool
E_InitSceneEntities(struct Scene *s)
{
	return Rt_InitArray(&s->entities, 100, sizeof(struct Entity *));
}

void
E_TermSceneEntities(struct Scene *s)
{
	size_t i = 0;
	for (i = 0; i < s->entities.count; ++i)
		free(Rt_ArrayGetPtr(&s->entities, i));
	Rt_TermArray(&s->entities);
}

static int
_TypeCmp(const void *item, const void *data)
{
	return !(((struct EntityType *)item)->hash == *((uint64_t *)data));
}

bool
_AddComponent(struct Scene *s, struct Entity *ent, CompTypeId type, CompHandle handle)
{
	uint8_t i = 0;
	struct EntityComp *comp = NULL;

	for (i = 0; i < ent->comp_count; ++i)
		if (ent->comp[i].type == type)
			return false;

	comp = &ent->comp[ent->comp_count++];
	comp->type = type;
	comp->handle = handle;
	E_SetComponentOwnerS(s, handle, ent);

	return true;
}

bool
_CreateComponent(struct Scene *s, struct Entity *ent, CompTypeId type, const void **args)
{
	CompHandle handle = E_CreateComponentIdS(s, type, ent, args);

	if (handle == ES_INVALID_COMPONENT)
		return false;

	if (!_AddComponent(s, ent, type, handle)) {
		Sys_LogEntry(ENT_MOD, LOG_CRITICAL,
			L"Failed to add component of type [%d]", type);
		return false;
	}

	return true;
}
