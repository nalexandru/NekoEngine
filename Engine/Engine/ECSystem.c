#include <System/Log.h>
#include <System/System.h>
#include <System/Thread.h>
#include <Runtime/Runtime.h>
#include <Scene/Scene.h>
#include <Engine/IO.h>
#include <Engine/Job.h>
#include <Engine/Entity.h>
#include <Engine/ECSystem.h>
#include <Engine/Component.h>
#include <Script/Interface.h>

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
	const char *name;
	const char *comp[MAX_ENTITY_COMPONENTS];
	uint64_t group;
	size_t numComp;
	NeECSysExecProc proc;
	int32_t priority;
	bool singleThread;
};

static struct NeArray f_systems;
static struct NeArray f_filteredEntities;
static struct NeArray f_initInfo;
static struct NeTSArray f_newSystems;
static void *f_dirWatch;

static int ECSysInsertCmp(const void *item, const void *data);
static inline void SysExec(struct NeScene *s, struct NeECSystem *sys, void *args);
static inline bool *SysExecJobs(struct NeScene *s, struct NeECSystem *sys, void *args);
static inline void FilterEntities(struct NeScene *s, struct NeArray *ent, NeCompTypeId *compTypes, size_t typeCount);
static void ExecJob(int worker, struct NeExecArgs *ea);
static void JobCompleted(uint64_t id, struct NeJobCompletedArgs *args);
static inline void Exec(struct NeECSystem *sys, void **components, void *args);
static inline bool LoadSystemInfo(const char *path, struct NeECSystem *sys);
static void LoadScript(const char *path);
static lua_State *CreateVM(struct NeECSystem *sys, const char *script);
static inline void ScriptExec(lua_State *vm, struct NeECSystem *sys, void **comp, void *args);
static inline void FileChanged(const char *path, enum NeFSEvent event, void *ud);

