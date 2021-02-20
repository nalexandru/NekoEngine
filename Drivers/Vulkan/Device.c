#include <stdlib.h>

#include <System/Memory.h>
#include <Render/Device.h>

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
	struct RenderDevice *dev = calloc(1, sizeof(*dev));

	if (!dev)
		return NULL;

	dev->physDev = physDev;
	vkGetPhysicalDeviceProperties(physDev, &dev->physDevProps);

	VkPhysicalDeviceMeshShaderFeaturesNV *msFeatures = Sys_Alloc(sizeof(*msFeatures), 1, MH_Transient);
	msFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;

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

	VkPhysicalDeviceVulkan12Features *vk12Features = Sys_Alloc(sizeof(*vk12Features), 1, MH_Transient);
	vk12Features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	vk12Features->pNext = rtFeatures;

	vk12Features->imagelessFramebuffer = VK_TRUE;
	vk12Features->descriptorIndexing = VK_TRUE;
	vk12Features->descriptorBindingPartiallyBound = VK_TRUE;
	vk12Features->bufferDeviceAddress = VK_TRUE;
	vk12Features->timelineSemaphore = VK_TRUE;

	if (info->features.drawIndirectCount)
		vk12Features->drawIndirectCount;

	VkPhysicalDeviceFeatures2 *features = Sys_Alloc(sizeof(*features), 1, MH_Transient);
	features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features->pNext = vk12Features;

	features->features.fullDrawIndexUint32 = VK_TRUE;
	features->features.samplerAnisotropy = VK_TRUE;

	if (info->features.textureCompression)
		features->features.textureCompressionBC = VK_TRUE;

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

	if (vkCreateDevice((VkPhysicalDevice)info->private, &devInfo, Vkd_allocCb, &dev->dev) != VK_SUCCESS)
		goto error;	

	volkLoadDevice(dev->dev);

	vkGetDeviceQueue(dev->dev, dev->graphicsFamily, 0, &dev->graphicsQueue);
	vkGetDeviceQueue(dev->dev, dev->computeFamily, 0, &dev->computeQueue);
	vkGetDeviceQueue(dev->dev, dev->transferFamily, 0, &dev->transferQueue);

	devProcs->GraphicsPipeline = (struct Pipeline *(*)(struct RenderDevice *, struct Shader *, uint64_t, const struct BlendAttachmentDesc *, uint32_t))Vk_GraphicsPipeline;
	devProcs->ComputePipeline = (struct Pipeline *(*)(struct RenderDevice *, struct Shader *))Vk_ComputePipeline;
	devProcs->RayTracingPipeline = (struct Pipeline *(*)(struct RenderDevice *, struct ShaderBindingTable *))Vk_RayTracingPipeline;
	
	devProcs->AcquireNextImage = (void *(*)(struct RenderDevice *, Swapchain))Vk_AcquireNextImage;
	devProcs->Present = (bool(*)(struct RenderDevice *, struct RenderContext *ctx, Swapchain, void *))Vk_Present;
	devProcs->SwapchainFormat = (enum TextureFormat(*)(Swapchain))Vk_SwapchainFormat;
	devProcs->SwapchainTexture = (struct Texture *(*)(Swapchain, void *))Vk_SwapchainTexture;

	devProcs->CreateTexture = (struct Texture *(*)(struct RenderDevice *, const struct TextureCreateInfo *))Vk_CreateTexture;
	devProcs->TextureDesc = (const struct TextureDesc *(*)(const struct Texture *))Vk_TextureDesc;
	devProcs->DestroyTexture = (void(*)(struct RenderDevice *, struct Texture *))Vk_DestroyTexture;

	devProcs->CreateBuffer = (struct Buffer *(*)(struct RenderDevice *, const struct BufferCreateInfo *))Vk_CreateBuffer;
	devProcs->BufferDesc = (const struct BufferDesc *(*)(const struct Buffer *))Vk_BufferDesc;
	devProcs->DestroyBuffer = (void(*)(struct RenderDevice *, struct Buffer *))Vk_DestroyBuffer;

	devProcs->CreateAccelerationStructure =
		(struct AccelerationStructure *(*)(struct RenderDevice *, const struct AccelerationStructureCreateInfo *))Vk_CreateAccelerationStructure;
	devProcs->DestroyAccelerationStructure = (void(*)(struct RenderDevice *, struct AccelerationStructure *))Vk_DestroyAccelerationStructure;

	devProcs->LoadPipelineCache = (void(*)(struct RenderDevice *))Vk_LoadPipelineCache;
	devProcs->SavePipelineCache = (void(*)(struct RenderDevice *))Vk_SavePipelineCache;

	devProcs->CreateContext = (struct RenderContext *(*)(struct RenderDevice *))Vk_CreateContext;
	devProcs->DestroyContext = (void(*)(struct RenderDevice *, struct RenderContext *))Vk_DestroyContext;

	devProcs->CreateSurface = (Surface (*)(struct RenderDevice *, void *))Vk_CreateSurface;
	devProcs->DestroySurface = (void (*)(struct RenderDevice *, Surface))Vk_DestroySurface;

	devProcs->CreateSwapchain = (Swapchain (*)(struct RenderDevice *dev, Surface))Vk_CreateSwapchain;
	devProcs->DestroySwapchain = (void(*)(struct RenderDevice *dev, Swapchain))Vk_DestroySwapchain;

	devProcs->CreateFramebuffer = Vk_CreateFramebuffer;
	devProcs->SetAttachment = Vk_SetAttachment;
	devProcs->DestroyFramebuffer = Vk_DestroyFramebuffer;

	devProcs->CreateRenderPass = Vk_CreateRenderPass;
	devProcs->DestroyRenderPass = Vk_DestroyRenderPass;

	devProcs->Execute = Vk_Execute;
	devProcs->WaitIdle = Vk_WaitIdle;

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
	dev->frameValues = calloc(RE_NUM_FRAMES, sizeof(*dev->frameValues));

	return dev;

error:
	if (dev->dev)
		vkDestroyDevice(dev->dev, Vkd_allocCb);

	free(dev);

	return NULL;
}

bool
Vk_Execute(struct RenderDevice *dev, struct RenderContext *ctx, bool wait)
{
	VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
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
	vkDestroySemaphore(dev->dev, dev->frameSemaphore, Vkd_allocCb);
	vkDestroyDevice(dev->dev, Vkd_allocCb);

	free(dev->frameValues);
	free(dev);
}
