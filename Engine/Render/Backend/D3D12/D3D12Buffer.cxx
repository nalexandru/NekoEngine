#include <stdlib.h>

#include "D3D12Backend.h"

struct NeBuffer *
Re_BkCreateBuffer(const struct NeBufferDesc *desc, uint16_t location)
{
	struct NeBuffer *buff = (struct NeBuffer *)Sys_Alloc(1, sizeof(*buff), MH_RenderBackend);
	if (!buff)
		return NULL;

	buff->staging = nullptr;
	buff->transient = false;

	/*auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(desc->size);

	if (HRESULT hr = Re_device->dev->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&buff->res)); FAILED(hr)) {
		Sys_LogEntry(D3D12BK_MOD, LOG_CRITICAL, "ID3D12Device10::CreateBuffer returned 0x%x", hr);
		Sys_Free(buff);
		return NULL;
	}*/

	// TODO: buffer properties
	D3D12_RESOURCE_DESC rd = CD3DX12_RESOURCE_DESC::Buffer(desc->size, D3D12_RESOURCE_FLAG_NONE, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
	D3D12_HEAP_PROPERTIES hp{};
	NeMemoryTypeToD3DHeapProps(desc->memoryType, &hp);


	if (HRESULT hr = Re_device->dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd,
									D3D12_RESOURCE_STATE_COPY_SOURCE, NULL, IID_PPV_ARGS(&buff->res)); FAILED(hr)) {
		D3D12BK_LOG_ERR("ID3D12Device10::CreateBuffer", hr);
		Sys_Free(buff);
		return NULL;
	}

	buff->state = D3D12_RESOURCE_STATE_COPY_SOURCE;

#ifdef _DEBUG
	if (desc->name)
		buff->res->SetName(NeWin32_UTF8toUCS2(desc->name));
#endif

	return buff;
}

void
Re_BkUpdateBuffer(struct NeBuffer *buff, uint64_t offset, void *data, uint64_t size)
{
	D3D12_RESOURCE_DESC rd = buff->res->GetDesc();
	D3D12_RESOURCE_ALLOCATION_INFO rai = Re_device->dev->GetResourceAllocationInfo(0, 1, &rd);

	D3D12_HEAP_PROPERTIES hp{ D3D12_HEAP_TYPE_UPLOAD };
	rd = CD3DX12_RESOURCE_DESC::Buffer(size, D3D12_RESOURCE_FLAG_NONE, rai.Alignment);

	ID3D12Resource *staging = nullptr;
	Re_device->dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_PPV_ARGS(&staging));

	Re_device->copyCmdList->Reset(Re_device->copyAllocator, NULL);

	D3D12_SUBRESOURCE_DATA srd = { data, (LONG_PTR)size, (LONG_PTR)size };
	UpdateSubresources(Re_device->copyCmdList, buff->res, staging, offset, 0, 1, &srd);

	Re_device->copyCmdList->Close();

	Re_device->copy->ExecuteCommandLists(1, (ID3D12CommandList **)&Re_device->copyCmdList);
	D3D12Bk_SignalFenceGPU(&Re_device->copyFence, Re_device->copy);

	D3D12Bk_WaitForFenceCPU(&Re_device->copyFence);

	staging->Release();
}

void *
Re_BkMapBuffer(struct NeBuffer *buff)
{
	if (buff->staging)
		return buff->staging;

	buff->res->Map(0, nullptr, &buff->staging);
	return buff->staging;
}

void
Re_BkFlushBuffer(struct NeBuffer *buff, uint64_t offset, uint64_t size)
{
}

void
Re_BkUnmapBuffer(struct NeBuffer *buff)
{
	buff->res->Unmap(0, NULL);
}

uint64_t
Re_BkBufferAddress(const struct NeBuffer *buff, uint64_t offset)
{
	return buff->res->GetGPUVirtualAddress() + offset;
}

uint64_t
Re_OffsetAddress(uint64_t addr, uint64_t offset)
{
	return addr + offset;
}

void
Re_BkDestroyBuffer(struct NeBuffer *buff)
{
	buff->res->Release();
	Sys_Free(buff);
}

/* NekoEngine
 *
 * D3D12Buffer.cxx
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
