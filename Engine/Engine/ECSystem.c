#include <System/Log.h>
#include <System/System.h>
#include <System/Thread.h>
#include <Runtime/Runtime.h>
#include <Scene/Scene.h>
#include <Engine/IO.h>
#include <Engine/Job.h>
#include <Engine/Entity.h>
#include <Engine/Profiler.h>
#include <Engine/ECSystem.h>
#include <Engine/Component.h>

#include "ECS.h"

#define ECSYS_MOD	"ECSystem"

struct NeExecArgs
{
	struct NeECSystem *sys;
	void *components[MAX_ENTITY_COMPONENTS];
	void *args;
};

struct NeJobCompletedArgs
{
	bool done;
	struct NeScene *scene;
};

struct NeSystemInitInfo
{
	const char *name, *group;
	const char *comp[MAX_ENTITY_COMPONENTS];
	size_t numComp;
	NeECSysExecProc proc;
	int32_t priority;
	bool singleThread;
};

static struct NeArray _systems;
static struct NeArray _filteredEntities;
static struct NeArray _initInfo;

static int _ECSysInsertCmp(const void *item, const void *data);
static inline void _SysExec(struct NeScene *s, struct NeECSystem *sys, void *args);
static inline bool *_SysExecJobs(struct NeScene *s, struct NeECSystem *sys, void *args);
static inline void _FilterEntities(struct NeScene *s, struct NeArray *ent, NeCompTypeId *compTypes, size_t typeCount);
static void _ExecJob(int worker, struct NeExecArgs *ea);
static void _JobCompleted(uint64_t id, struct NeJobCompletedArgs *jca);
static inline void _Exec(struct NeECSystem *sys, void **components, void *args);
static void _LoadScript(const char *path);

bool
E_RegisterSystem(const char *name, const char *group,
	const char **comp, size_t numComp,
	NeECSysExecProc proc, int32_t priority,
	bool singleThread)
{
	NeCompTypeId types[MAX_ENTITY_COMPONENTS];

	if (numComp > MAX_ENTITY_COMPONENTS)
		return false;

	if (_systems.data) {
		for (size_t i = 0; i < numComp; ++i) {
			types[i] = E_ComponentTypeId(comp[i]);
			if (types[i] == E_INVALID_HANDLE) {
				Sys_LogEntry(ECSYS_MOD, LOG_CRITICAL, "Failed to register system %s. Component type for %s not found.", name, comp[i]);
				return false;
			}
		}

		return E_RegisterSystemId(name, group, types, numComp, proc, priority, singleThread);
	} else {
		// this branch is taken during initialization, before E_InitECSystems is called

		if (!_initInfo.data)
			Rt_InitArray(&_initInfo, 10, sizeof(struct NeSystemInitInfo), MH_System);

		struct NeSystemInitInfo *init = Rt_ArrayAllocate(&_initInfo);

		init->name = name;
		init->group = group;
		init->numComp = numComp;
		init->proc = proc;
		init->priority = priority;
		init->singleThread = singleThread;

		for (size_t i = 0; i < numComp; ++i)
			init->comp[i] = comp[i];

		return true;
	}
}

bool
E_RegisterSystemId(const char *name, const char *group,
	const NeCompTypeId *comp, size_t numComp,
	NeECSysExecProc proc, int32_t priority,
	bool singleThread)
{
	size_t pos = 0;
	struct NeECSystem sys;

	memset(&sys, 0x0, sizeof(sys));

	if (numComp > MAX_ENTITY_COMPONENTS)
		return false;

	if (!Rt_ArrayAllocate(&_systems))
		return false;
	--_systems.count;

	sys.nameHash = Rt_HashString(name);
	sys.groupHash = Rt_HashString(group);
	sys.singleThread = singleThread;

	sys.compTypes = Sys_Alloc(numComp, sizeof(NeCompTypeId), MH_System);
	if (!sys.compTypes)
		return false;

	memcpy(sys.compTypes, comp, sizeof(NeCompTypeId) * numComp);
	sys.typeCount = numComp;
	sys.exec = proc;
	sys.priority = priority;

	pos = Rt_ArrayFindId(&_systems, &priority, _ECSysInsertCmp);
	if (pos == RT_NOT_FOUND)
		pos = _systems.count;

	if (!Rt_ArrayInsert(&_systems, &sys, pos)) {
		Sys_Free(sys.compTypes);
		return false;
	}

	return true;
}

