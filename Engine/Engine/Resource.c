#include <stdlib.h>
#include <stdint.h>

#include <Engine/IO.h>
#include <System/Log.h>
#include <Engine/Resource.h>
#include <Runtime/Array.h>
#include <System/AtomicLock.h>

#define RES_MOD	"Resource"

struct NeResourceList
{
	struct NeArray res;
	struct NeArray free;
	struct NeAtomicLock lock;
};

struct NeResType
{
	uint64_t hash;
	struct NeResourceList list;
	NeResourceLoadProc load;
	NeResourceUnloadProc unload;
	NeResourceCreateProc create;
	size_t size;
};

struct NeResInfo
{
	uint64_t pathHash;
	uint32_t id;
	int32_t references;
	uint32_t mutable;
	char path[236];
};

struct NeResource
{
	struct NeResInfo info;
	uint8_t dataStart;
};

static struct NeArray f_ResTypes;

static inline NeHandle NewResource(const char *path, const char *type, const void *ci, bool create, bool mutable);
static inline struct NeResource *DecodeHandle(NeHandle res, struct NeResType **rt);
static inline void RealUnload(struct NeResType *rt, struct NeResource *res);
static inline void UnloadAll(struct NeResType *);

static inline bool InitResourceList(uint64_t count, size_t size, struct NeResourceList *rl);
static inline void ResourceListAlloc(struct NeResourceList *rl, uint64_t *id, void **ptr);
static inline void ResourceListFree(struct NeResourceList *rl, uint64_t id, bool unlock);
static inline void TermResourceList(struct NeResourceList *rl);

bool
E_RegisterResourceType(const char *name, size_t size, NeResourceCreateProc create, NeResourceLoadProc load, NeResourceUnloadProc unload)
{
	struct NeResType rt, *ert;

	if (!name || !size || !load)
		return false;

	rt.hash = Rt_HashString(name);
	rt.load = load;
	rt.unload = unload;
	rt.create = create;
	rt.size = size;

	ert = Rt_ArrayFind(&f_ResTypes, &rt, Rt_U64CmpFunc);
	if (ert) {
		// TODO: Support multiple handlers ?
		Sys_LogEntry(RES_MOD, LOG_WARNING, "Attempt to register handler for [%s] multiple times", name);
		return false;
	}

	if (!InitResourceList(50, size + sizeof(struct NeResInfo), &rt.list)) {
		Sys_LogEntry(RES_MOD, LOG_CRITICAL, "Failed to initialize resource list for type [%s]", name);
		return false;
	}

	return Rt_ArrayAdd(&f_ResTypes, &rt);
}

NeHandle
E_CreateResource(const char *name, const char *type, const void *info)
{
	return NewResource(name, type, info, true, false);
}

NeHandle
E_LoadResource(const char *path, const char *type)
{
	return NewResource(path, type, NULL, false, false);
}

NeHandle
E_AllocateResource(const char *name, const char *type)
{
	return NewResource(name, type, NULL, false, true);
}

void *
E_ResourcePtr(NeHandle res)
{
	struct NeResType *rt;
	struct NeResource *rptr = DecodeHandle(res, &rt);
	return rptr ? &rptr->dataStart : NULL;
}

int32_t
E_ResourceReferences(NeHandle res)
{
	struct NeResType *rt;
	struct NeResource *rptr = DecodeHandle(res, &rt);
	return rptr ? rptr->info.references : 0;
}

void
E_RetainResource(NeHandle res)
{
	struct NeResType *rt;
	struct NeResource *rptr = DecodeHandle(res, &rt);
	if (rptr)
		++rptr->info.references;
}

void
E_ReleaseResource(NeHandle res)
{
	struct NeResType *rt;
	struct NeResource *rptr = DecodeHandle(res, &rt);
	if (rptr)
		--rptr->info.references;
}

NeHandle
E_GPUHandleToRes(uint16_t handle, const char *type)
{
	const uint64_t hash = Rt_HashString(type);

	const uint32_t rtId = (uint32_t)Rt_ArrayFindId(&f_ResTypes, &hash, Rt_U64CmpFunc);
	if (rtId == (uint32_t)RT_NOT_FOUND)
		return (uint32_t)-1;

	return (uint64_t)handle | (uint64_t)rtId << 32;
}

bool
E_UpdateResource(NeHandle res, const void *data)
{
	struct NeResType *rt;
	struct NeResource *rptr = DecodeHandle(res, &rt);

	if (!rptr || !rptr->info.mutable)
		return false;

	memcpy(&rptr->dataStart, data, rt->size);

	return true;
}

void
E_UnloadResource(NeHandle res)
{
	struct NeResType *rt;
	struct NeResource *rptr = NULL;

	if (res == NE_INVALID_HANDLE)
		return;

	rptr = DecodeHandle(res, &rt);
	if (!rptr)
		return;

	if (rptr->info.references == 0)
		return;

	if (--rptr->info.references > 0)
		return;

	RealUnload(rt, rptr);
}

bool
E_InitResourceSystem(void)
{
	if (!Rt_InitArray(&f_ResTypes, 10, sizeof(struct NeResType), MH_System))
		return false;

	return true;
}

void
E_PurgeResources(void)
{
	for (size_t i = 0; i < f_ResTypes.count; ++i)
		UnloadAll(Rt_ArrayGet(&f_ResTypes, i));
}

void
E_TermResourceSystem(void)
{
	for (size_t i = 0; i < f_ResTypes.count; ++i)
		TermResourceList(&((struct NeResType *)Rt_ArrayGet(&f_ResTypes, i))->list);

	Rt_TermArray(&f_ResTypes);
	memset(&f_ResTypes, 0x0, sizeof(f_ResTypes));
}

