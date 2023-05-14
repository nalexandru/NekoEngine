#include <Engine/Config.h>
#include <Runtime/Runtime.h>
#include <System/AtomicLock.h>

#include "D3D12Backend.h"

static inline void SetTextureSRV(ID3D12DescriptorHeap *heap, uint16_t location, ID3D12Resource *res);

bool
D3D12Bk_InitDescriptorHeap(struct NeRenderDevice *dev)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};

	heapDesc.NumDescriptors = UINT16_MAX;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(dev->dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&dev->cpuTextureHeap))))
		return false;

	dev->rtvIncrement = dev->dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	dev->dsvIncrement = dev->dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	dev->uavIncrement  = dev->dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	for (uint32_t i = 0; i < _countof(dev->gpuDescriptorHeap); ++i)
		if (HRESULT hr = dev->dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&dev->gpuDescriptorHeap[i])); FAILED(hr)) {
			D3D12BK_LOG_ERR("ID3D12Device10::CreateDescriptorHeap", hr);
			return false;
		}

	/*VkDescriptorSetLayoutBinding bindings[] =
		{
			{
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
				.descriptorCount = 3,
				.stageFlags = VK_SHADER_STAGE_ALL,
			},
			{
				.binding = 1,
				.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
				.descriptorCount = UINT16_MAX,
				.stageFlags = VK_SHADER_STAGE_ALL,
			},
		};
	VkDescriptorBindingFlags bindingFlags[] =
		{
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
		};
	VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
			.bindingCount = sizeof(bindingFlags) / sizeof(bindingFlags[0]),
			.pBindingFlags = bindingFlags
		};
	VkDescriptorSetLayoutCreateInfo dslInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
			.pNext = &flagsInfo,
			.bindingCount = sizeof(bindings) / sizeof(bindings[0]),
			.pBindings = bindings
		};
	if (vkCreateDescriptorSetLayout(dev->dev, &dslInfo, Vkd_allocCb, &dev->setLayout) != VK_SUCCESS)
		return false;

	VkDescriptorPoolSize poolSize[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 3 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, UINT16_MAX }
		};
	VkDescriptorPoolCreateInfo poolInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
			.maxSets = 1,
			.poolSizeCount = sizeof(poolSize) / sizeof(poolSize[0]),
			.pPoolSizes = poolSize
		};
	if (vkCreateDescriptorPool(dev->dev, &poolInfo, Vkd_allocCb, &dev->descriptorPool) != VK_SUCCESS)
		goto error;

	VkDescriptorSetAllocateInfo allocInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = dev->descriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &dev->setLayout
		};
	if (vkAllocateDescriptorSets(dev->dev, &allocInfo, &dev->descriptorSet) != VK_SUCCESS)
		goto error;

	// Input attachment set layout & pool
	VkDescriptorSetLayoutBinding iaBindings[] =
		{
			{
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
				.descriptorCount = 4,
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
			}
		};
	VkDescriptorBindingFlags iaBindingFlags[] = { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT };
	VkDescriptorSetLayoutBindingFlagsCreateInfo iaFlagsInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
			.bindingCount = sizeof(iaBindingFlags) / sizeof(iaBindingFlags[0]),
			.pBindingFlags = iaBindingFlags
		};
	VkDescriptorSetLayoutCreateInfo iaDslInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pNext = &iaFlagsInfo,
			.bindingCount = sizeof(iaBindings) / sizeof(iaBindings[0]),
			.pBindings = iaBindings
		};
	if (vkCreateDescriptorSetLayout(dev->dev, &iaDslInfo, Vkd_allocCb, &dev->iaSetLayout) != VK_SUCCESS)
		goto error;

	uint32_t inputAttachmentPoolSize = E_GetCVarU32("Vulkan_InputAttachmentPoolSize", 32)->i32;
	VkDescriptorPoolSize iaPoolSize[] =
		{
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, inputAttachmentPoolSize }
		};
	VkDescriptorPoolCreateInfo iaPoolInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = inputAttachmentPoolSize,
			.poolSizeCount = sizeof(iaPoolSize) / sizeof(iaPoolSize[0]),
			.pPoolSizes = iaPoolSize
		};
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		if (vkCreateDescriptorPool(dev->dev, &iaPoolInfo, Vkd_allocCb, &dev->iaDescriptorPool[i]) != VK_SUCCESS)
			goto error;

#ifdef _DEBUG
	Vkd_SetObjectName(dev->dev, dev->descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "Global Descriptor Pool");
	Vkd_SetObjectName(dev->dev, dev->descriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET, "Global Descriptor Set");
	Vkd_SetObjectName(dev->dev, dev->setLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Global Descriptor Set Layout");
	Vkd_SetObjectName(dev->dev, dev->iaSetLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Input Attachment Descriptor Set Layout");

	char *tmp = Sys_Alloc(sizeof(*tmp), 64, MH_Transient);
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		snprintf(tmp, 64, "Input Attachment Descriptor Pool %u", i);
		Vkd_SetObjectName(dev->dev, dev->iaDescriptorPool[i], VK_OBJECT_TYPE_DESCRIPTOR_POOL, tmp);
	}
#endif

	return true;

	error:
	if (dev->setLayout)
		vkDestroyDescriptorSetLayout(dev->dev, dev->setLayout, Vkd_allocCb);

	if (dev->iaSetLayout)
		vkDestroyDescriptorSetLayout(dev->dev, dev->iaSetLayout, Vkd_allocCb);

	if (dev->descriptorPool)
		vkDestroyDescriptorPool(dev->dev, dev->descriptorPool, Vkd_allocCb);

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		if (dev->iaDescriptorPool[i])
			vkDestroyDescriptorPool(dev->dev, dev->iaDescriptorPool[i], Vkd_allocCb);
*/
	return true;
}

