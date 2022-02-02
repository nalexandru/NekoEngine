#include <System/Log.h>
#include <Runtime/Runtime.h>
#include <Scene/Scene.h>
#include <Engine/Component.h>
#include <Engine/Events.h>
#include <Engine/IO.h>
#include <System/Thread.h>

#include "ECS.h"

static int64_t _nextHandle;
static struct NeArray _componentTypes;
static const char _scriptTemplate[] = "function %s_Get%s(c) return CompIF.Get%s(c, %lu, %lu); end\nfunction %s_Set%s(c, v) CompIF.Set%s(c, %lu, %lu, v); end";

#define COMP_MOD	"Component"

static inline bool _InitArray(void);
static int _CompType_cmp(const void *item, const void *data);
static int _CompHandle_cmp(const void *item, const void *data);
static inline struct NeCompHandleData *_HandlePtr(struct NeScene *s, NeCompHandle comp);
static void _ComponentRegistered(struct NeScene *s, struct NeCompType *type);
static void _LoadScript(const char *path);
static bool _ScriptCompInit(void *comp, const void **args, const char *script);
static void _ScriptCompTerm(void *comp, const char *script);

NeCompHandle
E_CreateComponentS(struct NeScene *s, const char *typeName, NeEntityHandle owner, const void **args)
{
	NeCompTypeId id = 0;
	struct NeArray *a = NULL;
	struct NeCompBase *comp = NULL;
	struct NeCompType *type = NULL;
	struct NeCompHandleData *handle = NULL;

	id = E_ComponentTypeId(typeName);
	if (id == RT_NOT_FOUND) {
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Type [%s] does not exist", typeName);
		return ES_INVALID_COMPONENT;
	}
	type = Rt_ArrayGet(&_componentTypes, id);

	Sys_AtomicLockWrite(&s->compLock);

	a = Rt_ArrayGet(&s->compData, id);
	if (!a) {
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Data for type [%s] does not exist", typeName);
		Sys_AtomicUnlockWrite(&s->compLock);
		return ES_INVALID_COMPONENT;
	}

	comp = Rt_ArrayAllocate(a);
	if (!comp) {
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to allocate component of type [%s]", typeName);
		Sys_AtomicUnlockWrite(&s->compLock);
		return ES_INVALID_COMPONENT;
	}

	handle = Rt_ArrayAllocate(&s->compHandle);
	if (!handle) {
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to allocate component handle of type [%s]", typeName);
		Sys_AtomicUnlockWrite(&s->compLock);
		return ES_INVALID_COMPONENT;
	}

	handle->type = id;
	handle->handle = _nextHandle++;
	handle->index = a->count - 1;

	comp->_owner = owner;
	comp->_handleId = s->compHandle.count - 1;
	comp->_sceneId = s->id;

	if (type->init) {
		bool rc = !type->script ? type->init(comp, args) : type->initScript(comp, args, type->script);
		if (!rc) {
			--a->count;
			--_nextHandle;
			--s->compHandle.count;
			Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to create component of type [%s]", typeName);
			Sys_AtomicUnlockWrite(&s->compLock);
			return ES_INVALID_COMPONENT;
		}
	}

	Sys_AtomicUnlockWrite(&s->compLock);

	struct NeComponentCreationData *ccd = Sys_Alloc(sizeof(*ccd), 1, MH_Frame);
	ccd->type = id;
	ccd->handle = handle->handle;
	ccd->owner = owner;
	ccd->ptr = comp;
	E_Broadcast(EVT_COMPONENT_CREATED, ccd);

	return handle->handle;
}

NeCompHandle
E_CreateComponentIdS(struct NeScene *s, NeCompTypeId id, NeEntityHandle owner, const void **args)
{
	struct NeArray *a = NULL;
	struct NeCompType *type = NULL;
	struct NeCompBase *comp = NULL;
	struct NeCompHandleData *handle = NULL;

	type = Rt_ArrayGet(&_componentTypes, id);
	if (!type)
		return -1;

	a = Rt_ArrayGet(&s->compData, id);
	if (!a)
		return -1;

	Sys_AtomicLockWrite(&s->compLock);

	comp = Rt_ArrayAllocate(a);
	if (!comp)
		return -1;

	handle = Rt_ArrayAllocate(&s->compHandle);
	if (!handle) {
		Sys_AtomicUnlockWrite(&s->compLock);
		return ES_INVALID_COMPONENT;
	}

	handle->type = id;
	handle->handle = _nextHandle++;
	handle->index = a->count - 1;

	comp->_owner = owner;
	comp->_handleId = s->compHandle.count - 1;
	comp->_sceneId = s->id;

	if (type->init) {
		bool rc = !type->script ? type->init(comp, args) : type->initScript(comp, args, type->script);
		if (!rc) {
			--a->count;
			--_nextHandle;
			--s->compHandle.count;
			Sys_AtomicUnlockWrite(&s->compLock);
			return ES_INVALID_COMPONENT;
		}
	}

	Sys_AtomicUnlockWrite(&s->compLock);

	struct NeComponentCreationData *ccd = Sys_Alloc(sizeof(*ccd), 1, MH_Frame);
	ccd->type = id;
	ccd->handle = handle->handle;
	ccd->owner = owner;
	ccd->ptr = comp;
	E_Broadcast(EVT_COMPONENT_CREATED, ccd);

	return handle->handle;
}