static inline NeHandle
NewResource(const char *path, const char *type, const void *ci, bool create, bool mutable)
{
	bool rc = false;
	NeHandle ret = NE_INVALID_HANDLE;
	struct NeResType *rt = NULL;
	struct NeResource *res = NULL;
	struct NeResourceLoadInfo li = { 0 };

	if (!path || !type)
		return NE_INVALID_HANDLE;

	const uint64_t path_hash = Rt_HashString(path);
	const uint64_t type_hash = Rt_HashString(type);

	const uint32_t rt_id = (uint32_t)Rt_ArrayFindId(&f_ResTypes, &type_hash, Rt_U64CmpFunc);
	rt = Rt_ArrayGet(&f_ResTypes, rt_id);
	if (!rt) {
		Sys_LogEntry(RES_MOD, LOG_CRITICAL, "Resource type [%s] not found", type);
		return NE_INVALID_HANDLE;
	}

	for (size_t i = 0; i < rt->list.res.count; ++i) {
		res = Rt_ArrayGet(&rt->list.res, i);

		if (res->info.pathHash != path_hash)
			continue;

		++res->info.references;

		ret = i;
		rc = true;
		goto exit;
	}

	ResourceListAlloc(&rt->list, &ret, (void **)&res);

	if (!res)
		return NE_INVALID_HANDLE;

	strlcpy(res->info.path, path, sizeof(res->info.path));
	res->info.pathHash = Rt_HashString(res->info.path);
	res->info.id = (uint32_t)ret;
	res->info.references = 1;

	if (!mutable) {
		if (create) {
			rc = rt->create == NULL ? true : rt->create(path, ci, &res->dataStart, ret);
		} else {
			char *args = NULL;
			char path_str[256] = { 0 };

			if (rt->load) {
				if ((args = strchr(path, ':'))) {
					strlcpy(path_str, path, sizeof(path_str));

					args = path_str + (args - path);
					path = path_str;
					*args++ = 0x0;
				}

				li.path = path;

				if (!E_FileStream(path, IO_READ, &li.stm)) {
					Sys_LogEntry(RES_MOD, LOG_DEBUG, "Failed to open file [%s] for resource of type [%s]", path, type);
					rc = false;
					goto exit;
				}

				rc = rt->load(&li, args, &res->dataStart, ret);

				E_CloseStream(&li.stm);
			}
		}
	} else {
		rc = true;
	}

exit:
	if (!rc) {
		ResourceListFree(&rt->list, ret, true);
		memset(res, 0x0, sizeof(*res));
	} else {
		ret |= (uint64_t)rt_id << 32;
	}

	return rc ? ret : NE_INVALID_HANDLE;
}

static inline struct NeResource *
DecodeHandle(NeHandle res, struct NeResType **rt)
{
	uint32_t id = E_HANDLE_ID(res);

	*rt = Rt_ArrayGet(&f_ResTypes, E_HANDLE_TYPE(res));
	if (!*rt)
		return NULL;

	return id < (*rt)->list.res.count ? Rt_ArrayGet(&(*rt)->list.res, id) : NULL;
}

static inline void
RealUnload(struct NeResType *rt, struct NeResource *res)
{
	ResourceListFree(&rt->list, res->info.id, false);

	if (rt->unload)
		rt->unload(&res->dataStart, res->info.id);

	Sys_ZeroMemory(res, sizeof(*res));

	Sys_AtomicUnlockWrite(&rt->list.lock);
}

static inline void
UnloadAll(struct NeResType *rt)
{
	for (size_t i = 0; i < rt->list.res.count; ++i) {
		struct NeResource *rptr = Rt_ArrayGet(&rt->list.res, i);

		if (!rptr->info.pathHash)
			continue;

		if (rptr->info.references) {
			Sys_LogEntry(RES_MOD, LOG_WARNING,
				"Resource [%s] has %d reference%s at shutdown time",
				rptr->info.path, rptr->info.references,
				rptr->info.references > 1 ? "s" : "");

			rptr->info.references = 0;
		}

		RealUnload(rt, rptr);
	}
}

static inline bool
InitResourceList(uint64_t count, size_t size, struct NeResourceList *rl)
{
	if (!Rt_InitArray(&rl->res, (size_t)count, size, MH_System))
		return false;

	if (!Rt_InitArray(&rl->free, (size_t)count, sizeof(uint32_t), MH_System))
		return false;

	Sys_InitAtomicLock(&rl->lock);

	return true;
}

static inline void
ResourceListAlloc(struct NeResourceList *rl, uint64_t *id, void **ptr)
{
	Sys_AtomicLockWrite(&rl->lock);

	if (rl->free.count) {
		*id = *((uint64_t *)Rt_ArrayLast(&rl->free));
		--rl->free.count;

		*ptr = Rt_ArrayGet(&rl->res, (size_t)*id);
	} else {
		*id = (uint64_t)rl->res.count;
		*ptr = Rt_ArrayAllocate(&rl->res);
	}

	Sys_AtomicUnlockWrite(&rl->lock);
}

static inline void
ResourceListFree(struct NeResourceList *rl, uint64_t id, bool unlock)
{
	Sys_AtomicLockWrite(&rl->lock);

	Rt_ArrayAdd(&rl->free, &id);

	if (unlock)
		Sys_AtomicUnlockWrite(&rl->lock);
}

static inline void
TermResourceList(struct NeResourceList *rl)
{
	Rt_TermArray(&rl->res);
	Rt_TermArray(&rl->free);
}

/* NekoEngine
 *
 * Resource.c
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
