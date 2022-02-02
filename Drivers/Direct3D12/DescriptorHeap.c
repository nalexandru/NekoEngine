#include "D3D12Driver.h"

bool
D3D12_InitDescriptorHeap(struct NeRenderDevice *dev)
{
	HRESULT hr;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc =
	{
		.NumDescriptors = UINT16_MAX,
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
	};
	hr = ID3D12Device5_CreateDescriptorHeap(dev->dev, &heapDesc, &IID_ID3D12DescriptorHeap, &dev->cpuDescriptorHeap);

	dev->heapIncrement = ID3D12Device5_GetDescriptorHandleIncrementSize(dev->dev, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc =
	{
		.NumDescriptors = 3,
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
	};
	hr = ID3D12Device5_CreateDescriptorHeap(dev->dev, &samplerHeapDesc, &IID_ID3D12DescriptorHeap, &dev->cpuSamplerDescriptorHeap);

	dev->samplerHeapIncrement = ID3D12Device5_GetDescriptorHandleIncrementSize(dev->dev, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		hr = ID3D12Device5_CreateDescriptorHeap(dev->dev, &heapDesc, &IID_ID3D12DescriptorHeap, &dev->descriptorHeap[i]);
		hr = ID3D12Device5_CreateDescriptorHeap(dev->dev, &samplerHeapDesc, &IID_ID3D12DescriptorHeap, &dev->samplerDescriptorHeap[i]);
	}

	return true;
}

void
D3D12_SetBuffer(struct NeRenderDevice *dev, uint16_t location, ID3D12Resource *res)
{
}

void
D3D12_SetSampler(struct NeRenderDevice *dev, uint16_t location, ID3D12Resource *res)
{
}

void
D3D12_SetTexture(struct NeRenderDevice *dev, uint16_t location, ID3D12Resource *res)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(dev->cpuDescriptorHeap, &handle);

	D3D12_SHADER_RESOURCE_VIEW_DESC desc =
	{
		.Format = DXGI_FORMAT_UNKNOWN,
		.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Texture2D.MipLevels = 1
	};

	handle.ptr += (uint64_t)location * dev->heapIncrement;
	ID3D12Device5_CreateShaderResourceView(dev->dev, res, &desc, handle);
}

void
D3D12_TermDescriptorHeap(struct NeRenderDevice *dev)
{
	ID3D12DescriptorHeap_Release(dev->cpuDescriptorHeap);
	ID3D12DescriptorHeap_Release(dev->cpuSamplerDescriptorHeap);

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		ID3D12DescriptorHeap_Release(dev->descriptorHeap[i]);
		ID3D12DescriptorHeap_Release(dev->samplerDescriptorHeap[i]);
	}
}