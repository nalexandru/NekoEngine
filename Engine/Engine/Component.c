#include <System/Log.h>
#include <Runtime/Runtime.h>
#include <Scene/Scene.h>
#include <Engine/Component.h>
#include <Engine/Events.h>
#include <Engine/IO.h>
#include <System/Thread.h>
#include <Engine/Config.h>
#include <Script/Interface.h>

#include <Editor/Editor.h>

#include "ECS.h"

static int64_t f_nextHandle;
static struct NeArray f_componentTypes;

#define COMP_MOD	"Component"
#define EVT_COMPONENT_REGISTERED_PTR	"ComponentRegisteredPTR"

static inline bool InitArray(void);
static void ComponentRegistered(struct NeScene *s, struct NeCompType *type);
static void LoadScript(const char *path);
static bool ScriptCompInit(struct NeScriptComponent *comp, const void **args, const char *type, const char *script);
static void ScriptCompTerm(struct NeScriptComponent *comp, const char *type, const char *script);

NeCompHandle
E_CreateComponentIdS(struct NeScene *s, NeCompTypeId typeId, NeEntityHandle owner, const void **args)
{
	struct NeArray *na = NULL, *a = NULL;
	struct NeQueue *fl = NULL;
	struct NeCompType *type = NULL;
	struct NeCompBase *comp = NULL;

	type = Rt_ArrayGet(&f_componentTypes, typeId);
	if (!type)
		return NE_INVALID_HANDLE;

	a = Rt_ArrayGet(&s->compData, typeId);
	if (!a)
		goto error;

	na = Rt_ArrayGet(&s->newCompData, typeId);
	if (!na)
		goto error;

	fl = Rt_ArrayGet(&s->compFree, typeId);
	if (!fl)
		goto error;

	Sys_AtomicLockWrite(&s->lock.newComp);

	comp = Rt_ArrayAllocate(na);
	if (!comp)
		goto error;

	if (fl->count) {
		size_t idx = *((size_t *)Rt_QueuePop(fl));
		comp->_handleId = idx;
	} else {
		uint64_t *offset = Rt_ArrayGet(&s->newCompOffset, typeId);
		comp->_handleId = a->count + (*offset)++;
	}

	Sys_AtomicUnlockWrite(&s->lock.newComp);

	comp->_owner = owner;
	comp->_valid = 1;
	comp->_enabled = 1;
	comp->_typeId = typeId;
	comp->_sceneId = s->id;

	NeCompHandle handle =  comp->_handleId | (uint64_t)typeId << 32;

	if (type->init) {
		bool rc = !type->script ? type->init(comp, args) : type->initScript(comp, args, type->name, type->script);
		if (!rc) {
			size_t idx = comp->_handleId;
			Sys_AtomicLockWrite(&s->lock.newComp);
			Rt_QueuePush(fl, &idx);
			--f_nextHandle;
			goto error;
		}
	}

	return handle;

error:
	Sys_AtomicUnlockWrite(&s->lock.newComp);
	return NE_INVALID_HANDLE;
}

void
E_DestroyComponentS(struct NeScene *s, NeCompHandle handle)
{
	struct NeCompType *type = NULL;
	struct NeArray *a = NULL;
	struct NeQueue *fl = NULL;
	size_t idx = E_HANDLE_ID(handle);
	struct NeCompBase *comp = NULL;

	type = Rt_ArrayGet(&f_componentTypes, E_HANDLE_TYPE(handle));
	if (!type)
		return;

	a = Rt_ArrayGet(&s->compData, E_HANDLE_TYPE(handle));
	if (!a)
		return;

	fl = Rt_ArrayGet(&s->compFree, E_HANDLE_TYPE(handle));
	if (!fl)
		return;

	Sys_AtomicLockWrite(&s->lock.comp);
	comp = Rt_ArrayGet(a, idx);
	Sys_AtomicUnlockWrite(&s->lock.comp);

	if (type->term) {
		if (!type->script)
			type->term(comp);
		else
			type->termScript(comp, type->name, type->script);
	}

	comp->_valid = false;

	Sys_AtomicLockWrite(&s->lock.comp);
	Rt_QueuePush(fl, &idx);
	Sys_AtomicUnlockWrite(&s->lock.comp);

	E_Broadcast(EVT_COMPONENT_DESTROYED, (void *)handle);
}

