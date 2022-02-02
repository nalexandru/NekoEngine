#include <Engine/Config.h>

#include "D3D12Driver.h"

struct NeTexture *
D3D12_CreateTransientTexture(struct NeRenderDevice *dev, const struct NeTextureDesc *desc, uint16_t location, uint64_t offset, uint64_t *size)
{
	struct NeTexture *tex = Sys_Alloc(1, sizeof(*tex), MH_Frame);
	if (!tex)
		return NULL;

	D3D12_RESOURCE_DESC resDesc;
	D3D12_HEAP_PROPERTIES heapProperties;
	D3D12D_InitTextureDesc(desc, &heapProperties, &resDesc);

	HRESULT hr = ID3D12Device5_CreatePlacedResource(dev->dev, dev->transientHeap, offset, &resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, &tex->res);

	if (FAILED(hr)) {
		return NULL;
	}

	/*if (location)
			D3D12_SetTexture(dev, location, tex->imageView);*/

	return tex;
}

struct NeBuffer *
D3D12_CreateTransientBuffer(struct NeRenderDevice *dev, const struct NeBufferDesc *desc, uint16_t location, uint64_t offset, uint64_t *size)
{
	struct NeBuffer *buff = Sys_Alloc(1, sizeof(*buff), MH_Frame);
	if (!buff)
		return NULL;

	D3D12_RESOURCE_DESC resDesc;
	D3D12_HEAP_PROPERTIES heapProperties;
	D3D12D_InitBufferDesc(desc, &heapProperties, &resDesc);

	HRESULT hr = ID3D12Device5_CreateCommittedResource(dev->dev, &heapProperties, D3D12_HEAP_FLAG_NONE,
		&resDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, &buff->res);

	if (FAILED(hr)) {
		return NULL;
	}

/*	if (location)
		D3D12_SetBuffer(dev, location, buff->buff);*/

	return buff;
}

bool
D3D12_InitTransientHeap(struct NeRenderDevice *dev, uint64_t size)
{
	D3D12_HEAP_DESC desc =
	{
		.SizeInBytes = size,
		.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		.Flags = D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
		.Properties.Type = D3D12_HEAP_TYPE_DEFAULT,
		.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE,
		.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_L1,
		.Properties.CreationNodeMask = 1,
		.Properties.VisibleNodeMask = 1
	};
	return SUCCEEDED(ID3D12Device5_CreateHeap(dev->dev, &desc, &IID_ID3D12Heap, &dev->transientHeap));
}

bool
D3D12_ResizeTransientHeap(struct NeRenderDevice *dev, uint64_t size)
{
	return false;
}

void
D3D12_TermTransientHeap(struct NeRenderDevice *dev)
{
	ID3D12Heap_Release(dev->transientHeap);
}