bool
E_RegisterScriptSystem(const char *name, const char *group,
	const char **comp, size_t numComp,
	const char *script, int32_t priority,
	bool singleThread)
{
	size_t i = 0;
	NeCompTypeId types[MAX_ENTITY_COMPONENTS];

	if (numComp > MAX_ENTITY_COMPONENTS)
		return false;

	for (i = 0; i < numComp; ++i)
		types[i] = E_ComponentTypeId(comp[i]);

	return E_RegisterScriptSystemId(name, group, types, numComp, script, priority, singleThread);
}

bool
E_RegisterScriptSystemId(const char *name, const char *group,
	const NeCompTypeId *comp, size_t numComp,
	const char *script, int32_t priority,
	bool singleThread)
{
	size_t pos = 0;
	struct NeECSystem sys;

	memset(&sys, 0x0, sizeof(sys));

	if (numComp > MAX_ENTITY_COMPONENTS)
		return false;

	if (!Rt_ArrayAllocate(&_systems))
		return false;
	--_systems.count;

	sys.nameHash = Rt_HashString(name);
	sys.groupHash = Rt_HashString(group);
	sys.singleThread = singleThread;

	sys.compTypes = Sys_Alloc(numComp, sizeof(NeCompTypeId), MH_System);
	if (!sys.compTypes)
		return false;

	memcpy(sys.compTypes, comp, sizeof(NeCompTypeId) * numComp);
	sys.typeCount = numComp;
	sys.priority = priority;

	sys.vm = Sc_CreateVM();
	if (!sys.vm)
		goto error;

	if (!Sc_LoadScriptFile(sys.vm, script))
		goto error;

	lua_getglobal(sys.vm, "Execute");

	if (!lua_isfunction(sys.vm, lua_gettop(sys.vm))) {
		Sys_LogEntry(ECSYS_MOD, LOG_CRITICAL, "Execute function not found in script %s", script);
		lua_pop(sys.vm, 1);
		goto error;
	}

	lua_pop(sys.vm, 1);

	pos = Rt_ArrayFindId(&_systems, &priority, _ECSysInsertCmp);
	if (pos == RT_NOT_FOUND)
		pos = _systems.count;

	if (!Rt_ArrayInsert(&_systems, &sys, pos))
		goto error;

	return true;

error:
	if (sys.vm)
		Sc_DestroyVM(sys.vm);

	Sys_Free(sys.compTypes);
	
	return false;
}

void
E_ExecuteSystemS(struct NeScene *s, const char *name, void *args)
{
	size_t i = 0;
	struct NeECSystem *sys = NULL;
	uint64_t hash = Rt_HashString(name);

	for (i = 0; i < _systems.count; ++i) {
		sys = Rt_ArrayGet(&_systems, i);
		if (sys->nameHash == hash)
			break;
		sys = NULL;
	}

	if (!sys)
		return;

	if (sys->singleThread) {
		Prof_InsertMarker("SerialExec");
		_SysExec(s, sys, args);
	} else {
		Prof_InsertMarker("ParallelExec");
		bool *done = _SysExecJobs(s, sys, args);
		while (done && !*done)
			Sys_Yield();
	}
	Prof_InsertMarker("EndExec");
}

void
E_ExecuteSystemGroupS(struct NeScene *s, const char *name)
{
	size_t i = 0;
	struct NeECSystem *sys = NULL;
	uint64_t hash = Rt_HashString(name);

	for (i = 0; i < _systems.count; ++i) {
		sys = Rt_ArrayGet(&_systems, i);

		if (sys->groupHash != hash)
			continue;

		if (sys->singleThread) {
			_SysExec(s, sys, NULL);
		} else {
			bool *done = _SysExecJobs(s, sys, NULL);
			while (done && !*done)
				Sys_Yield();
		}
	}
}

bool
E_InitECSystems(void)
{
	if (!Rt_InitArray(&_systems, 40, sizeof(struct NeECSystem), MH_System))
		return false;

	if (!Rt_InitPtrArray(&_filteredEntities, 100, MH_System))
		return false;

	struct NeSystemInitInfo *info;
	Rt_ArrayForEach(info, &_initInfo)
		E_RegisterSystem(info->name, info->group, info->comp, info->numComp, info->proc, info->priority, info->singleThread);
	Rt_TermArray(&_initInfo);

	E_ProcessFiles("/Scripts", "lua", true, _LoadScript);

	return true;
}

