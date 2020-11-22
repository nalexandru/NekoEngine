#include <assert.h>

#include <Engine/Resource.h>
#include <Render/Device.h>
#include <Render/Texture.h>
#include <System/Memory.h>

#include "D3D12Render.h"

static ID3D12DescriptorHeap *_textureHeap{ 0 };
static bool _heapDirty{ true };
static UINT _incrementSize;
static uint32_t _maxTextures = 1024, _freeSlots = 0;
static ID3D12Resource **_textures{ 0 };

static DXGI_FORMAT _textureFormat[] =
{
	DXGI_FORMAT_R8G8B8A8_UNORM,
	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
	DXGI_FORMAT_B8G8R8A8_UNORM,
	DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
	DXGI_FORMAT_B8G8R8X8_UNORM,
	DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
	DXGI_FORMAT_R16G16B16A16_FLOAT,
	DXGI_FORMAT_R16G16B16A16_UNORM,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_UINT,
	DXGI_FORMAT_R10G10B10A2_UNORM,
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R32G32B32_UINT,
	DXGI_FORMAT_R8G8_UNORM,
	DXGI_FORMAT_R16G16_FLOAT,
	DXGI_FORMAT_R16G16_UNORM,
	DXGI_FORMAT_R32G32_FLOAT,
	DXGI_FORMAT_R32G32_UINT,
	DXGI_FORMAT_R8_UNORM,
	DXGI_FORMAT_R16_FLOAT,
	DXGI_FORMAT_R16_UNORM,
	DXGI_FORMAT_R32_FLOAT,
	DXGI_FORMAT_R32_UINT,
	DXGI_FORMAT_BC1_UNORM,
	DXGI_FORMAT_BC1_UNORM_SRGB,
	DXGI_FORMAT_BC2_UNORM,
	DXGI_FORMAT_BC2_UNORM_SRGB,
	DXGI_FORMAT_BC3_UNORM,
	DXGI_FORMAT_BC3_UNORM_SRGB,
	DXGI_FORMAT_BC4_UNORM,
	DXGI_FORMAT_BC4_SNORM,
	DXGI_FORMAT_BC5_UNORM,
	DXGI_FORMAT_BC5_SNORM,
	DXGI_FORMAT_BC6H_UF16,
	DXGI_FORMAT_BC6H_SF16,
	DXGI_FORMAT_BC7_UNORM,
	DXGI_FORMAT_BC7_UNORM_SRGB,
	DXGI_FORMAT_R8_UNORM
};

bool
D3D12_InitTexture(const char *name, struct Texture *tex, Handle h)
{
	struct TextureRenderData *trd = (struct TextureRenderData *)&tex->renderDataStart;

	if (!_freeSlots)
		return false;

	if (tex->format >= _countof(_textureFormat))
		return false;

	trd->id = h;
	trd->format = _textureFormat[tex->format];

	D3D12_RESOURCE_DESC rd{};

	switch (tex->type) {
	case TT_2D: rd = CD3DX12_RESOURCE_DESC::Tex2D(trd->format, tex->width, tex->height, 1, tex->levels); break;
	case TT_3D: rd = CD3DX12_RESOURCE_DESC::Tex3D(trd->format, tex->width, tex->height, tex->depth, tex->levels); break;
	case TT_Cube: rd = CD3DX12_RESOURCE_DESC::Tex2D(trd->format, tex->width, tex->height, 6, tex->levels); break;
	}
	
	UINT64 bpr;
	Re_Device.dev->GetCopyableFootprints(&rd, 0, 1, 0, NULL, NULL, &bpr, NULL);

	D3D12_HEAP_PROPERTIES hp{ D3D12_HEAP_TYPE_DEFAULT };
	HRESULT hr = Re_Device.dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_PPV_ARGS(&trd->res));

	if (FAILED(hr))
		return false;

	_textures[E_ResHandleToGPU(h)] = trd->res;
	--_freeSlots;

	size_t len = strlen(name);
	wchar_t *wcsName = (wchar_t *)Sys_Alloc(sizeof(wchar_t), len + 1, MH_Transient);
	(void)mbstowcs(wcsName, name, len);
	trd->res->SetName(wcsName);

	D3D12_StageUpload(trd->res, 0 /* not used */, tex->data, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT, bpr, bpr * tex->height);

	_heapDirty = true;

	return true;
}

bool
D3D12_UpdateTexture(struct Texture *tex, const void *data, uint64_t dataSize, uint64_t offset)
{
	return false;
}

void
D3D12_TermTexture(struct Texture *tex)
{
	struct TextureRenderData *trd = (struct TextureRenderData *)&tex->renderDataStart;

	for (uint32_t i = 0; i < _maxTextures; ++i) {
		if (_textures[i] != trd->res)
			continue;

		_textures[i] = NULL;
		break;
	}

	D3D12_DestroyResource(trd->res);
}

bool
D3D12_InitTextureHeap(void)
{
	D3D12_DESCRIPTOR_HEAP_DESC textureHeapDesc{};
	textureHeapDesc.NumDescriptors = RE_NUM_BUFFERS * 1024;
	textureHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	textureHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(Re_Device.dev->CreateDescriptorHeap(&textureHeapDesc, IID_PPV_ARGS(&_textureHeap))))
		return false;

	D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc{};
	samplerHeapDesc.NumDescriptors = _maxTextures;
	samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(Re_Device.dev->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&Re_GlobalRenderData.samplerHeap))))
		return false;

	_incrementSize = Re_Device.dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	Re_GlobalRenderData.textureHeapSize = _incrementSize * textureHeapDesc.NumDescriptors;

	_textures = (ID3D12Resource **)calloc(_maxTextures, sizeof(ID3D12Resource *));
	if (!_textures)
		return false;

	_freeSlots = _maxTextures;

	return true;
}

void
D3D12_UpdateTextureHeap(D3D12_CPU_DESCRIPTOR_HANDLE dest)
{
	if (_heapDirty) {
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(_textureHeap->GetCPUDescriptorHandleForHeapStart());
	
		for (uint32_t i = 0; i < _maxTextures; ++i) {
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Texture2D.MipLevels = 1;

			if (_textures[i])
				Re_Device.dev->CreateShaderResourceView(_textures[i], &srvDesc, handle);
		
			handle.ptr += _incrementSize;
		}

		_heapDirty = false;
	}

	Re_Device.dev->CopyDescriptorsSimple(_maxTextures, dest, _textureHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void
D3D12_CopyTextureDescriptor(uint64_t id, D3D12_CPU_DESCRIPTOR_HANDLE dest)
{
	D3D12_CPU_DESCRIPTOR_HANDLE src = _textureHeap->GetCPUDescriptorHandleForHeapStart();
	src.ptr += id * _incrementSize;
	Re_Device.dev->CopyDescriptorsSimple(1, dest, src, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void
D3D12_TermTextureHeap(void)
{
	free(_textures);
	_textureHeap->Release();
	Re_GlobalRenderData.samplerHeap->Release();
}
