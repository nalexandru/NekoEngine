#include <Engine/Config.h>

#include "D3D12Backend.h"

struct NeTexture *
Re_BkCreateTransientTexture(const struct NeTextureDesc *desc, uint16_t location, uint64_t offset, uint64_t *size)
{
	struct NeTexture *tex = (struct NeTexture *)Sys_Alloc(1, sizeof(*tex), MH_Frame);
	if (!tex)
		return NULL;

	tex->transient = true;

	D3D12_RESOURCE_DESC rd = NeToD3DTextureDesc(desc);
	D3D12_RESOURCE_ALLOCATION_INFO rai = Re_device->dev->GetResourceAllocationInfo(0, 1, &rd);

	uint64_t realOffset = NE_ROUND_UP(offset, rai.Alignment);
	if (FAILED(Re_device->dev->CreatePlacedResource(Re_device->transientHeap, realOffset, &rd,
													D3D12_RESOURCE_STATE_RENDER_TARGET, NULL, IID_PPV_ARGS(&tex->res)))) {
		Sys_Free(tex);
		return NULL;
	}

	*size = rai.SizeInBytes + realOffset - offset;

	if (location)
		D3D12Bk_SetTexture(location, tex->res);

#ifdef _DEBUG
	if (desc->name)
		tex->res->SetName(NeWin32_UTF8toUCS2(desc->name));
#endif

	return tex;
}

struct NeBuffer *
Re_BkCreateTransientBuffer(const struct NeBufferDesc *desc, uint16_t location, uint64_t offset, uint64_t *size)
{
	struct NeBuffer *buff = (struct NeBuffer *)Sys_Alloc(1, sizeof(*buff), MH_Frame);
	if (!buff)
		return NULL;

	buff->transient = true;

	D3D12_RESOURCE_DESC rd = CD3DX12_RESOURCE_DESC::Buffer(desc->size);
	D3D12_RESOURCE_ALLOCATION_INFO rai = Re_device->dev->GetResourceAllocationInfo(0, 1, &rd);

	uint64_t realOffset = NE_ROUND_UP(offset, rai.Alignment);
	if (FAILED(Re_device->dev->CreatePlacedResource(Re_device->transientHeap, realOffset, &rd,
													D3D12_RESOURCE_STATE_RENDER_TARGET, NULL, IID_PPV_ARGS(&buff->res)))) {
		Sys_Free(buff);
		return NULL;
	}

	*size = rai.SizeInBytes + realOffset - offset;

#ifdef _DEBUG
	if (desc->name)
		buff->res->SetName(NeWin32_UTF8toUCS2(desc->name));
#endif

	return buff;
}

bool
Re_InitTransientHeap(uint64_t size)
{
	D3D12_HEAP_DESC desc{};/* =
	{
		NE_ROUND_UP(size, D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT),
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_L1,
		0,
		0,
		D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT,
		D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES
	};*/

	desc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
	// desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_L1; This fails for some reason
	desc.Flags = D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES;

	D3D12_FEATURE_DATA_D3D12_OPTIONS4 o4{};
	Re_device->dev->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS4, &o4, sizeof(o4));

	desc.Alignment = o4.MSAA64KBAlignedTextureSupported ? D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT : D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT;
	desc.SizeInBytes = NE_ROUND_UP(size, desc.Alignment);

	if (HRESULT hr = Re_device->dev->CreateHeap(&desc, IID_PPV_ARGS(&Re_device->transientHeap)); FAILED(hr)) {
		Sys_LogEntry(D3D12BK_MOD, LOG_CRITICAL, "ID3D12Device10::CreateHeap returned 0x%x", hr);
		return false;
	}

#ifdef _DEBUG
	Re_device->transientHeap->SetName(L"Transient Memory Heap");
#endif

	return true;
}

bool
Re_ResizeTransientHeap(uint64_t size)
{
	return false;
}

void
Re_TermTransientHeap(void)
{
	Re_device->transientHeap->Release();
}

/* NekoEngine
 *
 * D3D12TransientResources.cxx
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