void
E_DestroyComponentS(struct NeScene *s, NeCompHandle comp)
{
	struct NeCompHandleData *handle = NULL;
	struct NeCompType *type = NULL;
	uint8_t *dst = NULL, *src = NULL;
	size_t dst_index = 0;
	struct NeArray *a = NULL;

	handle = _HandlePtr(s, comp);
	if (!handle)
		return;

	type = Rt_ArrayGet(&_componentTypes, handle->type);
	if (!type)
		return;

	a = Rt_ArrayGet(&s->compData, handle->type);
	if (!a)
		return;

	Sys_AtomicLockWrite(&s->compLock);

	dst_index = handle->index;

	src = Rt_ArrayLast(a);
	dst = Rt_ArrayGet(a, dst_index);

	if (type->term) {
		if (!type->script)
			type->term(dst);
		else
			type->termScript(dst, type->script);
	}

	memcpy(dst, src, a->elemSize);

	handle = Rt_ArrayGet(&s->compHandle, ((struct NeCompBase *)dst)->_handleId);
	handle->index = dst_index;

	--a->count;

	Sys_AtomicUnlockWrite(&s->compLock);

	E_Broadcast(EVT_COMPONENT_DESTROYED, (void *)comp);
}

void *
E_ComponentPtrS(struct NeScene *s, NeCompHandle comp)
{
	void *r = NULL;

	Sys_AtomicLockRead(&s->compLock);

	struct NeCompHandleData *handle = _HandlePtr(s, comp);
	if (handle) {
		const struct NeArray *a = Rt_ArrayGet(&s->compData, handle->type);
		r = a ? Rt_ArrayGet(a, handle->index) : NULL;
	}

	Sys_AtomicUnlockRead(&s->compLock);

	return r;
}

NeCompTypeId
E_ComponentTypeS(struct NeScene *s, NeCompHandle comp)
{
	NeCompTypeId type = ES_INVALID_COMPONENT_TYPE;

	Sys_AtomicLockRead(&s->compLock);
	struct NeCompHandleData *handle = _HandlePtr(s, comp);
	if (handle)
		type = handle->type;
	Sys_AtomicUnlockRead(&s->compLock);

	return type;
}

NeCompTypeId
E_ComponentTypeId(const char *typeName)
{
	uint64_t hash = 0;
	hash = Rt_HashString(typeName);
	return Rt_ArrayFindId(&_componentTypes, &hash, _CompType_cmp);
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
E_ComponentOwnerS(struct NeScene *s, NeCompHandle comp)
{
	NeEntityHandle owner = ES_INVALID_ENTITY;

	Sys_AtomicLockRead(&s->compLock);
	const struct NeCompBase *ptr = E_ComponentPtrS(s, comp);
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

	if (size < sizeof(struct NeCompBase))
		return false;

	if (!_componentTypes.data)
		if (!_InitArray())
			return false;

	if (Rt_ArrayFindId(&_componentTypes, &type.hash, _CompType_cmp) != RT_NOT_FOUND)
		return false;

	if (!Rt_ArrayAdd(&_componentTypes, &type))
		return false;

	E_Broadcast(EVT_COMPONENT_REGISTERED, &type.hash);

	return true;
}

const struct NeArray *
E_GetAllComponentsS(struct NeScene *s, NeCompTypeId type)
{
	if (type >= s->compData.count)
		return NULL;

	return Rt_ArrayGet(&s->compData, type);
}

size_t
E_ComponentSize(struct NeScene *s, const struct NeCompBase *comp)
{
	Sys_AtomicLockRead(&s->compLock);
	const struct NeCompHandleData *handle = Rt_ArrayGet(&s->compHandle, comp->_handleId);
	const struct NeCompType *type = handle ? Rt_ArrayGet(&_componentTypes, handle->type) : NULL;
	Sys_AtomicUnlockRead(&s->compLock);

	return type ? type->size : 0;
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

	if (!Rt_InitArray(&s->compHandle, 100, sizeof(struct NeCompHandleData), MH_Scene))
		return false;

	Rt_FillArray(&s->compData);
	for (i = 0; i < _componentTypes.count; ++i) {
		struct NeCompType *type = Rt_ArrayGet(&_componentTypes, i);
		struct NeArray *a = Rt_ArrayGet(&s->compData, i);

		if (!Rt_InitAlignedArray(a, 10, type->size, type->alignment))
			return false;
	}

	E_RegisterHandler(EVT_COMPONENT_REGISTERED, (NeEventHandlerProc)_ComponentRegistered, s);

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
			if (!type->script)
				for (j = 0; j < a->count; ++j)
					type->term(Rt_ArrayGet(a, j));
			else
				for (j = 0; j < a->count; ++j)
					type->termScript(Rt_ArrayGet(a, j), type->script);
		}

		Sys_Free(type->script);

		Rt_TermArray(a);
	}

	Rt_TermArray(&s->compData);
	Rt_TermArray(&s->compHandle);
}