bool
E_RegisterSystem(const char *name, uint64_t group,
	const char **comp, size_t numComp,
	NeECSysExecProc proc, int32_t priority,
	bool singleThread)
{
	NeCompTypeId types[MAX_ENTITY_COMPONENTS];

	if (numComp > MAX_ENTITY_COMPONENTS)
		return false;

	if (f_systems.data) {
		for (size_t i = 0; i < numComp; ++i) {
			types[i] = E_ComponentTypeId(comp[i]);
			if (types[i] == NE_INVALID_HANDLE) {
				Sys_LogEntry(ECSYS_MOD, LOG_CRITICAL, "Failed to register system %s. Component type for %s not found.", name, comp[i]);
				return false;
			}
		}

		return E_RegisterSystemId(name, group, types, numComp, proc, priority, singleThread);
	} else {
		// this branch is taken during initialization, before E_InitECSystems is called

		if (!f_initInfo.data)
			Rt_InitArray(&f_initInfo, 10, sizeof(struct NeSystemInitInfo), MH_System);

		struct NeSystemInitInfo *init = Rt_ArrayAllocate(&f_initInfo);

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
E_RegisterSystemId(const char *name, uint64_t group,
	const NeCompTypeId *comp, size_t numComp,
	NeECSysExecProc proc, int32_t priority,
	bool singleThread)
{
	struct NeECSystem sys = { 0 };

	if (numComp > MAX_ENTITY_COMPONENTS)
		return false;

	if (!Rt_ArrayAllocate(&f_systems))
		return false;
	--f_systems.count;

	sys.nameHash = Rt_HashString(name);
	sys.groupHash = group;
	sys.singleThread = singleThread;
	sys.enabled = true;

	memcpy(sys.compTypes, comp, sizeof(NeCompTypeId) * numComp);
	sys.typeCount = numComp;
	sys.exec = proc;
	sys.priority = priority;

	size_t pos = Rt_ArrayFindId(&f_systems, &priority, ECSysInsertCmp);
	if (pos == RT_NOT_FOUND)
		pos = f_systems.count;

	if (!Rt_ArrayInsert(&f_systems, &sys, pos))
		return false;

	return true;
}

void
E_ExecuteSystemS(struct NeScene *s, uint64_t hash, void *args)
{
	struct NeECSystem *sys = NULL;
	Rt_ArrayForEach(sys, &f_systems) {
		if (sys->nameHash == hash)
			break;
		sys = NULL;
	}

	if (!sys || !sys->enabled)
		return;

	if (sys->singleThread) {
		SysExec(s, sys, args);
	} else {
		bool *done = SysExecJobs(s, sys, args);
		while (done && !*done)
			Sys_Yield();
	}
}

void
E_ExecuteSystemGroupS(struct NeScene *s, uint64_t hash)
{
	struct NeECSystem *sys;
	Rt_ArrayForEach(sys, &f_systems) {
		if (!sys->enabled || sys->groupHash != hash)
			continue;

		if (sys->singleThread) {
			SysExec(s, sys, NULL);
		} else {
			bool *done = SysExecJobs(s, sys, NULL);
			while (done && !*done)
				Sys_Yield();
		}
	}
}

bool
E_InitECSystems(void)
{
	if (!Rt_InitArray(&f_systems, 40, sizeof(struct NeECSystem), MH_System))
		return false;

	if (!Rt_InitPtrArray(&f_filteredEntities, 100, MH_System))
		return false;

	struct NeSystemInitInfo *info;
	Rt_ArrayForEach(info, &f_initInfo)
		E_RegisterSystem(info->name, info->group, info->comp, info->numComp, info->proc, info->priority, info->singleThread);
	Rt_TermArray(&f_initInfo);

	E_ProcessFiles("/Scripts/Systems", "lua", true, LoadScript);
	f_dirWatch = E_WatchDirectory("/Scripts/Systems", FE_All, FileChanged, NULL);

	return true;
}

void
E_ReloadSystemScripts(void)
{
	char path[256] = { 0 };
	struct NeECSystem *sys = NULL;

	Rt_ArrayForEach(sys, &f_systems) {
		if (!sys->reload)
			continue;

		snprintf(path, sizeof(path), "/Scripts/Systems/%s", sys->reload);
		Sys_Free(sys->reload);
		sys->reload = NULL;

		if (!LoadSystemInfo(path, sys)) {
			Sys_LogEntry(ECSYS_MOD, LOG_CRITICAL, "Failed to reload %s: couldn't load system info", path);
			continue;
		}

		if (sys->singleThread) {
			Sc_DestroyVM(sys->vm);
			if (!(sys->vm = CreateVM(sys, path)))
				sys->enabled = false;
		} else {
			for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
				Sc_DestroyVM(sys->vms[i]);
				if (!(sys->vms[i] = CreateVM(sys, path)))
					sys->enabled = false;
			}
		}

		sys->enabled = true;
		Sys_LogEntry(ECSYS_MOD, LOG_DEBUG, "%s reloaded", path);
	}

	if (!f_newSystems.a.data)
		return;

	Rt_LockTSArray(&f_newSystems);
	char *script;
	Rt_ArrayForEachPtr(script, &f_newSystems.a) {
		snprintf(path, sizeof(path), "/Scripts/Systems/%s", script);
		LoadScript(path);
		Sys_LogEntry(ECSYS_MOD, LOG_DEBUG, "%s loaded", path);
		Sys_Free(script);
	}
	Rt_ClearArray(&f_newSystems.a, false);
	Rt_UnlockTSArray(&f_newSystems);
}

void
E_TermECSystems(void)
{
	E_RemoveWatch(f_dirWatch);

	struct NeECSystem *sys = NULL;
	Rt_ArrayForEach(sys, &f_systems) {
		if (sys->vm) {
			if (sys->singleThread) {
				Sc_DestroyVM(sys->vm);
			} else {
				for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i)
					Sc_DestroyVM(sys->vms[i]);
				Sys_Free(sys->vms);
			}
		}
	}

	if (f_newSystems.a.data)
		Rt_TermTSArray(&f_newSystems);

	Rt_TermArray(&f_systems);
	Rt_TermArray(&f_filteredEntities);
}

static int
ECSysInsertCmp(const void *item, const void *data)
{
	const struct NeECSystem *sys = item;
	int32_t priority = *(int32_t *)data;

	return sys->priority - priority;
}

static inline void
SysExec(struct NeScene *s, struct NeECSystem *sys, void *args)
{
	struct NeCompBase *compBase = NULL;
	const struct NeArray *comp = NULL;
	void *components[MAX_ENTITY_COMPONENTS];

	Sys_AtomicLockRead(&s->lock.comp);

	if (sys->typeCount == 1) {
		comp = E_GetAllComponentsS(s, sys->compTypes[0]);

		if (!comp)
			goto exit;

		for (size_t i = 0; i < comp->count; ++i) {
			compBase = Rt_ArrayGet(comp, i);
			if (compBase->_valid)
				Exec(sys, (void **)&compBase, args);
		}
	} else {
		FilterEntities(s, &f_filteredEntities, sys->compTypes, sys->typeCount);

		for (size_t i = 0; i < f_filteredEntities.count; ++i) {
			NeEntityHandle handle = (NeEntityHandle *)Rt_ArrayGetPtr(&f_filteredEntities, i);

			for (size_t j = 0; j < sys->typeCount; ++j)
				components[j] = ECS_GetComponent(s, handle, sys->compTypes[j]);

			Exec(sys, components, args);
		}
	}

exit:
	Sys_AtomicUnlockRead(&s->lock.comp);
}