void *
E_ComponentPtrS(struct NeScene *s, NeCompHandle handle)
{
	struct NeCompBase *comp = NULL;
	if (E_HANDLE_TYPE(handle) >= s->compData.count)
		return NULL;

	struct NeArray *compData = (struct NeArray *)s->compData.data;
	struct NeArray *newCompData = (struct NeArray *)s->newCompData.data;
	Sys_AtomicLockRead(&s->lock.comp);
	{
		comp = Rt_ArrayGet(&compData[E_HANDLE_TYPE(handle)], E_HANDLE_ID(handle));
		if ((!comp || !comp->_valid) && newCompData[E_HANDLE_TYPE(handle)].count) {
			Sys_AtomicLockRead(&s->lock.newComp);
			{
				const size_t id = E_HANDLE_ID(handle);
				Rt_ArrayForEach(comp, &newCompData[E_HANDLE_TYPE(handle)]) {
					if (comp->_handleId == id)
						break;

					comp = NULL;
				}
			}
			Sys_AtomicUnlockRead(&s->lock.newComp);
		}
	}
	Sys_AtomicUnlockRead(&s->lock.comp);

	return comp;
}

NeCompTypeId
E_ComponentTypeS(struct NeScene *s, NeCompHandle handle)
{
	return E_HANDLE_TYPE(handle);
}

NeCompTypeId
E_ComponentTypeId(const char *typeName)
{
	const uint64_t hash = Rt_HashString(typeName);
	const size_t id = Rt_ArrayFindId(&f_componentTypes, &hash, Rt_U64CmpFunc);
	return id != RT_NOT_FOUND ? id : NE_INVALID_HANDLE;
}

const char *
E_ComponentTypeName(NeCompTypeId typeId)
{
	const struct NeCompType *type = Rt_ArrayGet(&f_componentTypes, typeId);
	return type ? type->name : NULL;
}

size_t
E_ComponentTypeSize(NeCompTypeId typeId)
{
	const struct NeCompType *type = Rt_ArrayGet(&f_componentTypes, typeId);
	return type ? type->size : 0;
}

size_t
E_ComponentCountS(struct NeScene *s, NeCompTypeId type)
{
	Sys_AtomicLockRead(&s->lock.comp);
	size_t r = ((struct NeArray *)Rt_ArrayGet(&s->compData, type))->count;
	Sys_AtomicUnlockRead(&s->lock.comp);

	return r;
}

NeEntityHandle
E_ComponentOwnerS(struct NeScene *s, NeCompHandle handle)
{
	NeEntityHandle owner = ES_INVALID_ENTITY;

	Sys_AtomicLockRead(&s->lock.comp);
	const struct NeCompBase *ptr = E_ComponentPtrS(s, handle);
	if (ptr) owner = ptr->_owner;
	Sys_AtomicUnlockRead(&s->lock.comp);

	return owner;
}

void
E_SetComponentOwnerS(struct NeScene *s, NeCompHandle comp, NeEntityHandle owner)
{
	struct NeCompBase *ptr = E_ComponentPtrS(s, comp);
	ptr->_owner = owner;
}

