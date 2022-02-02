#include <stdlib.h>

#include <System/Log.h>
#include <System/Memory.h>
#include <Engine/Config.h>

#include "OpenGLDriver.h"

struct NeRenderDevice *
GL_CreateDevice(struct NeRenderDeviceInfo *info, struct NeRenderDeviceProcs *devProcs, struct NeRenderContextProcs *ctxProcs)
{
	struct NeRenderDevice *dev = Sys_Alloc(1, sizeof(*dev), MH_RenderDriver);

	if (!dev)
		return NULL;

	devProcs->GraphicsPipeline = GL_GraphicsPipeline;
	devProcs->ComputePipeline = GL_ComputePipeline;
	devProcs->RayTracingPipeline = GL_RayTracingPipeline;
	devProcs->DestroyPipeline = GL_DestroyPipeline;

	devProcs->AcquireNextImage = GL_AcquireNextImage;
	devProcs->Present = GL_Present;
	devProcs->SwapchainFormat = GL_SwapchainFormat;
	devProcs->SwapchainTexture = GL_SwapchainTexture;
	devProcs->SwapchainDesc = GL_SwapchainDesc;
	devProcs->ScreenResized = GL_ScreenResized;

	devProcs->CreateTexture = GL_CreateTexture;
	devProcs->TextureLayout = GL_TextureLayout;
	devProcs->DestroyTexture = GL_DestroyTexture;

	devProcs->CreateSampler = GL_CreateSampler;
	devProcs->DestroySampler = GL_DestroySampler;

	devProcs->CreateBuffer = GL_CreateBuffer;
	devProcs->UpdateBuffer = GL_UpdateBuffer;
	devProcs->DestroyBuffer = GL_DestroyBuffer;

	devProcs->CreateAccelerationStructure = GL_CreateAccelerationStructure;
	devProcs->DestroyAccelerationStructure = GL_DestroyAccelerationStructure;

	devProcs->LoadPipelineCache = GL_LoadPipelineCache;
	devProcs->SavePipelineCache = GL_SavePipelineCache;

	devProcs->CreateContext = GL_CreateContext;
	devProcs->ResetContext = GL_ResetContext;
	devProcs->DestroyContext = GL_DestroyContext;

	devProcs->CreateSurface = GL_CreateSurface;
	devProcs->DestroySurface = GL_DestroySurface;

	devProcs->CreateSwapchain = GL_CreateSwapchain;
	devProcs->DestroySwapchain = GL_DestroySwapchain;

	devProcs->CreateFramebuffer = GL_CreateFramebuffer;
	devProcs->SetAttachment = GL_SetAttachment;
	devProcs->DestroyFramebuffer = GL_DestroyFramebuffer;

	devProcs->CreateRenderPassDesc = GL_CreateRenderPassDesc;
	devProcs->DestroyRenderPassDesc = GL_DestroyRenderPassDesc;

	devProcs->ShaderModule = GL_ShaderModule;

	devProcs->Execute = GL_Execute;
	devProcs->WaitIdle = GL_WaitIdle;

	devProcs->CreateTransientBuffer = GL_CreateTransientBuffer;
	devProcs->CreateTransientTexture = GL_CreateTransientTexture;
	devProcs->InitTransientHeap = GL_InitTransientHeap;
	devProcs->ResizeTransientHeap = GL_ResizeTransientHeap;
	devProcs->TermTransientHeap = GL_TermTransientHeap;

	devProcs->MapBuffer = GL_MapBuffer;
	devProcs->FlushBuffer = GL_FlushBuffer;
	devProcs->UnmapBuffer = GL_UnmapBuffer;
	devProcs->BufferAddress = GL_BufferAddress;

	devProcs->CreateSemaphore = GL_CreateSemaphore;
	devProcs->WaitSemaphore = GL_WaitSemaphore;
	devProcs->WaitSemaphores = GL_WaitSemaphores;
	devProcs->SignalSemaphore = GL_SignalSemaphore;
	devProcs->DestroySemaphore = GL_DestroySemaphore;

	devProcs->CreateFence = GL_CreateFence;
	devProcs->SignalFence = GL_SignalFence;
	devProcs->WaitForFence = GL_WaitForFence;
	devProcs->DestroyFence = GL_DestroyFence;

	devProcs->OffsetAddress = GL_OffsetAddress;

	GL_InitContextProcs(ctxProcs);

/*	VkSemaphoreTypeCreateInfo typeInfo =
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
	dev->frameValues = Sys_Alloc(RE_NUM_FRAMES, sizeof(*dev->frameValues), MH_RenderDriver);

	if (!GL_CreateDescriptorSet(dev))
		goto error;

	if (!GL_LoadShaders(dev->dev))
		goto error;

	VkFenceCreateInfo fenceInfo =
	{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
	};
	if (vkCreateFence(dev->dev, &fenceInfo, Vkd_allocCb, &dev->driverTransferFence) != VK_SUCCESS)
		goto error;

	VkCommandPoolCreateInfo poolInfo =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
		.queueFamilyIndex = dev->transferFamily
	};
	if (vkCreateCommandPool(dev->dev, &poolInfo, Vkd_allocCb, &dev->driverTransferPool) != VK_SUCCESS)
		goto error;

	if (E_GetCVarBln(L"VulkanDrv_ForceNonCoherentStaging", false)->bln) {
		Re_deviceInfo.features.coherentMemory = false;
		Sys_LogEntry(VKDRV_MOD, LOG_DEBUG, L"Forcing non-coherent staging memory");
	}

	if (!Re_deviceInfo.features.coherentMemory)
		if (!Vkd_InitStagingArea(dev))
			goto error;*/

	return dev;

//error:
//	if (dev->dev)
//		dkDeviceDestroy(dev->dev);

//	Sys_Free(dev);

//	return NULL;
}

bool
GL_Execute(struct NeRenderDevice *dev, struct NeRenderContext *ctx, bool wait)
{
/*	VkSubmitInfo si =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &ctx->cmdBuffer,
	};
	if (vkQueueSubmit(dev->graphicsQueue, 1, &si, wait ? ctx->executeFence : VK_NULL_HANDLE) != VK_SUCCESS)
		return false;

	if (wait)
		if (vkWaitForFences(dev->dev, 1, &ctx->executeFence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
			return false;*/

	return true;
}

void
GL_WaitIdle(struct NeRenderDevice *dev)
{
//	vkDeviceWaitIdle(dev->dev);
}

void
GL_DestroyDevice(struct NeRenderDevice *dev)
{
/*	if (!Re_deviceInfo.features.coherentMemory)
		Vkd_TermStagingArea(dev);

	GL_TermDescriptorSet(dev);
	GL_UnloadShaders(dev->dev);

	vkDestroyCommandPool(dev->dev, dev->driverTransferPool, Vkd_allocCb);
	vkDestroyFence(dev->dev, dev->driverTransferFence, Vkd_allocCb);
	vkDestroySemaphore(dev->dev, dev->frameSemaphore, Vkd_allocCb);*/

//	Sys_Free(dev->frameValues);
	Sys_Free(dev);
}