static inline bool
_InitArray(void)
{
	return Rt_InitArray(&_componentTypes, 40, sizeof(struct NeCompType), MH_System);
}

static int
_CompType_cmp(const void *item, const void *data)
{
	return !(((struct NeCompType *)item)->hash == *((uint64_t *)data));
}

static int
_CompHandle_cmp(const void *item, const void *data)
{
	const struct NeCompHandleData *handle = item;
	const NeCompHandle val = *(NeCompHandle *)data;

	if (handle->handle == val)
		return 0;
	else if (handle->handle > val)
		return 1;
	else
		return -1;
}

static inline struct NeCompHandleData *
_HandlePtr(struct NeScene *s, NeCompHandle comp)
{
	return Rt_ArrayBSearch(&s->compHandle, &comp, _CompHandle_cmp);
}

void
_ComponentRegistered(struct NeScene *s, struct NeCompType *type)
{
	struct NeArray *a = Rt_ArrayAllocate(&s->compData);
	if (!a) {
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to allocate array for registered component in scene %s", s->name);
		return;
	}

	if (!Rt_InitAlignedArray(a, 10, type->size, type->alignment))
		Sys_LogEntry(COMP_MOD, LOG_CRITICAL, "Failed to initialize array for registered component in scene %s", s->name);
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

	type.size = sizeof(struct NeCompBase);
	type.hash = Rt_HashString(name);
	type.alignment = p1 ? atoi(p1) : 1;
	type.script = Rt_StrDup(path, MH_Script);
	type.initScript = _ScriptCompInit;
	type.termScript = _ScriptCompTerm;

	isSize = 512;
	initScript = Sys_Alloc(sizeof(*initScript), isSize, MH_Asset);

	uint32_t i = 0;
	size_t byteOffset = 0;
	while (!E_EndOfStream(&stm)) {
		size_t fieldSize = 0;

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
		} else if (!strncmp(&buff[3], "int64", typeLen)) {
			typeStr = "I64";
			fieldSize = sizeof(int64_t);
		} else if (!strncmp(&buff[3], "float", typeLen)) {
			typeStr = "Flt";
			fieldSize = sizeof(float);
		} else if (!strncmp(&buff[3], "double", typeLen)) {
			typeStr = "Dbl";
			fieldSize = sizeof(double);
		} else if (!strncmp(&buff[3], "bool", typeLen)) {
			typeStr = "Bln";
			fieldSize = sizeof(bool);
		} else if (!strncmp(&buff[3], "string", typeLen)) {
			typeStr = "Str";
			fieldSize = sizeof(char *);
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

		type.size += fieldSize;
		byteOffset += fieldSize;
		++i;
	}

	type.size = (type.size + type.alignment - 1) & ~(type.alignment - 1);

	if (type.size < sizeof(struct NeCompBase))
		goto exit;

	if (Rt_ArrayFindId(&_componentTypes, &type.hash, _CompType_cmp) != RT_NOT_FOUND)
		goto exit;

	if (!Rt_ArrayAdd(&_componentTypes, &type))
		goto exit;

	Sc_RegisterInitScript(initScript);

	E_Broadcast(EVT_COMPONENT_REGISTERED, &type.hash);

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