void
E_TermECSystems(void)
{
	struct NeECSystem *sys = NULL;
	Rt_ArrayForEach(sys, &_systems) {
		if (sys->vm)
			Sc_DestroyVM(sys->vm);

		Sys_Free(sys->compTypes);
	}

	Rt_TermArray(&_systems);
	Rt_TermArray(&_filteredEntities);
}

static int
_ECSysInsertCmp(const void *item, const void *data)
{
	const struct NeECSystem *sys = item;
	int32_t priority = *(int32_t *)data;
	
	return sys->priority - priority;
}

static inline void
_SysExec(struct NeScene *s, struct NeECSystem *sys, void *args)
{
	size_t i = 0, j = 0;
	struct NeCompBase *compBase = NULL;
	const struct NeArray *comp = NULL;
	NeEntityHandle handle = 0;
	void *components[MAX_ENTITY_COMPONENTS];

	Sys_AtomicLockRead(&s->compLock);
	Prof_InsertMarker("Lock");

	if (sys->typeCount == 1) {
		comp = E_GetAllComponentsS(s, sys->compTypes[0]);

		if (!comp)
			goto exit;

		for (i = 0; i < comp->count; ++i) {
			compBase = Rt_ArrayGet(comp, i);
			if (compBase->_valid)
				_Exec(sys, (void **)&compBase, args);
		}
	} else {
		_FilterEntities(s, &_filteredEntities, sys->compTypes, sys->typeCount);
		Prof_InsertMarker("Filter");

		for (i = 0; i < _filteredEntities.count; ++i) {
			handle = (NeEntityHandle *)Rt_ArrayGetPtr(&_filteredEntities, i);

			for (j = 0; j < sys->typeCount; ++j)
				components[j] = E_GetComponentS(s, handle, sys->compTypes[j]);

			_Exec(sys, components, args);
		}
	}

exit:
	Sys_AtomicUnlockRead(&s->compLock);
}

static inline bool *
_SysExecJobs(struct NeScene *s, struct NeECSystem *sys, void *args)
{
	const struct NeArray *comp = NULL;
	NeEntityHandle handle = 0;
	struct NeExecArgs **ea;
	size_t count = 0;

	Sys_AtomicLockRead(&s->compLock);

	if (sys->typeCount == 1) {
		comp = E_GetAllComponentsS(s, sys->compTypes[0]);

		if (!comp || !comp->count)
			goto exit;
		
		ea = Sys_Alloc(sizeof(*ea), comp->count, MH_Frame);
		for (size_t i = 0; i < comp->count; ++i) {
			struct NeCompBase *compBase = Rt_ArrayGet(comp, i);
			if (!compBase->_valid || !compBase->_enabled)
				continue;

			ea[count] = Sys_Alloc(sizeof(*ea[0]), 1, MH_Frame);
			ea[count]->sys = sys;
			ea[count]->args = args;
			ea[count]->components[0] = compBase;
			++count;
		}
	} else {
		_FilterEntities(s, &_filteredEntities, sys->compTypes, sys->typeCount);

		if (!_filteredEntities.count)
			goto exit;

		count = _filteredEntities.count;
		ea = Sys_Alloc(sizeof(*ea), count, MH_Frame);
		for (size_t i = 0; i < count; ++i) {
			handle = (NeEntityHandle *)Rt_ArrayGetPtr(&_filteredEntities, i);
			ea[i] = Sys_Alloc(sizeof(*ea[0]), 1, MH_Frame);
			ea[i]->sys = sys;
			ea[i]->args = args;

			for (size_t j = 0; j < sys->typeCount; ++j)
				ea[i]->components[j] = E_GetComponentS(s, handle, sys->compTypes[j]);
		}
	}

	struct NeJobCompletedArgs *jca = Sys_Alloc(sizeof(*jca), 1, MH_Frame);
	jca->done = false;
	jca->scene = s;
	E_DispatchJobs(count, (NeJobProc)_ExecJob, (void **)ea, (NeJobCompletedProc)_JobCompleted, jca);

	return &jca->done;

exit:
	Sys_AtomicUnlockRead(&s->compLock);
	return NULL;
}

