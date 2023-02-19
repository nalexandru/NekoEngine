#include <System/Log.h>
#include <Runtime/Runtime.h>
#include <Scene/Scene.h>
#include <Engine/Component.h>
#include <Engine/Events.h>
#include <Engine/IO.h>
#include <System/Thread.h>

#ifdef _NEKO_EDITOR_
#	include <Editor/Editor.h>
#endif

#include "ECS.h"

static int64_t _nextHandle;
static struct NeArray _componentTypes;
static const char _scriptTemplate[] = "function %s_Get%s(c) return CompIF.Get%s(c, %lu, %lu); end\nfunction %s_Set%s(c, v) CompIF.Set%s(c, %lu, %lu, v); end";

#define COMP_MOD	"Component"
#define EVT_COMPONENT_REGISTERED_PTR	"ComponentRegisteredPTR"

static inline bool _InitArray(void);
static void _ComponentRegistered(struct NeScene *s, struct NeCompType *type);
static void _LoadScript(const char *path);
static bool _ScriptCompInit(void *comp, const void **args, const char *script);
static void _ScriptCompTerm(void *comp, const char *script);

NeCompHandle
E_CreateComponentS(struct NeScene *s, const char *typeName, NeEntityHandle owner, const void **args)
{
	NeCompTypeId typeId = 0;
	struct NeArray *a = NULL;
	struct NeQueue *fl = NULL;
	struct NeCompBase *comp = NULL;
	struct NeCompType *type = NULL;

	typeId = E_ComponentTypeId(typeName);
	if (typeId == RT_NOT_FOUND) {
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Type [%s] does not exist", typeName);
		return E_INVALID_HANDLE;
	}
	type = Rt_ArrayGet(&_componentTypes, typeId);

	Sys_AtomicLockWrite(&s->compLock);

	a = Rt_ArrayGet(&s->compData, typeId);
	if (!a) {
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Data for type [%s] does not exist", typeName);
		goto error;
	}

	fl = Rt_ArrayGet(&s->compFree, typeId);
	if (!fl) {
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Free list for type [%s] does not exist", typeName);
		goto error;
	}

	if (fl->count) {
		size_t idx = *((size_t *) Rt_QueuePop(fl));
		comp = Rt_ArrayGet(a, idx);
		comp->_handleId = idx;
	} else {
		comp = Rt_ArrayAllocate(a);
		if (!comp) {
			Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to allocate component of type [%s]", typeName);
			goto error;
		}

		comp->_handleId = a->count - 1;
	}

	comp->_owner = owner;
	comp->_valid = 1;
	comp->_enabled = 1;
	comp->_typeId = typeId;
	comp->_sceneId = s->id;

	NeCompHandle handle = comp->_handleId | (uint64_t)typeId << 32;

	if (type->init) {
		bool rc = !type->script ? type->init(comp, args) : type->initScript(comp, args, type->script);
		if (!rc) {
			size_t idx = comp->_handleId;
			Rt_QueuePush(fl, &idx);
			--_nextHandle;
			Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to create component of type [%s]", typeName);
			goto error;
		}
	}

	Sys_AtomicUnlockWrite(&s->compLock);

	struct NeComponentCreationData *ccd = Sys_Alloc(sizeof(*ccd), 1, MH_Frame);
	ccd->type = typeId;
	ccd->handle = handle;
	ccd->owner = owner;
	ccd->ptr = comp;
	E_Broadcast(EVT_COMPONENT_CREATED, ccd);

	return handle;

error:
	Sys_AtomicUnlockWrite(&s->compLock);
	return E_INVALID_HANDLE;
}