/*VkDescriptorSet
Vk_AllocateIADescriptorSet(struct NeRenderDevice *dev)
{
	VkDescriptorSet ds = VK_NULL_HANDLE;
	VkDescriptorSetAllocateInfo allocInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = dev->iaDescriptorPool[Re_frameId],
			.descriptorSetCount = 1,
			.pSetLayouts = &dev->iaSetLayout
		};
	if (vkAllocateDescriptorSets(dev->dev, &allocInfo, &ds) != VK_SUCCESS)
		return VK_NULL_HANDLE;

#ifdef _DEBUG
	Vkd_SetObjectName(dev->dev, ds, VK_OBJECT_TYPE_DESCRIPTOR_SET, "Input Attachment Descriptor Set");
#endif

	return ds;
}*/

void
D3D12Bk_SetSampler(struct NeRenderDevice *dev, uint16_t location, ID3D12Resource *res)
{
/*	VkDescriptorImageInfo dii =
		{
			.sampler = sampler
		};
	VkWriteDescriptorSet wds =
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = dev->descriptorSet,
			.dstBinding = 0,
			.dstArrayElement = location,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
			.pImageInfo = &dii
		};
	vkUpdateDescriptorSets(dev->dev, 1, &wds, 0, NULL);*/
}

void
D3D12Bk_SetTexture(uint16_t location, ID3D12Resource *res) { SetTextureSRV(Re_device->cpuTextureHeap, location, res); }

void
D3D12Bk_SetInputAttachment(ID3D12DescriptorHeap *heap, uint16_t location, ID3D12Resource *res) { SetTextureSRV(heap, location, res); }

void
D3D12_CopyTextureHeap(struct NeRenderDevice *dev, D3D12_CPU_DESCRIPTOR_HANDLE dst)
{
/*	CD3DX12_CPU_DESCRIPTOR_HANDLE handle{ dev->cpuDescriptorHeap->GetCPUDescriptorHandleForHeapStart() };
	for (uint32_t i = 0; i < UINT16_MAX; ++i) {

	}*/
}

void
D3D12Bk_TermDescriptorSet(struct NeRenderDevice *dev)
{
	/*vkDestroyDescriptorPool(dev->dev, dev->descriptorPool, Vkd_allocCb);
	vkDestroyDescriptorSetLayout(dev->dev, dev->setLayout, Vkd_allocCb);

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		vkDestroyDescriptorPool(dev->dev, dev->iaDescriptorPool[i], Vkd_allocCb);
	vkDestroyDescriptorSetLayout(dev->dev, dev->iaSetLayout, Vkd_allocCb);*/


}

void
SetTextureSRV(ID3D12DescriptorHeap *heap, uint16_t location, ID3D12Resource *res)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE handle(heap->GetCPUDescriptorHandleForHeapStart(), location);

	D3D12_RESOURCE_DESC rd = res->GetDesc();
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc
	{
		.Format = rd.Format,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
	};

	switch (rd.Dimension) {
	case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
	break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		if (rd.DepthOrArraySize == 6) {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = rd.MipLevels;
		} else if (rd.SampleDesc.Count > 1) {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
		} else {
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = rd.MipLevels;
		}
		break;
	case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		srvDesc.Texture3D.MipLevels = rd.MipLevels;
	break;
	}

	Re_device->dev->CreateShaderResourceView(res, &srvDesc, handle);
	D3D12Bk_LogDXGIMessages();
}


/* NekoEngine
 *
 * D3D12DescriptorHeap.cxx
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
