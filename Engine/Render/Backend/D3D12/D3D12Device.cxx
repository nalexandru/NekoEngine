#include <stdlib.h>

#include <System/Log.h>
#include <System/Memory.h>
#include <Engine/Config.h>

#include "D3D12Backend.h"

struct NeRenderDevice *
Re_CreateDevice(struct NeRenderDeviceInfo *info)
{
	UINT id = (UINT)info->reserved;
	NeRenderDevice *dev = (NeRenderDevice *)Sys_Alloc(1, sizeof(*dev), MH_RenderBackend);

	if (!dev)
		return NULL;

	if (FAILED(D3D12_factory->EnumAdapterByGpuPreference(id, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&dev->adapter)))) {
		Sys_Free(dev);
		return NULL;
	}

	if (HRESULT hr = D3D12CreateDevice(dev->adapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&dev->dev)); FAILED(hr)) {
		Sys_Free(dev);
		return NULL;
	}

/*	Sys_LogEntry(D3D12DRV_MOD, LOG_INFORMATION, "Device API version %d.%d.%d",
				 VK_VERSION_MAJOR(dev->physDevProps.apiVersion),
				 VK_VERSION_MINOR(dev->physDevProps.apiVersion),
				 VK_VERSION_PATCH(dev->physDevProps.apiVersion));*/

	D3D12_COMMAND_QUEUE_DESC cqd
	{
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		D3D12_COMMAND_QUEUE_PRIORITY_HIGH,
		D3D12_COMMAND_QUEUE_FLAG_NONE,
		0
	};
	if (HRESULT hr = dev->dev->CreateCommandQueue(&cqd, IID_PPV_ARGS(&dev->direct)); FAILED(hr)) {
		D3D12Bk_LogDXGIMessages();
		goto error;
	}
	if (HRESULT hr = dev->direct->SetName(L"Graphics Queue"); FAILED(hr)) {
		D3D12Bk_LogDXGIMessages();
	}

	cqd.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	cqd.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	if (HRESULT hr = dev->dev->CreateCommandQueue(&cqd, IID_PPV_ARGS(&dev->compute)); FAILED(hr)) {
		D3D12Bk_LogDXGIMessages();
		goto error;
	}
	if (HRESULT hr = dev->compute->SetName(L"Compute Queue"); FAILED(hr)) {
		D3D12Bk_LogDXGIMessages();
	}

	cqd.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	cqd.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	if (HRESULT hr = dev->dev->CreateCommandQueue(&cqd, IID_PPV_ARGS(&dev->copy)); FAILED(hr)) {
		D3D12Bk_LogDXGIMessages();
		goto error;
	}
	if (HRESULT hr = dev->copy->SetName(L"Copy Queue"); FAILED(hr)) {
		D3D12Bk_LogDXGIMessages();
	}

	D3D12Bk_LogDXGIMessages();
	if (HRESULT hr = dev->dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&dev->copyAllocator)); FAILED(hr)) {
		D3D12Bk_LogDXGIMessages();
		goto error;
	}
	if (HRESULT hr = dev->copyAllocator->SetName(L"Global Copy Allocator"); FAILED(hr)) {
		D3D12Bk_LogDXGIMessages();
	}

	/*

	VkSemaphoreTypeCreateInfo typeInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = 0
		};
	VkSemaphoreCreateInfo semInfo =
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &typeInfo
		};
	vkCreateSemaphore(dev->dev, &semInfo, Vkd_allocCb, &dev->frameSemaphore);

	dev->semaphoreValue = 0;
	dev->frameValues = Sys_Alloc(RE_NUM_FRAMES, sizeof(*dev->frameValues), MH_RenderBackend);

	*/

	if (!D3D12Bk_InitDescriptorHeap(dev))
		goto error;

	if (!D3D12Bk_LoadShaders())
		goto error;

	/*VkFenceCreateInfo fenceInfo =
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
		};
	if (vkCreateFence(dev->dev, &fenceInfo, Vkd_allocCb, &dev->driverTransferFence) != VK_SUCCESS)
		goto error;
	*/

	if (CVAR_BOOL("Direct3D12_EnableDebug")) {
		dev->dev->QueryInterface(IID_PPV_ARGS(&dev->debugDevice));
		if (dev->debugDevice && CVAR_BOOL("Direct3D12_Windows7Emulation")) {
			D3D12_DEBUG_FEATURE f{ D3D12_DEBUG_FEATURE_EMULATE_WINDOWS7 };
			dev->debugDevice->SetDebugParameter(D3D12_DEBUG_DEVICE_PARAMETER_FEATURE_FLAGS, &f, sizeof(f));
		}
	}