bool
E_RegisterComponent(const char *name, size_t size, size_t alignment, NeCompInitProc init,
					NeCompMessageHandlerProc handler, NeCompTermProc term, NeCompTypeId *id)
{
	struct NeCompType type =
	{
		.size = (size + alignment - 1) & ~(alignment - 1),
		.alignment = alignment,
		.hash = Rt_HashString(name),
		.init = init,
		.term = term,
		.messageHandler = handler
	};
	strlcpy(type.name, name, sizeof(type.name));

	if (size < sizeof(struct NeCompBase)) {
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to register component %s: invalid size", name);
		return false;
	}

	if (!f_componentTypes.data)
		if (!InitArray())
			return false;

	if (Rt_ArrayFindId(&f_componentTypes, &type.hash, Rt_U64CmpFunc) != RT_NOT_FOUND)
		return false;

	if (!Rt_ArrayAdd(&f_componentTypes, &type))
		return false;

	if (id)
		*id = f_componentTypes.count - 1;

	E_Broadcast(EVT_COMPONENT_REGISTERED_PTR, Rt_ArrayLast(&f_componentTypes));
	E_Broadcast(EVT_COMPONENT_REGISTERED, (void *)(f_componentTypes.count - 1));

	return true;
}

const struct NeArray *
E_GetAllComponentsS(struct NeScene *s, NeCompTypeId type)
{
	if (type >= s->compData.count)
		return NULL;

	return Rt_ArrayGet(&s->compData, type);
}

const struct NeCompType *
ECS_ComponentType(NeCompTypeId typeId)
{
	return Rt_ArrayGet(&f_componentTypes, typeId);
}

bool
E_InitComponents(void)
{
	if (!f_componentTypes.data)
		if (!InitArray())
			return false;

	E_ProcessFiles("/Scripts/Components", "lua", true, LoadScript);

	return true;
}

void
E_TermComponents(void)
{
	struct NeCompType *type;
	Rt_ArrayForEach(type, &f_componentTypes)
		Sys_Free(type->script);

	Rt_TermArray(&f_componentTypes);
}

bool
E_InitSceneComponents(struct NeScene *s)
{
	if (!Rt_InitArray(&s->compData, f_componentTypes.count, sizeof(struct NeArray), MH_Scene))
		return false;

	if (!Rt_InitArray(&s->newCompData, f_componentTypes.count, sizeof(struct NeArray), MH_Scene))
		return false;

	if (!Rt_InitArray(&s->newCompOffset, f_componentTypes.count, sizeof(uint64_t), MH_Scene))
		return false;

	if (!Rt_InitArray(&s->compFree, f_componentTypes.count, sizeof(struct NeQueue), MH_Scene))
		return false;

	Rt_FillArray(&s->compData);
	Rt_FillArray(&s->compFree);
	Rt_FillArray(&s->newCompData);
	Rt_FillArray(&s->newCompOffset);

	for (size_t i = 0; i < f_componentTypes.count; ++i) {
		struct NeCompType *type = Rt_ArrayGet(&f_componentTypes, i);

		struct NeArray *a = Rt_ArrayGet(&s->compData, i);
		if (!Rt_InitAlignedArray(a, 10, type->size, type->alignment, MH_Scene))
			return false;

		a = Rt_ArrayGet(&s->newCompData, i);
		if (!Rt_InitAlignedArray(a, 10, type->size, type->alignment, MH_Scene))
			return false;
	}

	E_RegisterHandler(EVT_COMPONENT_REGISTERED_PTR, (NeEventHandlerProc)ComponentRegistered, s);

	return true;
}

void
E_TermSceneComponents(struct NeScene *s)
{
	for (size_t i = 0; i < s->compData.count; ++i) {
		struct NeCompType *type = Rt_ArrayGet(&f_componentTypes, i);
		struct NeArray *a = Rt_ArrayGet(&s->compData, i);

		if (type->term) {
			if (!type->script) {
				for (size_t j = 0; j < a->count; ++j) {
					struct NeCompBase *comp = Rt_ArrayGet(a, j);
					if (comp->_valid)
						type->term(comp);
				}
			} else {
				for (size_t j = 0; j < a->count; ++j) {
					struct NeCompBase *comp = Rt_ArrayGet(a, j);
					if (comp->_valid)
						type->termScript(comp, type->name, type->script);
				}
			}
		}

		Rt_TermArray(a);
		Rt_TermArray(Rt_ArrayGet(&s->newCompData, i));
	}

	Rt_TermArray(&s->newCompOffset);
	Rt_TermArray(&s->newCompData);
	Rt_TermArray(&s->compFree);
	Rt_TermArray(&s->compData);
}