NeCompHandle
E_CreateComponentIdS(struct NeScene *s, NeCompTypeId typeId, NeEntityHandle owner, const void **args)
{
	struct NeArray *a = NULL;
	struct NeQueue *fl = NULL;
	struct NeCompType *type = NULL;
	struct NeCompBase *comp = NULL;

	type = Rt_ArrayGet(&_componentTypes, typeId);
	if (!type)
		return E_INVALID_HANDLE;

	Sys_AtomicLockWrite(&s->compLock);

	a = Rt_ArrayGet(&s->compData, typeId);
	if (!a)
		goto error;

	fl = Rt_ArrayGet(&s->compFree, typeId);
	if (!fl)
		goto error;

	if (fl->count) {
		size_t idx = *((size_t *) Rt_QueuePop(fl));
		comp = Rt_ArrayGet(a, idx);
		comp->_handleId = idx;
	} else {
		comp = Rt_ArrayAllocate(a);
		if (!comp)
			goto error;

		comp->_handleId = a->count - 1;
	}

	comp->_owner = owner;
	comp->_valid = 1;
	comp->_enabled = 1;
	comp->_typeId = typeId;
	comp->_sceneId = s->id;

	NeCompHandle handle =  comp->_handleId | (uint64_t)typeId << 32;

	if (type->init) {
		bool rc = !type->script ? type->init(comp, args) : type->initScript(comp, args, type->script);
		if (!rc) {
			size_t idx = comp->_handleId;
			Rt_QueuePush(fl, &idx);
			--_nextHandle;
			goto error;
		}
	}

	Sys_AtomicUnlockWrite(&s->compLock);

	struct NeComponentCreationData *ccd = Sys_Alloc(sizeof(*ccd), 1, MH_Frame);
	ccd->type = typeId;
	ccd->handle = handle;
	ccd->owner = owner;
	ccd->ptr = comp;
	E_Broadcast(EVT_COMPONENT_CREATED, ccd);

	return handle;

error:
	Sys_AtomicUnlockWrite(&s->compLock);
	return E_INVALID_HANDLE;
}

void
E_DestroyComponentS(struct NeScene *s, NeCompHandle handle)
{
	struct NeCompType *type = NULL;
	struct NeArray *a = NULL;
	struct NeQueue *fl = NULL;
	size_t idx = E_HANDLE_ID(handle);
	struct NeCompBase *comp = NULL;

	type = Rt_ArrayGet(&_componentTypes, E_HANDLE_TYPE(handle));
	if (!type)
		return;

	a = Rt_ArrayGet(&s->compData, E_HANDLE_TYPE(handle));
	if (!a)
		return;

	fl = Rt_ArrayGet(&s->compFree, E_HANDLE_TYPE(handle));
	if (!fl)
		return;

	Sys_AtomicLockWrite(&s->compLock);

	comp = Rt_ArrayGet(a, idx);

	if (type->term) {
		if (!type->script)
			type->term(comp);
		else
			type->termScript(comp, type->script);
	}

	comp->_valid = false;
	Rt_QueuePush(fl, &idx);

	Sys_AtomicUnlockWrite(&s->compLock);

	E_Broadcast(EVT_COMPONENT_DESTROYED, (void *)handle);
}

void *
E_ComponentPtrS(struct NeScene *s, NeCompHandle handle)
{
	void *r = NULL;

	Sys_AtomicLockRead(&s->compLock);
	{
		const struct NeArray *a = Rt_ArrayGet(&s->compData, E_HANDLE_TYPE(handle));
		r = a ? Rt_ArrayGet(a, E_HANDLE_ID(handle)) : NULL;
	}
	Sys_AtomicUnlockRead(&s->compLock);

	return r;
}

NeCompTypeId
E_ComponentTypeS(struct NeScene *s, NeCompHandle handle)
{
	return E_HANDLE_TYPE(handle);
}

NeCompTypeId
E_ComponentTypeId(const char *typeName)
{
	size_t id = 0;
	uint64_t hash = 0;
	hash = Rt_HashString(typeName);
	id = Rt_ArrayFindId(&_componentTypes, &hash, Rt_U64CmpFunc);
	return id != RT_NOT_FOUND ? id : E_INVALID_HANDLE;
}

const char *
E_ComponentTypeName(NeCompTypeId typeId)
{
	const struct NeCompType *type = Rt_ArrayGet(&_componentTypes, typeId);
	return type ? type->name : NULL;
}

size_t
E_ComponentTypeSize(NeCompTypeId typeId)
{
	const struct NeCompType *type = Rt_ArrayGet(&_componentTypes, typeId);
	return type ? type->size : 0;
}

size_t
E_ComponentCountS(struct NeScene *s, NeCompTypeId type)
{
	Sys_AtomicLockRead(&s->compLock);
	size_t r = ((struct NeArray *)Rt_ArrayGet(&s->compData, type))->count;
	Sys_AtomicUnlockRead(&s->compLock);

	return r;
}

