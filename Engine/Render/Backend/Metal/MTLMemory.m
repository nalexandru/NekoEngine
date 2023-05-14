#include "MTLBackend.h"

struct HeapInfo
{
	id<MTLDevice> dev;
	id<MTLHeap> heap;
	MTLResourceOptions options;
};

static struct NeArray _textureHeapInfo, _bufferHeapInfo, _textureHeaps, _bufferHeaps;

bool
MTLBk_InitMemory(void)
{
	Rt_InitArray(&_bufferHeapInfo, 10, sizeof(struct HeapInfo), MH_RenderBackend);
	Rt_InitArray(&_textureHeapInfo, 10, sizeof(struct HeapInfo), MH_RenderBackend);

	Rt_InitPtrArray(&_bufferHeaps, 10, MH_RenderBackend);
	Rt_InitPtrArray(&_textureHeaps, 10, MH_RenderBackend);

	return true;
}

id<MTLBuffer>
MTLBk_CreateBuffer(id<MTLDevice> dev, uint64_t size, MTLResourceOptions options)
{
	struct HeapInfo *info;
	Rt_ArrayForEach(info, &_bufferHeapInfo) {
		if (info->dev != dev || info->options != options)
			continue;

		id<MTLBuffer> buff = [info->heap newBufferWithLength: size options: options];
		if (buff)
			return buff;
	}

	MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc] init];
	heapDesc.size = E_GetCVarU32("MetalBackend_BufferHeapSize", 128 * 1024 * 1024)->u32;
	heapDesc.resourceOptions = options;
	heapDesc.hazardTrackingMode = MTLHazardTrackingModeTracked;
	heapDesc.type = MTLHeapTypeAutomatic;

	id<MTLHeap> heap = [dev newHeapWithDescriptor: heapDesc];

	[heapDesc release];

	struct HeapInfo newInfo = { .dev = dev, .heap = heap, .options = options };
	Rt_ArrayAdd(&_bufferHeapInfo, &newInfo);
	Rt_ArrayAddPtr(&_bufferHeaps, heap);

	return [heap newBufferWithLength: size options: options];
}

id<MTLTexture>
MTLBk_CreateTexture(id<MTLDevice> dev, MTLTextureDescriptor *desc)
{
	struct HeapInfo *info;
	Rt_ArrayForEach(info, &_textureHeapInfo) {
		if (info->dev != dev || info->options != [desc resourceOptions])
			continue;

		id<MTLTexture> tex = [info->heap newTextureWithDescriptor: desc];
		if (tex)
			return tex;
	}

	MTLHeapDescriptor *heapDesc = [[MTLHeapDescriptor alloc] init];
	heapDesc.size = E_GetCVarU32("MetalBackend_TextureHeapSize", 256 * 1024 * 1024)->u32;
	heapDesc.resourceOptions = [desc resourceOptions];
	heapDesc.hazardTrackingMode = MTLHazardTrackingModeTracked;
	heapDesc.type = MTLHeapTypeAutomatic;
	id<MTLHeap> heap = [dev newHeapWithDescriptor: heapDesc];

	struct HeapInfo newInfo = { .dev = dev, .heap = heap, .options = [desc resourceOptions] };
	Rt_ArrayAdd(&_textureHeapInfo, &newInfo);
	Rt_ArrayAddPtr(&_textureHeaps, heap);

	return [heap newTextureWithDescriptor: desc];
}

void
MTLBk_SetRenderHeaps(id<MTLRenderCommandEncoder> encoder)
{
	if (@available(macOS 13, iOS 16, *)) {
		if (_textureHeaps.count)
			[encoder useHeaps: (id<MTLHeap> *)_textureHeaps.data count: _textureHeaps.count stages: MTLRenderStageFragment];

		if (_bufferHeaps.count)
			[encoder useHeaps: (id<MTLHeap> *)_bufferHeaps.data count: _bufferHeaps.count stages: MTLRenderStageVertex | MTLRenderStageFragment |
																								  MTLRenderStageObject | MTLRenderStageMesh | MTLRenderStageTile];
	} else {
		if (_textureHeaps.count)
			[encoder useHeaps: (id<MTLHeap> *)_textureHeaps.data count: _textureHeaps.count stages: MTLRenderStageFragment];

		if (_bufferHeaps.count)
			[encoder useHeaps: (id<MTLHeap> *)_bufferHeaps.data count: _bufferHeaps.count stages: MTLRenderStageVertex | MTLRenderStageFragment];
	}
}

void
MTLBk_SetComputeHeaps(id<MTLComputeCommandEncoder> encoder)
{
	if (_textureHeaps.count)
		[encoder useHeaps: (id<MTLHeap> *)_textureHeaps.data count: _textureHeaps.count];

	if (_bufferHeaps.count)
		[encoder useHeaps: (id<MTLHeap> *)_bufferHeaps.data count: _bufferHeaps.count];
}

void
MTLBk_TermMemory(void)
{
	struct HeapInfo *info;

	Rt_ArrayForEach(info, &_textureHeapInfo)
		[info->heap release];
	Rt_TermArray(&_textureHeapInfo);
	Rt_TermArray(&_textureHeaps);

	Rt_ArrayForEach(info, &_bufferHeapInfo)
		[info->heap release];
	Rt_TermArray(&_bufferHeapInfo);
	Rt_TermArray(&_bufferHeaps);
}

/* NekoEngine
 *
 * MTLMemory.m
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