void *
ECS_CommitedComponentPtr(struct NeScene *s, NeCompHandle handle)
{
	struct NeArray *compData = (struct NeArray *)s->compData.data;
	return Rt_ArrayGet(compData, E_HANDLE_ID(handle));
}

void *
ECS_ComponentPtr(struct NeScene *s, NeCompHandle handle)
{
	struct NeArray *compData = (struct NeArray *)s->compData.data;
	struct NeArray *newCompData = (struct NeArray *)s->newCompData.data;
	struct NeCompBase *comp = Rt_ArrayGet(&compData[E_HANDLE_TYPE(handle)], E_HANDLE_ID(handle));
	if ((!comp || !comp->_valid) && newCompData[E_HANDLE_TYPE(handle)].count) {
		const size_t id = E_HANDLE_ID(handle);
		Rt_ArrayForEach(comp, &newCompData[E_HANDLE_TYPE(handle)]) {
			if (comp->_handleId == id)
				break;
			comp = NULL;
		}
	}
	return comp;
}

static inline bool
InitArray(void)
{
	return Rt_InitArray(&f_componentTypes, 40, sizeof(struct NeCompType), MH_System);
}

void
ComponentRegistered(struct NeScene *s, struct NeCompType *type)
{
	struct NeArray *a = Rt_ArrayAllocate(&s->compData);
	if (!a) {
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to allocate data array for registered component in scene %s", s->name);
		return;
	}

	if (!Rt_InitAlignedArray(a, 10, type->size, type->alignment, MH_Scene))
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to initialize data array for registered component in scene %s", s->name);

	a = Rt_ArrayAllocate(&s->newCompData);
	if (!a) {
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to allocate data array for registered component in scene %s", s->name);
		return;
	}

	if (!Rt_InitAlignedArray(a, 10, type->size, type->alignment, MH_Scene))
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to initialize data array for registered component in scene %s", s->name);

	(void)Rt_ArrayAllocate(&s->newCompOffset);

	struct NeQueue *q = Rt_ArrayAllocate(&s->compFree);
	if (!q) {
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to allocate free list for registered component in scene %s", s->name);
		return;
	}

	if (Rt_InitQueue(q, 10, sizeof(size_t), MH_Scene))
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to initialize free list for registered component in scene %s", s->name);
}

static void
LoadScript(const char *path)
{
	lua_State *vm = Sc_CreateVM();

	if (!Sc_LoadScriptFile(vm, path))
		return;

	lua_getglobal(vm, "NeComponent");
	int t = lua_gettop(vm);
	if (!lua_istable(vm, t))
		goto exit;

	char *name = (char *)SIF_OPTSTRINGFIELD(t, "name", NULL);
	if (!name) {
		char *p = Rt_TransientStrDup(path);
		name = strrchr(p, '/'); *name++ = 0x0;
		if ((p = strrchr(name, '.'))) *p = 0x0;
	}

	struct NeCompType type =
	{
		.size = NE_ROUND_UP(sizeof(struct NeScriptComponent), 16),
		.alignment = 16,
		.hash = Rt_HashString(name),
		.script = Rt_StrDup(path, MH_System),
		.initScript = (NeScriptCompInitProc)ScriptCompInit,
		.termScript = (NeScriptCompTermProc)ScriptCompTerm
	};

	if (Rt_ArrayFindId(&f_componentTypes, &type.hash, Rt_U64CmpFunc) != RT_NOT_FOUND)
		goto exit;

	strlcpy(type.name, name, sizeof(type.name));

	char initScript[512];
	snprintf(initScript, sizeof(initScript),
		"local t = {\n"								\
			"__name = \"%s\",\n"					\
			"__index = Component.Index,\n"			\
			"__newindex = Component.NewIndex,\n"	\
			"__tostring = Component.ToString,\n"	\
		"}; return t;",
		 name
	);

	Sc_RegisterInterfaceScript(name, initScript);

	lua_getglobal(vm, "MessageHandler");
	type.scriptMessageHandler = lua_isfunction(vm, lua_gettop(vm));

	if (!Rt_ArrayAdd(&f_componentTypes, &type))
		goto exit;

	E_Broadcast(EVT_COMPONENT_REGISTERED_PTR, Rt_ArrayLast(&f_componentTypes));
	E_Broadcast(EVT_COMPONENT_REGISTERED, (void *)(f_componentTypes.count - 1));

exit:
	Sc_DestroyVM(vm);
	return;
}