NeEntityHandle
E_ComponentOwnerS(struct NeScene *s, NeCompHandle handle)
{
	NeEntityHandle owner = ES_INVALID_ENTITY;

	Sys_AtomicLockRead(&s->compLock);
	const struct NeCompBase *ptr = E_ComponentPtrS(s, handle);
	if (ptr) owner = ptr->_owner;
	Sys_AtomicUnlockRead(&s->compLock);

	return owner;
}

void
E_SetComponentOwnerS(struct NeScene *s, NeCompHandle comp, NeEntityHandle owner)
{
	struct NeCompBase *ptr = E_ComponentPtrS(s, comp);
	ptr->_owner = owner;
}

bool
E_RegisterComponent(const char *name, size_t size, size_t alignment, NeCompInitProc init, NeCompTermProc term)
{
	struct NeCompType type = { 0 };
	type.size = (size + alignment - 1) & ~(alignment - 1);
	type.alignment = alignment;
	type.hash = Rt_HashString(name);
	type.init = init;
	type.term = term;

	snprintf(type.name, sizeof(type.name), "%s", name);

	if (size < sizeof(struct NeCompBase)) {
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to register component %s: invalid size", name);
		return false;
	}

	if (!_componentTypes.data)
		if (!_InitArray())
			return false;

	if (Rt_ArrayFindId(&_componentTypes, &type.hash, Rt_U64CmpFunc) != RT_NOT_FOUND)
		return false;

	if (!Rt_ArrayAdd(&_componentTypes, &type))
		return false;

	E_Broadcast(EVT_COMPONENT_REGISTERED_PTR, Rt_ArrayLast(&_componentTypes));
	E_Broadcast(EVT_COMPONENT_REGISTERED, (void *)(_componentTypes.count - 1));

	return true;
}

const struct NeArray *
E_GetAllComponentsS(struct NeScene *s, NeCompTypeId type)
{
	if (type >= s->compData.count)
		return NULL;

	return Rt_ArrayGet(&s->compData, type);
}

bool
E_InitComponents(void)
{
	if (!_componentTypes.data)
		if (!_InitArray())
			return false;

	E_ProcessFiles("/Scripts", "lua", true, _LoadScript);

	return true;
}

void
E_TermComponents(void)
{
	Rt_TermArray(&_componentTypes);
}

bool
E_InitSceneComponents(struct NeScene *s)
{
	size_t i = 0;

	if (!Rt_InitArray(&s->compData, _componentTypes.count, sizeof(struct NeArray), MH_Scene))
		return false;

	if (!Rt_InitArray(&s->compFree, _componentTypes.count, sizeof(struct NeQueue), MH_Scene))
		return false;

	Rt_FillArray(&s->compData);
	for (i = 0; i < _componentTypes.count; ++i) {
		struct NeCompType *type = Rt_ArrayGet(&_componentTypes, i);
		struct NeArray *a = Rt_ArrayGet(&s->compData, i);

		if (!Rt_InitAlignedArray(a, 10, type->size, type->alignment))
			return false;
	}

	E_RegisterHandler(EVT_COMPONENT_REGISTERED_PTR, (NeEventHandlerProc)_ComponentRegistered, s);

	return true;
}

void
E_TermSceneComponents(struct NeScene *s)
{
	size_t i = 0, j = 0;
	for (i = 0; i < s->compData.count; ++i) {
		struct NeCompType *type = Rt_ArrayGet(&_componentTypes, i);
		struct NeArray *a = Rt_ArrayGet(&s->compData, i);

		if (type->term) {
			if (!type->script) {
				for (j = 0; j < a->count; ++j) {
					struct NeCompBase *comp = Rt_ArrayGet(a, j);
					if (comp->_valid)
						type->term(comp);
				}
			} else {
				for (j = 0; j < a->count; ++j) {
					struct NeCompBase *comp = Rt_ArrayGet(a, j);
					if (comp->_valid)
						type->termScript(comp, type->script);
				}
			}
		}

		Sys_Free(type->script);

		Rt_TermArray(a);
	}

	Rt_TermArray(&s->compFree);
	Rt_TermArray(&s->compData);
}

static inline bool
_InitArray(void)
{
	return Rt_InitArray(&_componentTypes, 40, sizeof(struct NeCompType), MH_System);
}