#ifdef _DEBUG
/*
	Vkd_SetObjectName(dev->dev, dev->frameSemaphore, VK_OBJECT_TYPE_SEMAPHORE, "Frame Semaphore");

	Vkd_SetObjectName(dev->dev, dev->driverTransferFence, VK_OBJECT_TYPE_FENCE, "Driver Transfer Fence");*/
#endif

	return dev;

error:
	if (dev) {
		if (dev->adapter)
			dev->adapter->Release();
	}

	Sys_Free(dev);

	return NULL;
}

void
Re_WaitIdle(void)
{
	//vkDeviceWaitIdle(Re_device->dev);
}

void
Re_DestroyDevice(struct NeRenderDevice *dev)
{
	D3D12Bk_TermDescriptorSet(dev);
	D3D12Bk_UnloadShaders();

	dev->copyAllocator->Release();

	dev->copy->Release();
	dev->compute->Release();
	dev->direct->Release();

	/*vkDestroyFence(dev->dev, dev->driverTransferFence, Vkd_allocCb);
	vkDestroySemaphore(dev->dev, dev->frameSemaphore, Vkd_allocCb);*/

	if (dev->debugDevice)
		dev->debugDevice->Release();

	dev->dev->Release();
	dev->adapter->Release();

	Sys_Free(dev->frameValues);
	Sys_Free(dev);
}

/*static uint64_t IFace_FrameSemaphoreValue(struct NeRenderDevice *dev) { return dev->frameValues[Re_frameId]; }
static VkCommandBuffer IFace_CurrentCommandBuffer(struct NeRenderContext *ctx) { return ctx->cmdBuffer; }
static VkSemaphore IFace_SemaphoreHandle(struct NeSemaphore *sem) { return sem->sem; }
static uint64_t IFace_CurrentSemaphoreValue(struct NeSemaphore *sem) { return sem->value; }
static VkImage IFace_Image(struct NeTexture *tex) { return tex->image; }
static VkImageView IFace_ImageView(struct NeTexture *tex) { return tex->imageView; }
static VkBuffer IFace_Buffer(struct NeBuffer *buff) { return buff->buff; }
static VkAccelerationStructureKHR IFace_AccelerationStructure(struct NeAccelerationStructure *as) { return as->as; }
static VkFramebuffer IFace_Framebuffer(struct NeFramebuffer *fb) { return fb->fb; }
static VkRenderPass IFace_RenderPass(struct NeRenderPassDesc *rp) { return rp->rp; }
static VkSampler IFace_Sampler(struct NeSampler *s) { return (VkSampler)s; }
static VkPipeline IFace_Pipeline(struct NePipeline *p) { return p->pipeline; }

struct NeRenderInterface *
Re_CreateRenderInterface(void)
{
	struct NeRenderInterface *iface = Sys_Alloc(sizeof(*iface), 1, MH_RenderBackend);
	if (!iface)
		return NULL;

	iface->CurrentCommandBuffer = IFace_CurrentCommandBuffer;

	iface->FrameSemaphoreValue = IFace_FrameSemaphoreValue;
	iface->frameSemaphore = Re_device->frameSemaphore;

	iface->SemaphoreHandle = IFace_SemaphoreHandle;
	iface->CurrentSemaphoreValue = IFace_CurrentSemaphoreValue;
	iface->Image = IFace_Image;
	iface->ImageView = IFace_ImageView;
	iface->Buffer = IFace_Buffer;
	iface->Framebuffer = IFace_Framebuffer;
	iface->Sampler = IFace_Sampler;
	iface->RenderPass = IFace_RenderPass;
	iface->AccelerationStructure = IFace_AccelerationStructure;
	iface->Pipeline = IFace_Pipeline;

	iface->device = Re_device->dev;
	iface->graphicsQueue = Re_device->graphics.queue;
	iface->transferQueue = Re_device->transfer.queue;
	iface->computeQueue = Re_device->compute.queue;
	iface->graphicsFamily = Re_device->graphics.family;
	iface->transferFamily = Re_device->transfer.family;
	iface->computeFamily = Re_device->compute.family;
	iface->physicalDevice = Re_device->physDev;
	iface->pipelineCache = Vkd_pipelineCache;
	iface->allocationCallbacks = Vkd_allocCb;

	iface->instance = Vkd_inst;
	iface->GetInstanceProcAddr = vkGetInstanceProcAddr;

	return iface;
}

void
Re_DestroyRenderInterface(struct NeRenderInterface *iface)
{
	Sys_Free(iface);
}*/

/* NekoEngine
 *
 * D3D12Device.cxx
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