static inline bool *
SysExecJobs(struct NeScene *s, struct NeECSystem *sys, void *args)
{
	const struct NeArray *comp = NULL;
	struct NeExecArgs **ea;
	size_t count = 0;

	Sys_AtomicLockRead(&s->lock.comp);

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
		FilterEntities(s, &f_filteredEntities, sys->compTypes, sys->typeCount);

		if (!f_filteredEntities.count)
			goto exit;

		count = f_filteredEntities.count;
		ea = Sys_Alloc(sizeof(*ea), count, MH_Frame);
		for (size_t i = 0; i < count; ++i) {
			NeEntityHandle handle = (NeEntityHandle *)Rt_ArrayGetPtr(&f_filteredEntities, i);
			ea[i] = Sys_Alloc(sizeof(*ea[0]), 1, MH_Frame);
			ea[i]->sys = sys;
			ea[i]->args = args;

			for (size_t j = 0; j < sys->typeCount; ++j)
				ea[i]->components[j] = ECS_GetComponent(s, handle, sys->compTypes[j]);
		}
	}

	struct NeJobCompletedArgs *jca = Sys_Alloc(sizeof(*jca), 1, MH_Frame);
	jca->done = false;
	jca->scene = s;
	E_DispatchJobs(count, (NeJobProc)ExecJob, (void **)ea, (NeJobCompletedProc)JobCompleted, jca);

	return &jca->done;

exit:
	Sys_AtomicUnlockRead(&s->lock.comp);
	return NULL;
}

static inline void
FilterEntities(struct NeScene *s, struct NeArray *ent, NeCompTypeId *compTypes, size_t typeCount)
{
	const struct NeArray *components;
	NeCompTypeId type = -1;
	size_t min_count = SIZE_MAX;
	struct NeCompBase *comp = NULL;
	bool valid = false;

	for (size_t i = 0; i < typeCount; ++i) {
		const size_t count = E_ComponentCountS(s, compTypes[i]);

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

	for (size_t i = 0; i < components->count; ++i) {
		comp = Rt_ArrayGet(components, i);

		if (!comp->_owner || !comp->_valid || !comp->_enabled)
			continue;

		valid = true;

		for (size_t j = 0; j < typeCount; ++j) {
			if (ECS_GetComponent(s, comp->_owner, compTypes[j]))
				continue;

			valid = false;
			break;
		}

		if (valid)
			Rt_ArrayAddPtr(ent, comp->_owner);
	}
}

static void
ExecJob(int worker, struct NeExecArgs *ea)
{
	Exec(ea->sys, ea->components, ea->args);
}

static void
JobCompleted(uint64_t id, struct NeJobCompletedArgs *args)
{
	Sys_AtomicUnlockRead(&args->scene->lock.comp);
	args->done = true;
}

static inline void
Exec(struct NeECSystem *sys, void **comp, void *args)
{
	if (sys->exec)
		sys->exec(comp, args);
	else
		ScriptExec(sys->singleThread ? sys->vm : sys->vms[E_WorkerId()], sys, comp, args);
}

static inline bool
LoadSystemInfo(const char *path, struct NeECSystem *sys)
{
	bool rc = false;
	lua_State *vm = Sc_CreateVM();

	if (!Sc_LoadScriptFile(vm, path))
		goto exit;

	lua_getglobal(vm, "NeSystem");
	int t = lua_gettop(vm);
	if (!lua_istable(vm, t))
		goto exit;

	char *name = (char *)SIF_OPTSTRINGFIELD(t, "name", NULL);
	if (!name) {
		char *p = Rt_TransientStrDup(path);
		name = strrchr(p, '/'); *name++ = 0x0;
		if ((p = strrchr(name, '.'))) *p = 0x0;
	}

	int f = SIF_GETFIELD(t, "components");
	const uint32_t typeCount = (uint32_t)lua_rawlen(vm, f);
	if (!typeCount || typeCount > MAX_ENTITY_COMPONENTS) {
		Sys_LogEntry(ECSYS_MOD, LOG_CRITICAL, "No components found for system %s", name);
		goto exit;
	}

	NeCompTypeId *types = Sys_Alloc(sizeof(*types), typeCount, MH_Transient);
	for (uint32_t i = 0; i < typeCount; ++i) {
		lua_rawgeti(vm, f, i + 1);
		int v = lua_gettop(vm);

		if (!lua_isstring(vm, v)) {
			Sys_LogEntry(ECSYS_MOD, LOG_CRITICAL, "Error while loading system %s: Components must be strings", name);
			goto exit;
		}

		const char *type = lua_tostring(vm, v);
		if ((types[i] = E_ComponentTypeId(type)) == -1) {
			Sys_LogEntry(ECSYS_MOD, LOG_CRITICAL, "Error while loading system %s: Component %s not registered", name, type);
			goto exit;
		}

		lua_remove(vm, v);
	}
	SIF_POPFIELD(f);

	// all good, commit
	memcpy(sys->compTypes, types, sizeof(*types) * typeCount);
	sys->typeCount = typeCount;

	sys->nameHash = Rt_HashString(name);
	sys->groupHash = SIF_OPTU64FIELD(t, "group", ECSYS_GROUP_LOGIC_HASH);
	sys->singleThread = SIF_OPTBOOLFIELD(t, "singleThread", false);
	sys->priority = SIF_OPTINTFIELD(t, "priority", 0);

	rc = true;
exit:
	lua_close(vm);
	return rc;
}

static void
LoadScript(const char *path)
{
	struct NeECSystem sys = { 0 };
	if (!LoadSystemInfo(path, &sys))
		return;

	if (!Rt_ArrayAllocate(&f_systems))
		return;
	--f_systems.count;

	sys.scriptHash = Rt_HashString(strrchr(Rt_StrDup(path, MH_Transient), '/') + 1);
	sys.enabled = true;

	if (sys.singleThread) {
		sys.vm = CreateVM(&sys, path);
		if (!sys.vm)
			goto error;
	} else {
		sys.vms = Sys_Alloc(sizeof(*sys.vms), E_JobWorkerThreads(), MH_System);
		for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i)
			if (!(sys.vms[i] = CreateVM(&sys, path)))
				goto error;
	}

	size_t pos = Rt_ArrayFindId(&f_systems, &sys.priority, ECSysInsertCmp);
	if (pos == RT_NOT_FOUND)
		pos = f_systems.count;

	if (!Rt_ArrayInsert(&f_systems, &sys, pos))
		goto error;

	return;
error:
	if (sys.vm) {
		if (sys.singleThread) {
			Sc_DestroyVM(sys.vm);
		} else {
			for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i)
				Sc_DestroyVM(sys.vms[i]);
			Sys_Free(sys.vms);
		}
	}
}

