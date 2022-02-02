#include <stdlib.h>

#include "D3D12Driver.h"

void
D3D12D_InitTextureDesc(const struct NeTextureDesc *desc, D3D12_HEAP_PROPERTIES *heapProperties, D3D12_RESOURCE_DESC *resDesc)
{
	ZeroMemory(heapProperties, sizeof(*heapProperties));
	ZeroMemory(resDesc, sizeof(*resDesc));

	heapProperties->MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties->CreationNodeMask = 1;
	heapProperties->VisibleNodeMask = 1;

	resDesc->Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc->Alignment = 0;
	resDesc->Width = desc->width;
	resDesc->Height = desc->height;
	resDesc->DepthOrArraySize = desc->depth;
	resDesc->MipLevels = desc->mipLevels;
	resDesc->Format = DXGI_FORMAT_UNKNOWN;
	resDesc->SampleDesc.Count = 1;
	resDesc->SampleDesc.Quality = 0;
	resDesc->Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc->Flags = D3D12_RESOURCE_FLAG_NONE;

	switch (desc->type) {
	case TT_2D: resDesc->Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; break;
	case TT_2D_Multisample:
		resDesc->Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	break;
	case TT_Cube:
		resDesc->Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resDesc->DepthOrArraySize = desc->arrayLayers;
	break;
	case TT_3D: resDesc->Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D; break;
	}

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

struct NeTexture *
D3D12_CreateTexture(struct NeRenderDevice *dev, const struct NeTextureDesc *desc, uint16_t location)
{
	struct NeTexture *tex = Sys_Alloc(1, sizeof(*tex), MH_RenderDriver);
	if (!tex)
		return NULL;

	D3D12_RESOURCE_DESC resDesc;
	D3D12_HEAP_PROPERTIES heapProperties;
	D3D12D_InitTextureDesc(desc, &heapProperties, &resDesc);

	HRESULT hr = ID3D12Device5_CreateCommittedResource(dev->dev, &heapProperties, D3D12_HEAP_FLAG_NONE,
		&resDesc, D3D12_RESOURCE_STATE_COPY_DEST, NULL, &IID_ID3D12Resource, &tex->res);

	if (FAILED(hr)) {
		Sys_Free(tex);
		return NULL;
	}

//	D3D12_SetTexture(dev, location, tex->res);

	return tex;
}

enum NeTextureLayout
D3D12_TextureLayout(const struct NeTexture *tex)
{
	return 0;// VkToNeImageLayout(tex->layout);
}

void
D3D12_DestroyTexture(struct NeRenderDevice *dev, struct NeTexture *tex)
{
	ID3D12Resource_Release(tex->res);

	Sys_Free(tex);
}