static inline void
_FilterEntities(struct NeScene *s, struct NeArray *ent, NeCompTypeId *compTypes, size_t typeCount)
{
	const struct NeArray *components;
	NeCompTypeId type = -1;
	size_t count = 0, min_count = SIZE_MAX;
	struct NeCompBase *comp = NULL;
	bool valid = false;
	size_t i = 0, j = 0;

	for (i = 0; i < typeCount; ++i) {
		count = E_ComponentCountS(s, compTypes[i]);

		if (count >= min_count)
			continue;

		type = compTypes[i];
		min_count = count;
	}

	if (type == -1) {
		Sys_LogEntry(ECSYS_MOD, LOG_CRITICAL, "_filterEntities: Entity with least components not found. Is typeCount set ?");
		return;
	}

	Rt_ClearArray(ent, false);
	components = E_GetAllComponentsS(s, type);

	if (ent->size < components->count)
		Rt_ResizeArray(ent, components->count);

	for (i = 0; i < components->count; ++i) {
		comp = Rt_ArrayGet(components, i);

		if (!comp->_owner || !comp->_valid || !comp->_enabled)
			continue;

		valid = true;

		for (j = 0; j < typeCount; ++j) {
			if (E_GetComponentS(s, comp->_owner, compTypes[j]))
				continue;

			valid = false;
			break;
		}

		if (valid)
			Rt_ArrayAddPtr(ent, comp->_owner);
	}
}

static void
_ExecJob(int worker, struct NeExecArgs *ea)
{
	_Exec(ea->sys, ea->components, ea->args);
}

static void
_JobCompleted(uint64_t id, struct NeJobCompletedArgs *args)
{
	Sys_AtomicUnlockRead(&args->scene->compLock);
	args->done = true;
}

static inline void
_Exec(struct NeECSystem *sys, void **comp, void *args)
{
	if (sys->exec) {
		sys->exec(comp, args);
	} else {
		lua_getglobal(sys->vm, "Execute");

		for (size_t i = 0; i < sys->typeCount; ++i)
			lua_pushlightuserdata(sys->vm, comp[i]);
		lua_pushlightuserdata(sys->vm, args);

		if (lua_pcall(sys->vm, (int)sys->typeCount + 1, 0, 0) && lua_gettop(sys->vm)) {
			Sys_LogEntry(ECSYS_MOD, LOG_CRITICAL, "Failed to execute script system: %s", lua_tostring(sys->vm, -1));
			Sc_LogStackDump(sys->vm, LOG_CRITICAL);
			lua_pop(sys->vm, 1);
		}
	}
}

static void
_LoadScript(const char *path)
{
	char buff[512], *p1 = NULL, *p2 = NULL;
	char *name, *module = NULL;
	int32_t priority = 0;
	struct NeStream stm;
	NeCompTypeId types[MAX_ENTITY_COMPONENTS];
	uint32_t typeCount = 0;
	
	if (!E_FileStream(path, IO_READ, &stm))
		return;

	E_ReadStreamLine(&stm, buff, sizeof(buff));

	if (strncmp(buff, "-- NeSystem ", 12))
		goto exit;

	p1 = strchr(&buff[12], ' ');
	*p1++ = 0x0;
	name = Rt_TransientStrDup(&buff[12]);

	p2 = strchr(p1, ' ');
	*p2++ = 0x0;
	module = Rt_TransientStrDup(p1);
	
	priority = atoi(p2);

	while (!E_EndOfStream(&stm)) {
		E_ReadStreamLine(&stm, buff, sizeof(buff));

		if (!strncmp(buff, "-- End", 6))
			break;

		NeCompTypeId typeId = E_ComponentTypeId(&buff[3]);
		if (typeId == RT_NOT_FOUND)
			break;

		if (typeCount == MAX_ENTITY_COMPONENTS)
			goto exit;

		types[typeCount++] = typeId;
	}

	E_RegisterScriptSystemId(name, module, types, typeCount, path, priority, true);
	
exit:
	E_CloseStream(&stm);
}

/* NekoEngine
 *
 * ECSystem.c
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
