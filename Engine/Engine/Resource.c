#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include <Engine/IO.h>
#include <System/Log.h>
#include <Engine/Resource.h>
#include <Runtime/Array.h>
#include <System/AtomicLock.h>

#define RES_MOD	L"Resource"

struct ResourceList
{
	Array res;
	Array free;
	struct AtomicLock *lock;
};

struct ResType
{
	uint64_t hash;
	struct ResourceList list;
	ResourceLoadProc load;
	ResourceUnloadProc unload;
	ResourceCreateProc create;
};

struct ResInfo
{
	uint64_t pathHash;
	uint32_t id;
	int32_t references;
	char path[256];
};

struct Resource
{
	struct ResInfo info;
	uint8_t dataStart;
};

static Array _ResTypes;

static inline Handle _NewResource(const char *path, const char *type, const void *ci, bool create);
static inline struct Resource *_DecodeHandle(Handle res, struct ResType **rt);
static inline void _RealUnload(struct ResType *rt, struct Resource *res);
static int32_t _ResTypeCmp(const void *, const void *);
static inline void _UnloadAll(struct ResType *);

static inline bool _InitResourceList(uint64_t count, size_t size, struct ResourceList *rl);
static inline void _ResourceListAlloc(struct ResourceList *rl, uint64_t *id, void **ptr);
static inline void _ResourceListFree(struct ResourceList *rl, uint64_t id, bool unlock);
static inline void _TermResourceList(struct ResourceList *rl);

bool
E_RegisterResourceType(const char *name, size_t size, ResourceCreateProc create, ResourceLoadProc load, ResourceUnloadProc unload)
{
	struct ResType rt, *ert;

	if (!name || !size || !load)
		return false;

	rt.hash = Rt_HashString(name);
	rt.load = load;
	rt.unload = unload;
	rt.create = create;

	ert = Rt_ArrayFind(&_ResTypes, &rt, _ResTypeCmp);
	if (ert) {
		// TODO: Support multiple handlers ?
		Sys_LogEntry(RES_MOD, LOG_WARNING, L"Attempt to register handler for [%ls] multiple times", name);
		return false;
	}

	if (!_InitResourceList(50, size + sizeof(struct ResInfo), &rt.list)) {
		Sys_LogEntry(RES_MOD, LOG_CRITICAL, L"Failed to initialize resource list for type [%ls]", name);
		return false;
	}

	return Rt_ArrayAdd(&_ResTypes, &rt);
}

Handle
E_CreateResource(const char *name, const char *type, const void *info)
{
	return _NewResource(name, type, info, true);
}

Handle
E_LoadResource(const char *path, const char *type)
{
	return _NewResource(path, type, NULL, false);
}

void *
E_ResourcePtr(Handle res)
{
	struct ResType *rt;
	struct Resource *rptr = _DecodeHandle(res, &rt);
	return rptr ? &rptr->dataStart : NULL;
}

int32_t
E_ResourceReferences(Handle res)
{
	struct ResType *rt;
	struct Resource *rptr = _DecodeHandle(res, &rt);
	return rptr ? rptr->info.references : 0;
}

void
E_RetainResource(Handle res)
{
	struct ResType *rt;
	struct Resource *rptr = _DecodeHandle(res, &rt);
	++rptr->info.references;
}

Handle
E_GPUHandleToRes(uint32_t handle, const char *type)
{
	uint32_t rtId = 0;
	uint64_t hash = Rt_HashString(type);

	rtId = (uint32_t)Rt_ArrayFindId(&_ResTypes, &hash, _ResTypeCmp);
	if (rtId == (uint32_t)RT_NOT_FOUND)
		return (uint32_t)-1;

	return (uint64_t)handle | (uint64_t)rtId << 32;
}

void
E_UnloadResource(Handle res)
{
	struct ResType *rt;
	struct Resource *rptr = NULL;
	
	if (res == E_INVALID_HANDLE)
		return;
	
	rptr = _DecodeHandle(res, &rt);

	if (rptr->info.references == 0)
		return;

	if (--rptr->info.references > 0)
		return;

	_RealUnload(rt, rptr);
}

bool
E_InitResourceSystem(void)
{
	if (!Rt_InitArray(&_ResTypes, 10, sizeof(struct ResType)))
		return false;

	return true;
}

void
E_PurgeResources(void)
{
	size_t i = 0;
	for (i = 0; i < _ResTypes.count; ++i)
		_UnloadAll(Rt_ArrayGet(&_ResTypes, i));
}

void
E_TermResourceSystem(void)
{
	size_t i = 0;

	for (i = 0; i < _ResTypes.count; ++i)
		_TermResourceList(&((struct ResType *)Rt_ArrayGet(&_ResTypes, i))->list);

	Rt_TermArray(&_ResTypes);
	memset(&_ResTypes, 0x0, sizeof(_ResTypes));
}

