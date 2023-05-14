#include "MTLBackend.h"

struct NeBuffer *
Re_BkCreateBuffer(const struct NeBufferDesc *desc, uint16_t location)
{
	struct NeBuffer *buff = Sys_Alloc(sizeof(*buff), 1, MH_RenderBackend);
	if (!buff)
		return NULL;
	
	MTLResourceOptions options = MTL_GPUMemoryTypetoResourceOptions([MTL_device hasUnifiedMemory], desc->memoryType);

	if (desc->usage > (BU_TRANSFER_SRC | BU_TRANSFER_DST))
		buff->buff = MTLBk_CreateBuffer(MTL_device, desc->size, options);
	else
		buff->buff = [MTL_device newBufferWithLength: desc->size options: options];

	if (!buff->buff) {
		Sys_Free(buff);
		return NULL;
	}
	
	MTL_SetBuffer(location, buff->buff);
	buff->memoryType = desc->memoryType;
	buff->location = location;

	return buff;
}

void
Re_BkUpdateBuffer(struct NeBuffer *buff, uint64_t offset, void *data, uint64_t size)
{
	if (buff->memoryType == MT_GPU_LOCAL) {
		id<MTLBuffer> staging = [MTL_device newBufferWithBytes: data length: size options: MTLResourceStorageModeShared];
		
		id<MTLCommandQueue> queue = [MTL_device newCommandQueue];
		id<MTLCommandBuffer> cmdBuffer = [queue commandBufferWithUnretainedReferences];
		id<MTLBlitCommandEncoder> encoder = [cmdBuffer blitCommandEncoder];
		
		[encoder copyFromBuffer: staging
				   sourceOffset: 0
					   toBuffer: buff->buff
			  destinationOffset: offset
						   size: size];
		[encoder endEncoding];
		
		[cmdBuffer commit];
		[cmdBuffer waitUntilCompleted];

		[staging release];
		[queue release];
	} else {
		uint8_t *dst = [buff->buff contents];
		dst += offset;
	
		memcpy(dst, data, size);
	
	#if TARGET_OS_OSX
		if (![MTL_device hasUnifiedMemory] && !(buff->buff.resourceOptions & MTLResourceStorageModeShared))
			[buff->buff didModifyRange: NSMakeRange(offset, size)];
	#endif
	}
}

void *
Re_BkMapBuffer(struct NeBuffer *buff)
{
	return [buff->buff contents];
}

void
Re_BkFlushBuffer(struct NeBuffer *buff, uint64_t offset, uint64_t size)
{
#if TARGET_OS_OSX
	if (![MTL_device hasUnifiedMemory] && !(buff->buff.resourceOptions & MTLResourceStorageModeShared))
		[buff->buff didModifyRange: NSMakeRange(offset, size)];
#else
	(void)buff; (void)offset; (void)size;
#endif
}

void
Re_BkUnmapBuffer(struct NeBuffer *buff)
{
#if TARGET_OS_OSX
	if (![MTL_device hasUnifiedMemory] && !(buff->buff.resourceOptions & MTLResourceStorageModeShared))
		[buff->buff didModifyRange: NSMakeRange(0, 0)];
#else
	(void)buff;
#endif
}

uint64_t
Re_BkBufferAddress(const struct NeBuffer *buff, uint64_t offset)
{
	uint64_t r = 0;
	uint32_t *v = (uint32_t *)&r;
	v[0] = buff->location;
	v[1] = (uint32_t)offset;
	return r;
}

uint64_t
Re_OffsetAddress(uint64_t addr, uint64_t offset)
{
	uint32_t *v = (uint32_t *)&addr;
	v[1] += (uint32_t)offset;
	return addr;
}

void
Re_BkDestroyBuffer(struct NeBuffer *buff)
{
	MTL_RemoveBuffer(buff->buff);
	[buff->buff release];
	Sys_Free(buff);
}

/* NekoEngine
 *
 * MTLBuffer.m
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
