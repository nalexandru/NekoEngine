#ifndef NE_ENGINE_RESOURCE_H
#define NE_ENGINE_RESOURCE_H

#include <Engine/IO.h>
#include <Engine/Types.h>
#include <Render/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct NeResourceLoadInfo
{
	struct NeStream stm;
	const char *path;
};

typedef bool (*NeResourceCreateProc)(const char *name, const void *createInfo, void *ptr, NeHandle h);
typedef bool (*NeResourceLoadProc)(struct NeResourceLoadInfo *li, const char *args, void *ptr, NeHandle h);
typedef void (*NeResourceUnloadProc)(void *, NeHandle);

bool E_RegisterResourceType(const char *name, size_t size, NeResourceCreateProc create, NeResourceLoadProc load, NeResourceUnloadProc unload);

NeHandle E_CreateResource(const char *name, const char *type, const void *info);
NeHandle E_LoadResource(const char *path, const char *type);
NeHandle E_AllocateResource(const char *name, const char *type);

void *E_ResourcePtr(NeHandle res);

int32_t	E_ResourceReferences(NeHandle res);
void	E_RetainResource(NeHandle res);
void	E_ReleaseResource(NeHandle res);

static inline uint16_t	E_ResHandleToGPU(NeHandle h) { return (uint16_t)(h & (uint64_t)0x000000000000FFFF); }
NeHandle				E_GPUHandleToRes(uint16_t id, const char *type);

bool	E_UpdateResource(NeHandle res, const void *data);

void	E_UnloadResource(NeHandle res);

void	E_PurgeResources(void);

bool	E_InitResourceSystem(void);
void	E_TermResourceSystem(void);

#ifdef __cplusplus
}
#endif

#endif /* NE_ENGINE_RESOURCE_H */

/* NekoEngine
 *
 * Resource.h
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
