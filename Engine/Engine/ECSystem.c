#include <System/Log.h>
#include <System/System.h>
#include <Runtime/Runtime.h>

#include <Engine/ECSystem.h>
#include <Engine/Entity.h>
#include <Engine/Component.h>

#include "ECS.h"

#define ECSYS_MOD	L"ECSystem"

bool E_RegisterSystems(void);

static struct Array _systems;
static struct Array _filteredEntities;

static int _ecsysInsertCmp(const void *item, const void *data);
static inline void _sysExec(struct Scene *s, struct ECSystem *sys, void *args);
static inline void _filterEntities(struct Scene *s, struct Array *ent, CompTypeId *comp_types, size_t type_count);

bool
E_RegisterSystem(const wchar_t *name, const wchar_t *group,
	const wchar_t **comp, size_t num_comp,
	ECSysExecProc proc, int32_t priority)
{
	size_t i = 0;
	CompTypeId types[MAX_ENTITY_COMPONENTS];

	if (num_comp > MAX_ENTITY_COMPONENTS)
		return false;

	for (i = 0; i < num_comp; ++i)
		types[i] = E_ComponentTypeId(comp[i]);

	return E_RegisterSystemId(name, group, types, num_comp, proc, priority);
}

bool
E_RegisterSystemId(const wchar_t *name, const wchar_t *group,
	const CompTypeId *comp, size_t num_comp,
	ECSysExecProc proc, int32_t priority)
{
	size_t pos = 0;
	struct ECSystem sys;

	memset(&sys, 0x0, sizeof(sys));

	if (num_comp > MAX_ENTITY_COMPONENTS)
		return false;

	if (!Rt_ArrayAllocate(&_systems))
		return false;
	--_systems.count;

	sys.name_hash = Rt_HashStringW(name);
	sys.group_hash = Rt_HashStringW(group);

	sys.comp_types = Sys_Alloc(num_comp, sizeof(CompTypeId), MH_System);
	if (!sys.comp_types)
		return false;

	memcpy(sys.comp_types, comp, sizeof(CompTypeId) * num_comp);
	sys.type_count = num_comp;
	sys.exec = proc;
	sys.priority = priority;

	pos = Rt_ArrayFindId(&_systems, &priority, _ecsysInsertCmp);
	if (pos == RT_NOT_FOUND)
		pos = _systems.count;

	if (!Rt_ArrayInsert(&_systems, &sys, pos)) {
		Sys_Free(sys.comp_types);
		return false;
	}

	return true;
}

void
E_ExecuteSystemS(struct Scene *s, const wchar_t *name, void *args)
{
	size_t i = 0;
	struct ECSystem *sys = NULL;
	uint64_t hash = Rt_HashStringW(name);

	for (i = 0; i < _systems.count; ++i) {
		sys = Rt_ArrayGet(&_systems, i);
		if (sys->name_hash == hash)
			break;
		sys = NULL;
	}

	if (!sys)
		return;

	_sysExec(s, sys, args);
}

void
E_ExecuteSystemGroupS(struct Scene *s, const wchar_t *name)
{
	size_t i = 0;
	struct ECSystem *sys = NULL;
	uint64_t hash = Rt_HashStringW(name);

	for (i = 0; i < _systems.count; ++i) {
		sys = Rt_ArrayGet(&_systems, i);

		if (sys->group_hash != hash)
			continue;

		_sysExec(s, sys, NULL);
	}
}

bool
E_InitECSystems(void)
{
	if (!Rt_InitArray(&_systems, 40, sizeof(struct ECSystem), MH_System))
		return false;

	if (!Rt_InitPtrArray(&_filteredEntities, 100, MH_System))
		return false;

	return E_RegisterSystems();
}

void
E_TermECSystems(void)
{
	size_t i = 0;

	for (i = 0; i < _systems.count; ++i)
		Sys_Free(((struct ECSystem *)Rt_ArrayGet(&_systems, i))->comp_types);

	Rt_TermArray(&_systems);
	Rt_TermArray(&_filteredEntities);
}

static int
_ecsysInsertCmp(const void *item, const void *data)
{
	const struct ECSystem *sys = item;
	int32_t priority = *(int32_t *)data;

	if (priority > sys->priority)
		return 1;
	else if (priority < sys->priority)
		return -1;
	else
		return 0;
}

static inline void
_sysExec(struct Scene *s, struct ECSystem *sys, void *args)
{
	size_t i = 0, j = 0;
	void *ptr = NULL;
	const struct Array *comp = NULL;
	EntityHandle handle = 0;
	void *components[MAX_ENTITY_COMPONENTS];

	if (sys->type_count == 1) {
		comp = E_GetAllComponentsS(s, sys->comp_types[0]);

		if (!comp)
			return;

		for (i = 0; i < comp->count; ++i) {
			ptr = Rt_ArrayGet(comp, i);
			sys->exec(&ptr, args);
		}
	} else {
		_filterEntities(s, &_filteredEntities, sys->comp_types, sys->type_count);

		for (i = 0; i < _filteredEntities.count; ++i) {
			handle = (EntityHandle *)Rt_ArrayGetPtr(&_filteredEntities, i);

			for (j = 0; j < sys->type_count; ++j)
				components[j] = E_GetComponentS(s, handle, sys->comp_types[j]);

			sys->exec(components, args);
		}
	}
}

static inline void
_filterEntities(struct Scene *s, struct Array *ent, CompTypeId *comp_types, size_t type_count)
{
	const struct Array *components;
	CompTypeId type = -1;
	size_t count = 0, min_count = SIZE_MAX;
	struct CompBase *comp = NULL;
	bool valid = false;
	size_t i = 0, j = 0;

	for (i = 0; i < type_count; ++i) {
		count = E_ComponentCountS(s, comp_types[i]);

		if (count >= min_count)
			continue;

		type = comp_types[i];
		min_count = count;
	}

	if (type == -1) {
		Sys_LogEntry(ECSYS_MOD, LOG_CRITICAL, L"_filterEntities: Entity with least components not found. Is type_count set ?");
		return;
	}

	Rt_ClearArray(ent, false);
	components = E_GetAllComponentsS(s, type);

	if (ent->size < components->count)
		Rt_ResizeArray(ent, components->count);

	for (i = 0; i < components->count; ++i) {
		comp = Rt_ArrayGet(components, i);

		if (!comp->_owner)
			continue;

		valid = true;

		for (j = 0; j < type_count; ++j) {
			if (E_GetComponentS(s, comp->_owner, comp_types[j]))
				continue;

			valid = false;
			break;
		}

		if (valid)
			Rt_ArrayAddPtr(ent, comp->_owner);
	}
}