static inline Handle
_NewResource(const char *path, const char *type, const void *ci, bool create)
{
	bool rc = false;
	uint32_t rt_id = 0;
	Handle ret = E_INVALID_HANDLE;
	uint64_t path_hash = 0, type_hash = 0;
	struct ResType *rt = NULL;
	struct Resource *res = NULL;
	struct ResourceLoadInfo li = { 0, 0, 0, 0 };
	size_t i = 0;

	if (!path || !type)
		return E_INVALID_HANDLE;

	path_hash = Rt_HashString(path);
	type_hash = Rt_HashString(type);

	rt_id = (uint32_t)Rt_ArrayFindId(&_ResTypes, &type_hash, _ResTypeCmp);
	rt = Rt_ArrayGet(&_ResTypes, rt_id);
	if (!rt) {
		Sys_LogEntry(RES_MOD, LOG_CRITICAL, L"Resource type [%s] not found", type);
		return E_INVALID_HANDLE;
	}

	for (i = 0; i < rt->list.res.count; ++i) {
		res = Rt_ArrayGet(&rt->list.res, i);

		if (res->info.pathHash != path_hash)
			continue;

		++res->info.references;

		ret = i;
		rc = true;
		goto exit;
	}

	_ResourceListAlloc(&rt->list, &ret, (void **)&res);

	if (!res)
		return E_INVALID_HANDLE;

	memcpy(res->info.path, path, strlen(path));
	res->info.pathHash = Rt_HashString(res->info.path);
	res->info.id = (uint32_t)ret;
	res->info.references = 1;

	if (create) {
		rc = rt->create == NULL ? true : rt->create(path, ci, &res->dataStart, ret);
	} else {
		char *args = NULL;
		char path_str[256] = { 0 };

		if (rt->load) {
			if ((args = strchr(path, ':'))) {
				memcpy(path_str, path, strlen(path));

				args = path_str + (args - path);
				path = path_str;
				*args++ = 0x0;
			}

			li.path = path;

			if (!E_FileStream(path, IO_READ, &li.stm)) {
				Sys_LogEntry(RES_MOD, LOG_CRITICAL, L"Failed to open file [%s] for resource of type [%s]", path, type);
				rc = false;
				goto exit;
			}

			rc = rt->load(&li, args, &res->dataStart, ret);

			E_CloseStream(&li.stm);
		}
	}

exit:
	if (!rc) {
		_ResourceListFree(&rt->list, ret, true);
		memset(res, 0x0, sizeof(*res));
	} else {
		ret |= (uint64_t)rt_id << 32;
	}

	return rc ? ret : E_INVALID_HANDLE;
}

static inline struct Resource *
_DecodeHandle(Handle res, struct ResType **rt)
{
	uint32_t type = (uint32_t)((res & (uint64_t)0xFFFFFFFF00000000) >> 32);
	uint32_t id = (uint32_t)(res & (uint64_t)0x00000000FFFFFFFF);

	*rt = Rt_ArrayGet(&_ResTypes, type);
	if (!*rt)
		return NULL;

	return Rt_ArrayGet(&(*rt)->list.res, id);
}

static inline void
_RealUnload(struct ResType *rt, struct Resource *res)
{
	_ResourceListFree(&rt->list, res->info.id, false);

	if (rt->unload)
		rt->unload(&res->dataStart, res->info.id);

	Sys_AtomicUnlockWrite(rt->list.lock);
}

static int32_t
_ResTypeCmp(const void *item, const void *data)
{
	const struct ResType *a = item;
	const struct ResType *b = data;

	return a->hash != b->hash;
}

static inline void
_UnloadAll(struct ResType *rt)
{
	size_t i = 0;
	for (i = 0; i < rt->list.res.count; ++i) {
		struct Resource *rptr = Rt_ArrayGet(&rt->list.res, i);

		if (!rptr->info.references)
			continue;

		Sys_LogEntry(RES_MOD, LOG_WARNING,
			L"Resource [%s] has %d reference%s at shutdown time",
			rptr->info.path, rptr->info.references,
			rptr->info.references > 1 ? "s" : "");

		rptr->info.references = 0;
		_RealUnload(rt, rptr);
	}
}

static inline bool
_InitResourceList(uint64_t count, size_t size, struct ResourceList *rl)
{
	if (!Rt_InitArray(&rl->res, (size_t)count, size))
		return false;

	if (!Rt_InitArray(&rl->free, (size_t)count, sizeof(uint32_t)))
		return false;

	rl->lock = Sys_InitAtomicLock();

	return true;
}

static inline void
_ResourceListAlloc(struct ResourceList *rl, uint64_t *id, void **ptr)
{
	Sys_AtomicLockWrite(rl->lock);

	if (rl->free.count) {
		*id = *((uint64_t *)Rt_ArrayLast(&rl->free));
		--rl->free.count;

		*ptr = Rt_ArrayGet(&rl->res, (size_t)*id);
	} else {
		*id = (uint64_t)rl->res.count;
		*ptr = Rt_ArrayAllocate(&rl->res);
	}

	Sys_AtomicUnlockWrite(rl->lock);
}

static inline void
_ResourceListFree(struct ResourceList *rl, uint64_t id, bool unlock)
{
	Sys_AtomicLockWrite(rl->lock);

	Rt_ArrayAdd(&rl->free, &id);

	if (unlock)
		Sys_AtomicUnlockWrite(rl->lock);
}

static inline void
_TermResourceList(struct ResourceList *rl)
{
	Rt_TermArray(&rl->res);
	Rt_TermArray(&rl->free);
	Sys_TermAtomicLock(rl->lock);
}

