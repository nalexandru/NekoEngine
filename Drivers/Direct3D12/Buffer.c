#include <stdlib.h>

#include "D3D12Driver.h"

void
D3D12D_InitBufferDesc(const struct NeBufferDesc *desc, D3D12_HEAP_PROPERTIES *heapProperties, D3D12_RESOURCE_DESC *resDesc)
{
	heapProperties->MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties->CreationNodeMask = 1;
	heapProperties->VisibleNodeMask = 1;

	resDesc->Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc->Alignment = 0;
	resDesc->Width = desc->size;
	resDesc->Height = 1;
	resDesc->DepthOrArraySize = 1;
	resDesc->MipLevels = 1;
	resDesc->Format = DXGI_FORMAT_UNKNOWN;
	resDesc->SampleDesc.Count = 1;
	resDesc->SampleDesc.Quality = 0;
	resDesc->Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc->Flags = D3D12_RESOURCE_FLAG_NONE;

	switch (desc->memoryType) {
	case MT_GPU_LOCAL:
		heapProperties->Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties->CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
	break;
	case MT_CPU_READ:
		heapProperties->Type = D3D12_HEAP_TYPE_READBACK;
		heapProperties->CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	break;
	case MT_CPU_WRITE:
		heapProperties->Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProperties->CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
	break;
	case MT_CPU_COHERENT:
		heapProperties->Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProperties->CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	break;
	}
}

struct NeBuffer *
D3D12_CreateBuffer(struct NeRenderDevice *dev, const struct NeBufferDesc *desc, uint16_t location)
{
	struct NeBuffer *buff = Sys_Alloc(1, sizeof(*buff), MH_RenderDriver);
	if (!buff)
		return NULL;

	D3D12_RESOURCE_DESC resDesc;
	D3D12_HEAP_PROPERTIES heapProperties;
	D3D12D_InitBufferDesc(desc, &heapProperties, &resDesc);

	HRESULT hr = ID3D12Device5_CreateCommittedResource(dev->dev, &heapProperties, D3D12_HEAP_FLAG_NONE,
		&resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, &buff->res);

	if (FAILED(hr)) {
		Sys_Free(buff);
		return NULL;
	}

//	D3D12_SetBuffer(dev, location, buff->buff);

	return buff;
}

void
D3D12_UpdateBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff, uint64_t offset, void *data, uint64_t size)
{
	D3D12_HEAP_PROPERTIES heapProperties =
	{
		.Type = D3D12_HEAP_TYPE_UPLOAD,
		.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
		.CreationNodeMask = 1,
		.VisibleNodeMask = 1
	};
	D3D12_RESOURCE_DESC desc =
	{
		.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
		.Alignment = 0,
		.Width = size,
		.Height = 1,
		.DepthOrArraySize = 1,
		.MipLevels = 1,
		.Format = DXGI_FORMAT_UNKNOWN,
		.SampleDesc.Count = 1,
		.SampleDesc.Quality = 0,
		.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		.Flags = D3D12_RESOURCE_FLAG_NONE
	};

	ID3D12Resource *upload;
	ID3D12Device5_CreateCommittedResource(dev->dev, &heapProperties, D3D12_HEAP_FLAG_NONE,
		&desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, &upload);

	uint32_t numRows = 0;
	uint64_t reqSize = 0, rowSize = 0;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout = { 0 };

	D3D12_RESOURCE_DESC dstDesc = { 0 };
	ID3D12Resource_GetDesc(buff->res, &dstDesc);

	ID3D12Device5_GetCopyableFootprints(dev->dev, &dstDesc, 0, 1, 0, &layout, &numRows, &rowSize, &reqSize);

	void *ptr;
	ID3D12Resource_Map(upload, 0, NULL, &ptr);
	// check HRESULT

	memcpy(ptr, data, size);
	ID3D12Resource_Unmap(upload, 0, NULL);

	ID3D12GraphicsCommandList4 *cmdList = D3D12D_TransferCmdList(dev);
	ID3D12GraphicsCommandList4_Reset(cmdList, dev->driverCopyAllocator, NULL);

	ID3D12GraphicsCommandList4_CopyBufferRegion(cmdList, buff->res, offset, upload, layout.Offset, layout.Footprint.Width);

	ID3D12GraphicsCommandList4_Close(cmdList);
	ID3D12CommandQueue_ExecuteCommandLists(dev->copyQueue, 1, (ID3D12CommandList **)&cmdList);

	// wait for cmd list exec
}

void *
D3D12_MapBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff)
{
	void *mem;
	ID3D12Resource_Map(buff->res, 0, NULL, &mem);
	return mem;
}

void
D3D12_UnmapBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff)
{
	ID3D12Resource_Unmap(buff->res, 0, NULL);
}

void
D3D12_FlushBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff, uint64_t offset, uint64_t size)
{
	// TODO
}

uint64_t
D3D12_BufferAddress(struct NeRenderDevice *dev, const struct NeBuffer *buff, uint64_t offset)
{
	return ID3D12Resource_GetGPUVirtualAddress(buff->res) + offset;
}

uint64_t
D3D12_OffsetAddress(uint64_t addr, uint64_t offset)
{
	return addr + offset;
}

void
D3D12_DestroyBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff)
{
	ID3D12Resource_Release(buff->res);
	Sys_Free(buff);
}
