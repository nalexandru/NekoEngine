#include "MTLBackend.h"

static id<MTLHeap> _heap;

struct NeTexture *
Re_CreateTransientTexture(const struct NeTextureDesc *tDesc, uint16_t location, uint64_t offset, uint64_t *size)
{
	MTLTextureDescriptor *desc = MTL_TextureDescriptor(MTL_device, tDesc);
	if (!desc)
		return NULL;
	
	struct NeTexture *tex = Sys_Alloc(sizeof(*tex), 1, MH_Frame);
	if (!tex) {
		[desc release];
		return NULL;
	}
	
	desc.storageMode = MTLStorageModePrivate;
	desc.cpuCacheMode = MTLCPUCacheModeWriteCombined;
	
	tex->tex = [_heap newTextureWithDescriptor: desc offset: offset];
	[desc release];

	*size = [tex->tex allocatedSize];
	
	if (location)
		MTL_SetTexture(location, tex->tex);
	
	return tex;
}

struct NeBuffer *
Re_BkCreateTransientBuffer(const struct NeBufferDesc *desc, uint16_t location, uint64_t offset, uint64_t *size)
{
	struct NeBuffer *buff = Sys_Alloc(sizeof(*buff), 1, MH_Frame);
	if (!buff)
		return NULL;
	
	MTLResourceOptions options = MTL_GPUMemoryTypetoResourceOptions([MTL_device hasUnifiedMemory], desc->memoryType);
	
	buff->buff = [_heap newBufferWithLength: desc->size options: options offset: offset];
	buff->location = location;
	
	if (!buff->buff) {
		Sys_Free(buff);
		return NULL;
	}
	
	MTL_SetBuffer(location, buff->buff);
	buff->memoryType = desc->memoryType;

	*size = [buff->buff allocatedSize];
	
	return buff;
}

bool
Re_InitTransientHeap(uint64_t size)
{
	MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc] init];
	heapDesc.size = size;
	heapDesc.storageMode = MTLStorageModePrivate;
	heapDesc.cpuCacheMode = MTLCPUCacheModeWriteCombined;
	heapDesc.hazardTrackingMode = MTLHazardTrackingModeUntracked;
	heapDesc.type = MTLHeapTypePlacement;

	_heap = [MTL_device newHeapWithDescriptor: heapDesc];

	[heapDesc release];
	
	return _heap != nil;
}

bool
Re_ResizeTransientHeap(uint64_t size)
{
	[_heap release];
	
	MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc] init];
	heapDesc.size = size;
	heapDesc.storageMode = MTLStorageModePrivate;
	heapDesc.hazardTrackingMode = MTLHazardTrackingModeUntracked;
	heapDesc.type = MTLHeapTypePlacement;
	_heap = [MTL_device newHeapWithDescriptor: heapDesc];
	
	return _heap != nil;
}

void
Re_TermTransientHeap(void)
{
	[_heap release];
}

/* NekoEngine
 *
 * MTLTransientResources.m
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
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