static lua_State *
CreateVM(struct NeECSystem *sys, const char *script)
{
	lua_State *vm = Sc_CreateVM();
	if (!vm)
		return NULL;

	if (!Sc_LoadScriptFile(vm, script)) {
		lua_close(vm);
		return NULL;
	}

	lua_getglobal(vm, "Execute");

	if (!lua_isfunction(vm, lua_gettop(vm))) {
		Sys_LogEntry(ECSYS_MOD, LOG_CRITICAL, "Execute function not found in script %s", script);
		lua_pop(vm, 1);
		lua_close(vm);
		return NULL;
	}

	for (uint32_t i = 0; i < sys->typeCount; ++i)
		Sc_PushScriptWrapper(vm, NULL, E_ComponentTypeName(sys->compTypes[i]));

	return vm;
}

static inline void
ScriptExec(lua_State *vm, struct NeECSystem *sys, void **comp, void *args)
{
	const int sp = -(1 + (int32_t)sys->typeCount);
	lua_pushvalue(vm, sp);

	for (size_t i = 0; i < sys->typeCount; ++i) {
		struct NeScriptWrapper *sw = lua_touserdata(vm, sp);
		sw->comp = comp[i];
		lua_pushvalue(vm, sp);
	}
	lua_pushlightuserdata(vm, args);

	if (lua_pcall(vm, (int)sys->typeCount + 1, 0, 0) && lua_gettop(vm)) {
		Sys_LogEntry(ECSYS_MOD, LOG_CRITICAL, "Failed to execute script system: %s", lua_tostring(vm, -1));
		Sc_LogStackDump(vm, LOG_CRITICAL);
		lua_pop(vm, 1);
	}
}

static inline void
FileChanged(const char *path, enum NeFSEvent event, void *ud)
{
	if (event == FE_Create) {
		if (!f_newSystems.a.data)
			Rt_InitPtrTSArray(&f_newSystems, 1, MH_System);

		Rt_TSArrayAddPtr(&f_newSystems, Rt_StrDup(path, MH_System));
	} else {
		const uint64_t hash = Rt_HashString(path);

		struct NeECSystem *sys;
		Rt_ArrayForEach(sys, &f_systems) {
			if (sys->scriptHash != hash)
				continue;

			switch (event) {
			case FE_Modify:
				sys->reload = Rt_StrDup(path, MH_System);
			break;
			case FE_Delete:
				sys->enabled = false;
				sys->nameHash = 0;
				Sys_LogEntry(ECSYS_MOD, LOG_DEBUG, "%s deleted", path);
			break;
			}

			break;
		}
	}
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