static bool
ScriptCompInit(struct NeScriptComponent *comp, const void **args, const char *type, const char *script)
{
	bool rc = true;

	lua_State *vm = Sc_CreateVM();

	char buff[256];
	snprintf(buff, sizeof(buff), "require \"%s\"", type);
	(void)luaL_dostring(vm, buff);

	Sc_LoadScriptFile(vm, script);

	lua_getglobal(vm, "Init");
	if (!lua_isfunction(vm, lua_gettop(vm)))
		goto exit;

	Sc_PushScriptWrapper(vm, comp, type);

	uint32_t count = 0;
	if (args)
		for (const char **i = (const char **)args; *i; ++i)
			++count;

	if (count) {
		lua_createtable(vm, 0, count / 2);
		for (; args && *args; ++args) {
			const char *arg = *args;
			lua_pushstring(vm, *(++args));
			lua_setfield(vm, -2, arg);
		}
	} else {
		lua_pushnil(vm);
	}

	Rt_InitArray(&comp->fields, 5, sizeof(struct NeScriptField), MH_Script);

	struct NeScriptField sf = { .hash = Rt_HashLiteral("owner"), .type = SFT_Entity, .usrdata = comp->_owner, .readOnly = true };
	Rt_ArrayAdd(&comp->fields, &sf);

	sf.hash = Rt_HashLiteral("sceneId"); sf.type = SFT_Integer; sf.integer = (lua_Integer)comp->_sceneId;
	Rt_ArrayAdd(&comp->fields, &sf);

	sf.hash = Rt_HashLiteral("typeId"); sf.type = SFT_Integer; sf.integer = (lua_Integer)comp->_typeId;
	Rt_ArrayAdd(&comp->fields, &sf);

	if (lua_pcall(vm, 2, 1, 0) && lua_gettop(vm)) {
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to execute Init script %s: %s", script, lua_tostring(vm, -1));
		Sc_LogStackDump(vm, LOG_CRITICAL);
		rc = false;
		goto exit;
	}

	if (!lua_isboolean(vm, -1)) {
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Init script %s did not return a boolean: %s", script, lua_tostring(vm, -1));
		rc = false;
		goto exit;
	}

	rc = lua_toboolean(vm, -1);

exit:
	lua_pop(vm, 1);
	Sc_DestroyVM(vm);

	return rc;
}

static void
ScriptCompTerm(struct NeScriptComponent *comp, const char *type, const char *script)
{
	lua_State *vm = Sc_CreateVM();

	char buff[256];
	snprintf(buff, sizeof(buff), "require \"%s\"", type);
	(void)luaL_dostring(vm, buff);

	Sc_LoadScriptFile(vm, script);

	lua_getglobal(vm, "Term");
	if (lua_isfunction(vm, lua_gettop(vm))) {
		Sc_PushScriptWrapper(vm, comp, type);
		if (lua_pcall(vm, 1, 0, 0)) {
			lua_pop(vm, 1);
		}
	} else {
		lua_pop(vm, 1);
	}

	Rt_TermArray(&comp->fields);

	Sc_DestroyVM(vm);
}

/* NekoEngine
 *
 * Component.c
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