void
_ComponentRegistered(struct NeScene *s, struct NeCompType *type)
{
	struct NeArray *a = Rt_ArrayAllocate(&s->compData);
	if (!a) {
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to allocate data array for registered component in scene %s", s->name);
		return;
	}

	if (!Rt_InitAlignedArray(a, 10, type->size, type->alignment))
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to initialize data array for registered component in scene %s", s->name);

	struct NeQueue *q = Rt_ArrayAllocate(&s->compFree);
	if (!q) {
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to allocate free list for registered component in scene %s", s->name);
		return;
	}

	if (Rt_InitQueue(q, 10, sizeof(size_t), MH_Scene))
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to initialize free list for registered component in scene %s", s->name);
}

static void
_LoadScript(const char *path)
{
	char buff[512], *p1 = NULL;
	struct NeCompType type = { 0 };
	struct NeStream stm;
	char *initScript = NULL;
	size_t isLen = 0, isSize = 0;

	if (!E_FileStream(path, IO_READ, &stm))
		return;

	E_ReadStreamLine(&stm, buff, sizeof(buff));

	if (strncmp(buff, "-- NeComponent ", 15))
		goto exit;

	p1 = strchr(&buff[15], ' ');
	*p1++ = 0x0;

	char *name = Rt_TransientStrDup(&buff[15]);
	size_t nameLen = strnlen(name, sizeof(buff) - 15);
	snprintf(type.name, sizeof(type.name), "%s", name);

	type.size = sizeof(struct NeCompBase);
	type.hash = Rt_HashString(name);
	type.alignment = p1 ? atoi(p1) : 1;
	type.script = Rt_StrDup(path, MH_Script);
	type.initScript = _ScriptCompInit;
	type.termScript = _ScriptCompTerm;

	isSize = 512;
	initScript = Sys_Alloc(sizeof(*initScript), isSize, MH_Asset);

#ifdef _NEKO_EDITOR_
	struct NeArray insFields;
	Rt_InitArray(&insFields, 4, sizeof(struct NeDataField), MH_Editor);
#endif

	size_t byteOffset = 0;
	while (!E_EndOfStream(&stm)) {
		size_t fieldSize = 0;

#ifdef _NEKO_EDITOR_
		struct NeDataField field = { .type = FT_UNKNOWN };
#endif

		E_ReadStreamLine(&stm, buff, sizeof(buff));

		if (!strncmp(buff, "-- End", 6))
			break;

		// build init script
		p1 = strchr(&buff[3], ' ');
		*p1++ = 0x0;

		const char *typeStr = NULL;
		size_t typeLen = strnlen(&buff[3], sizeof(buff) - 3);
		if (!strncmp(&buff[3], "int32", typeLen)) {
			typeStr = "I32";
			fieldSize = sizeof(int32_t);
		#ifdef _NEKO_EDITOR_
			field.type = FT_INT32;
		#endif
		} else if (!strncmp(&buff[3], "int64", typeLen)) {
			typeStr = "I64";
			fieldSize = sizeof(int64_t);
		#ifdef _NEKO_EDITOR_
			field.type = FT_INT64;
		#endif
		} else if (!strncmp(&buff[3], "float", typeLen)) {
			typeStr = "Flt";
			fieldSize = sizeof(float);
		#ifdef _NEKO_EDITOR_
			field.type = FT_FLOAT;
		#endif
		} else if (!strncmp(&buff[3], "double", typeLen)) {
			typeStr = "Dbl";
			fieldSize = sizeof(double);
		#ifdef _NEKO_EDITOR_
			field.type = FT_DOUBLE;
		#endif
		} else if (!strncmp(&buff[3], "bool", typeLen)) {
			typeStr = "Bln";
			fieldSize = sizeof(bool);
		#ifdef _NEKO_EDITOR_
			field.type = FT_BOOL;
		#endif
		} else if (!strncmp(&buff[3], "string", typeLen)) {
			typeStr = "Str";
			fieldSize = sizeof(char *);
		#ifdef _NEKO_EDITOR_
			field.type = FT_STRING;
		#endif
		} else if (!strncmp(&buff[3], "quat", typeLen)) {
			typeStr = "Quat";
			fieldSize = sizeof(char *);
		#ifdef _NEKO_EDITOR_
			field.type = FT_QUAT;
		#endif
		} else if (!strncmp(&buff[3], "vec2", typeLen)) {
			typeStr = "Vec2";
			fieldSize = sizeof(char *);
		#ifdef _NEKO_EDITOR_
			field.type = FT_VEC2;
		#endif
		} else if (!strncmp(&buff[3], "vec3", typeLen)) {
			typeStr = "Vec3";
			fieldSize = sizeof(char *);
		#ifdef _NEKO_EDITOR_
			field.type = FT_VEC3;
		#endif
		} else if (!strncmp(&buff[3], "vec4", typeLen)) {
			typeStr = "Vec4";
			fieldSize = sizeof(char *);
		#ifdef _NEKO_EDITOR_
			field.type = FT_VEC4;
		#endif
		} else if (!strncmp(&buff[3], "pointer", typeLen)) {
			typeStr = "Ptr";
			fieldSize = sizeof(void *);
		} else {
			goto exit;
		}

		size_t len = strnlen(p1, sizeof(buff) - typeLen - 3);
		size_t newLen = isLen + ((nameLen + len + strnlen(typeStr, 4)) * 2) + sizeof(_scriptTemplate);
		if (newLen >= isSize) {
			size_t newSize = isSize * 2;
			if (newSize < isSize)
				goto exit;

			char *new = Sys_ReAlloc(initScript, sizeof(*initScript), newSize, MH_Asset);
			if (!new)
				goto exit;

			initScript = new;
			isSize = newSize;
		}

		char *dst = initScript;

		if (isLen) {
			initScript[isLen] = '\n';
			dst = initScript + (isLen + 1);
		}

		snprintf(dst, isSize - isLen, _scriptTemplate, name, p1, typeStr, byteOffset, fieldSize, name, p1, typeStr, byteOffset, fieldSize);
		isLen = strnlen(initScript, isSize);

#ifdef _NEKO_EDITOR_
		if (field.type != FT_UNKNOWN) {
			field.offset = sizeof(struct NeCompBase) + byteOffset;
			snprintf(field.name, sizeof(field.name), "%s", p1);
			Rt_ArrayAdd(&insFields, &field);
		}
#endif

		type.size += fieldSize;
		byteOffset += fieldSize;
	}

	type.size = (type.size + type.alignment - 1) & ~(type.alignment - 1);

	if (type.size < sizeof(struct NeCompBase))
		goto exit;

	if (Rt_ArrayFindId(&_componentTypes, &type.hash, Rt_U64CmpFunc) != RT_NOT_FOUND)
		goto exit;

	if (!Rt_ArrayAdd(&_componentTypes, &type))
		goto exit;

	Sc_RegisterInitScript(initScript);

	E_Broadcast(EVT_COMPONENT_REGISTERED_PTR, Rt_ArrayLast(&_componentTypes));
	E_Broadcast(EVT_COMPONENT_REGISTERED, (void *)(_componentTypes.count - 1));

#ifdef _NEKO_EDITOR_
	struct NeComponentFields f = { .type = _componentTypes.count - 1, .fieldCount = (uint32_t)insFields.count, .fields = (struct NeDataField *)insFields.data };

	if (f.fieldCount) {
		if (!Ed_componentFields.data)
			Rt_InitArray(&Ed_componentFields, 10, sizeof(f), MH_Editor);

		Rt_ArrayAdd(&Ed_componentFields, &f);
	} else {
		Rt_TermArray(&insFields);
	}
#endif

exit:
	Sys_Free(initScript);

	return;
}

static bool
_ScriptCompInit(void *comp, const void **args, const char *script)
{
	bool rc = true;

	lua_State *vm = Sc_CreateVM();
	Sc_LoadScriptFile(vm, script);

	lua_getglobal(vm, "Init");
	if (!lua_isfunction(vm, lua_gettop(vm)))
		goto exit;

	lua_pushlightuserdata(vm, comp);

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

	if (lua_pcall(vm, 2, 1, 0)) {
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
_ScriptCompTerm(void *comp, const char *script)
{
	lua_State *vm = Sc_CreateVM();
	Sc_LoadScriptFile(vm, script);

	lua_getglobal(vm, "Term");
	if (lua_isfunction(vm, lua_gettop(vm))) {
		lua_pushlightuserdata(vm, comp);
		if (lua_pcall(vm, 1, 0, 0)) {
			lua_pop(vm, 1);
		}
	} else {
		lua_pop(vm, 1);
	}
	
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
