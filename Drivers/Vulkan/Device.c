#include <stdlib.h>

#include <System/Log.h>
#include <System/Memory.h>

#include "VulkanDriver.h"

static const char *_devExtensions[10] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
static uint32_t _devExtensionCount = 1;

struct RenderDevice *
Vk_CreateDevice(struct RenderDeviceInfo *info, struct RenderDeviceProcs *devProcs, struct RenderContextProcs *ctxProcs)
{
	VkPhysicalDevice physDev = (VkPhysicalDevice)info->private;
	struct RenderDevice *dev = Sys_Alloc(1, sizeof(*dev), MH_RenderDriver);

	if (!dev)
		return NULL;

	dev->physDev = physDev;
	vkGetPhysicalDeviceProperties(physDev, &dev->physDevProps);
	vkGetPhysicalDeviceMemoryProperties(physDev, &dev->physDevMemProps);

	VkPhysicalDeviceMeshShaderFeaturesNV *msFeatures = Sys_Alloc(sizeof(*msFeatures), 1, MH_Transient);
	msFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
	msFeatures->pNext = NULL;

	if (info->features.meshShading) {
		msFeatures->taskShader = VK_TRUE;
		msFeatures->meshShader = VK_TRUE;
		_devExtensions[_devExtensionCount] = VK_NV_MESH_SHADER_EXTENSION_NAME;
	}

	VkPhysicalDeviceRayTracingPipelineFeaturesKHR *rtFeatures = Sys_Alloc(sizeof(*rtFeatures), 1, MH_Transient);
	rtFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	rtFeatures->pNext = msFeatures;

	if (info->features.rayTracing) {
		rtFeatures->rayTracingPipeline = VK_TRUE;
		rtFeatures->rayTracingPipelineTraceRaysIndirect = info->features.indirectRayTracing ? VK_TRUE : VK_FALSE;
		_devExtensions[_devExtensionCount] = VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME;
	}

	VkPhysicalDeviceVulkan11Features *vk11Features = Sys_Alloc(sizeof(*vk11Features), 1, MH_Transient);
	vk11Features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	vk11Features->pNext = rtFeatures;

	if (info->features.multiDrawIndirect)
		vk11Features->shaderDrawParameters = true;

	VkPhysicalDeviceVulkan12Features *vk12Features = Sys_Alloc(sizeof(*vk12Features), 1, MH_Transient);
	vk12Features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	vk12Features->pNext = vk11Features;

	vk12Features->imagelessFramebuffer = VK_TRUE;
	vk12Features->descriptorIndexing = VK_TRUE;
	vk12Features->descriptorBindingPartiallyBound = VK_TRUE;
	vk12Features->bufferDeviceAddress = VK_TRUE;
	vk12Features->timelineSemaphore = VK_TRUE;
	vk12Features->runtimeDescriptorArray = VK_TRUE;
	vk12Features->descriptorBindingVariableDescriptorCount = VK_TRUE;
	vk12Features->shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	vk12Features->shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
	vk12Features->descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
	vk12Features->descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;

	if (info->features.drawIndirectCount)
		vk12Features->drawIndirectCount = true;

	VkPhysicalDeviceFeatures2 *features = Sys_Alloc(sizeof(*features), 1, MH_Transient);
	features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features->pNext = vk12Features;

	features->features.fullDrawIndexUint32 = VK_TRUE;
	features->features.samplerAnisotropy = VK_TRUE;

	if (info->features.bcTextureCompression)
		features->features.textureCompressionBC = VK_TRUE;

	if (info->features.astcTextureCompression)
		features->features.textureCompressionASTC_LDR = VK_TRUE;

	// retrieve queue information
	uint32_t count;
	vkGetPhysicalDeviceQueueFamilyProperties(physDev, &count, NULL);

	VkQueueFamilyProperties *queueProperties = Sys_Alloc(sizeof(*queueProperties), count, MH_Transient);
	vkGetPhysicalDeviceQueueFamilyProperties(physDev, &count, queueProperties);

	dev->graphicsFamily = dev->computeFamily = dev->transferFamily = UINT32_MAX;
	for (uint32_t i = 0; i < count; ++i) {
		if (dev->graphicsFamily == UINT32_MAX && queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			dev->graphicsFamily = i;

		if (dev->computeFamily == UINT32_MAX && queueProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT
				&& !(queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
			dev->computeFamily = i;

		if (dev->transferFamily == UINT32_MAX && queueProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT
				&& !(queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				&& !(queueProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
			dev->transferFamily = i;
	}

	if (dev->graphicsFamily == UINT32_MAX)
		goto error;

	if (dev->computeFamily == UINT32_MAX)
		dev->computeFamily = dev->graphicsFamily;

	if (dev->transferFamily == UINT32_MAX)
		dev->transferFamily = dev->computeFamily;

	float priority = 1.f;
	uint32_t families[3] = { dev->graphicsFamily, dev->computeFamily, dev->transferFamily };
	uint32_t queueInfoCount = 0;
	VkDeviceQueueCreateInfo *queueInfo = Sys_Alloc(sizeof(*queueInfo), 3, MH_Transient);

	for (uint32_t i = 0; i < 3; ++i) {
		bool add = true;

		for (uint32_t j = 0; j < queueInfoCount; ++j) {
			if (queueInfo[j].queueFamilyIndex == families[i]) {
				add = false;
				break;
			}
		}

		if (!add)
			continue;

		queueInfo[queueInfoCount  ].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo[queueInfoCount  ].queueFamilyIndex = families[i];
		queueInfo[queueInfoCount  ].queueCount = 1;
		queueInfo[queueInfoCount++].pQueuePriorities = &priority;
	}

	// check extension support

	VkDeviceCreateInfo devInfo =
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = features,
		.queueCreateInfoCount = queueInfoCount,
		.pQueueCreateInfos = queueInfo,
		.enabledExtensionCount = _devExtensionCount,
		.ppEnabledExtensionNames = _devExtensions
	};

	VkResult rc = vkCreateDevice((VkPhysicalDevice)info->private, &devInfo, Vkd_allocCb, &dev->dev);
	if (rc != VK_SUCCESS) {
		Sys_LogEntry(VKDRV_MOD, LOG_CRITICAL, L"vkCreateDevice = %d", rc);
		goto error;
	}

	volkLoadDevice(dev->dev);

	vkGetDeviceQueue(dev->dev, dev->graphicsFamily, 0, &dev->graphicsQueue);
	vkGetDeviceQueue(dev->dev, dev->computeFamily, 0, &dev->computeQueue);
	vkGetDeviceQueue(dev->dev, dev->transferFamily, 0, &dev->transferQueue);

	devProcs->GraphicsPipeline = Vk_GraphicsPipeline;
	devProcs->ComputePipeline = Vk_ComputePipeline;
	devProcs->RayTracingPipeline = Vk_RayTracingPipeline;
	devProcs->DestroyPipeline = Vk_DestroyPipeline;

	devProcs->AcquireNextImage = Vk_AcquireNextImage;
	devProcs->Present = Vk_Present;
	devProcs->SwapchainFormat = Vk_SwapchainFormat;
	devProcs->SwapchainTexture = Vk_SwapchainTexture;
	devProcs->ScreenResized = Vk_ScreenResized;

	devProcs->CreateTexture = Vk_CreateTexture;
	devProcs->TextureLayout = Vk_TextureLayout;
	devProcs->DestroyTexture = Vk_DestroyTexture;

	devProcs->CreateSampler = (struct Sampler *(*)(struct RenderDevice *, const struct SamplerDesc *))Vk_CreateSampler;
	devProcs->DestroySampler = (void (*)(struct RenderDevice *dev, struct Sampler *))Vk_DestroySampler;

	devProcs->CreateBuffer = Vk_CreateBuffer;
	devProcs->UpdateBuffer = Vk_UpdateBuffer;
	devProcs->DestroyBuffer = Vk_DestroyBuffer;

	devProcs->CreateAccelerationStructure = Vk_CreateAccelerationStructure;
	devProcs->DestroyAccelerationStructure = Vk_DestroyAccelerationStructure;

	devProcs->LoadPipelineCache = Vk_LoadPipelineCache;
	devProcs->SavePipelineCache = Vk_SavePipelineCache;

	devProcs->CreateContext = Vk_CreateContext;
	devProcs->ResetContext = Vk_ResetContext;
	devProcs->DestroyContext = Vk_DestroyContext;

	devProcs->CreateSurface = (struct Surface *(*)(struct RenderDevice *, void *))Vk_CreateSurface;
	devProcs->DestroySurface = (void (*)(struct RenderDevice *, struct Surface *))Vk_DestroySurface;

	devProcs->CreateSwapchain = (struct Swapchain *(*)(struct RenderDevice *dev, struct Surface *))Vk_CreateSwapchain;
	devProcs->DestroySwapchain = (void(*)(struct RenderDevice *dev, struct Swapchain *))Vk_DestroySwapchain;

	devProcs->CreateFramebuffer = Vk_CreateFramebuffer;
	devProcs->SetAttachment = Vk_SetAttachment;
	devProcs->DestroyFramebuffer = Vk_DestroyFramebuffer;

	devProcs->CreateRenderPassDesc = Vk_CreateRenderPassDesc;
	devProcs->DestroyRenderPassDesc = Vk_DestroyRenderPassDesc;

	devProcs->ShaderModule = Vk_ShaderModule;

	devProcs->Execute = Vk_Execute;
	devProcs->WaitIdle = Vk_WaitIdle;

	devProcs->CreateTransientBuffer = Vk_CreateTransientBuffer;
	devProcs->CreateTransientTexture = Vk_CreateTransientTexture;
	devProcs->InitTransientHeap = Vk_InitTransientHeap;
	devProcs->ResizeTransientHeap = Vk_ResizeTransientHeap;
	devProcs->TermTransientHeap = Vk_TermTransientHeap;

	devProcs->MapBuffer = Vk_MapBuffer;
	devProcs->UnmapBuffer = Vk_UnmapBuffer;
	devProcs->BufferAddress = Vk_BufferAddress;

	Vk_InitContextProcs(ctxProcs);

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
	dev->frameValues = Sys_Alloc(RE_NUM_FRAMES, sizeof(*dev->frameValues), MH_RenderDriver);

	if (!Vk_CreateDescriptorSet(dev))
		goto error;

	if (!Vk_LoadShaders(dev->dev))
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

	return dev;

error:
	if (dev->dev)
		vkDestroyDevice(dev->dev, Vkd_allocCb);

	Sys_Free(dev);

	return NULL;
}

bool
Vk_Execute(struct RenderDevice *dev, struct RenderContext *ctx, bool wait)
{
	VkSubmitInfo si =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &ctx->cmdBuffer,
	};
	if (vkQueueSubmit(dev->graphicsQueue, 1, &si, wait ? ctx->executeFence : VK_NULL_HANDLE) != VK_SUCCESS)
		return false;

	if (wait)
		if (vkWaitForFences(dev->dev, 1, &ctx->executeFence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
			return false;

	return true;
}

void
Vk_WaitIdle(struct RenderDevice *dev)
{
	vkDeviceWaitIdle(dev->dev);
}

void
Vk_DestroyDevice(struct RenderDevice *dev)
{
	Vk_TermDescriptorSet(dev);
	Vk_UnloadShaders(dev->dev);

	vkDestroyCommandPool(dev->dev, dev->driverTransferPool, Vkd_allocCb);
	vkDestroyFence(dev->dev, dev->driverTransferFence, Vkd_allocCb);
	vkDestroySemaphore(dev->dev, dev->frameSemaphore, Vkd_allocCb);
	vkDestroyDevice(dev->dev, Vkd_allocCb);

	Sys_Free(dev->frameValues);
	Sys_Free(dev);
}
