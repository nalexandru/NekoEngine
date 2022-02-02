#include <stdlib.h>

#include <System/Log.h>
#include <System/Memory.h>
#include <Engine/Config.h>

#include "VulkanDriver.h"

static const char *_devExtensions[10] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
static uint32_t _devExtensionCount = 1;

static struct NeRenderInterface *_CreateRenderInterface(struct NeRenderDevice *dev);
static void _DestroyRenderInterface(struct NeRenderInterface *iface);

static inline bool _CheckExtension(const char *name, VkExtensionProperties *extProps, uint32_t extCount);

struct NeRenderDevice *
Vk_CreateDevice(struct NeRenderDeviceInfo *info, struct NeRenderDeviceProcs *devProcs, struct NeRenderContextProcs *ctxProcs)
{
	VkPhysicalDevice physDev = (VkPhysicalDevice)info->private;
	struct NeRenderDevice *dev = Sys_Alloc(1, sizeof(*dev), MH_RenderDriver);

	if (!dev)
		return NULL;

	dev->physDev = physDev;
	vkGetPhysicalDeviceProperties(physDev, &dev->physDevProps);
	vkGetPhysicalDeviceMemoryProperties(physDev, &dev->physDevMemProps);

	void *pNext = NULL;

	VkPhysicalDeviceMeshShaderFeaturesNV *msFeatures = Sys_Alloc(sizeof(*msFeatures), 1, MH_Transient);
	msFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
	msFeatures->pNext = pNext;

	if (info->features.meshShading) {
		pNext = msFeatures;
		msFeatures->taskShader = VK_TRUE;
		msFeatures->meshShader = VK_TRUE;
		_devExtensions[_devExtensionCount++] = VK_NV_MESH_SHADER_EXTENSION_NAME;
	}

	VkPhysicalDeviceRayTracingPipelineFeaturesKHR *rtFeatures = Sys_Alloc(sizeof(*rtFeatures), 1, MH_Transient);
	rtFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	rtFeatures->pNext = pNext;

	if (info->features.rayTracing) {
		pNext = rtFeatures;
		rtFeatures->rayTracingPipeline = VK_TRUE;
		rtFeatures->rayTracingPipelineTraceRaysIndirect = info->features.indirectRayTracing ? VK_TRUE : VK_FALSE;
		_devExtensions[_devExtensionCount++] = VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME;
		_devExtensions[_devExtensionCount++] = VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME;
		_devExtensions[_devExtensionCount++] = VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME;
	}

	VkPhysicalDeviceVulkan11Features *vk11Features = Sys_Alloc(sizeof(*vk11Features), 1, MH_Transient);
	vk11Features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	vk11Features->pNext = pNext;

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
	vk12Features->separateDepthStencilLayouts = VK_TRUE;

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
	uint32_t extCount = 0;
	vkEnumerateDeviceExtensionProperties((VkPhysicalDevice)info->private, NULL, &extCount, NULL);

	VkExtensionProperties *extProps = Sys_Alloc(sizeof(*extProps), extCount, MH_Transient);
	vkEnumerateDeviceExtensionProperties((VkPhysicalDevice)info->private, NULL, &extCount, extProps);

	for (uint32_t i = 0; i < _devExtensionCount; ++i)
		if (!_CheckExtension(_devExtensions[i], extProps, extCount)) {
			Sys_LogEntry(VKDRV_MOD, LOG_CRITICAL, "Selected device missing required extension %s", _devExtensions[i]);
			goto error;
		}

#ifdef _DEBUG
	if (_CheckExtension(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME, extProps, extCount)) {
		_devExtensions[_devExtensionCount++] = VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME;
		VkDeviceDiagnosticsConfigCreateInfoNV ddcci =
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_DIAGNOSTICS_CONFIG_CREATE_INFO_NV,
			.pNext = features->pNext,
			.flags = VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_AUTOMATIC_CHECKPOINTS_BIT_NV |
				VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_RESOURCE_TRACKING_BIT_NV |
				VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_SHADER_DEBUG_INFO_BIT_NV
		};
		features->pNext = &ddcci;
	}
#endif

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
		Sys_LogEntry(VKDRV_MOD, LOG_CRITICAL, "vkCreateDevice = %d", rc);
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
	devProcs->SwapchainDesc = Vk_SwapchainDesc;
	devProcs->ScreenResized = Vk_ScreenResized;

	devProcs->CreateTexture = Vk_CreateTexture;
	devProcs->TextureLayout = Vk_TextureLayout;
	devProcs->DestroyTexture = Vk_DestroyTexture;

	devProcs->CreateSampler = (struct NeSampler *(*)(struct NeRenderDevice *, const struct NeSamplerDesc *))Vk_CreateSampler;
	devProcs->DestroySampler = (void (*)(struct NeRenderDevice *dev, struct NeSampler *))Vk_DestroySampler;

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

	devProcs->CreateSurface = (struct NeSurface *(*)(struct NeRenderDevice *, void *))Vk_CreateSurface;
	devProcs->DestroySurface = (void (*)(struct NeRenderDevice *, struct NeSurface *))Vk_DestroySurface;

	devProcs->CreateSwapchain = (struct NeSwapchain *(*)(struct NeRenderDevice *dev, struct NeSurface *, bool))Vk_CreateSwapchain;
	devProcs->DestroySwapchain = (void(*)(struct NeRenderDevice *dev, struct NeSwapchain *))Vk_DestroySwapchain;

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
	devProcs->FlushBuffer = Vk_FlushBuffer;
	devProcs->UnmapBuffer = Vk_UnmapBuffer;
	devProcs->BufferAddress = Vk_BufferAddress;

	devProcs->CreateSemaphore = Vk_CreateSemaphore;
	devProcs->WaitSemaphore = Vk_WaitSemaphore;
	devProcs->WaitSemaphores = Vk_WaitSemaphores;
	devProcs->SignalSemaphore = Vk_SignalSemaphore;
	devProcs->DestroySemaphore = Vk_DestroySemaphore;

	devProcs->CreateFence = (struct NeFence *(*)(struct NeRenderDevice *, bool))Vk_CreateFence;
	devProcs->SignalFence = (void(*)(struct NeRenderDevice *, struct NeFence *))Vk_SignalFence;
	devProcs->WaitForFence = (bool(*)(struct NeRenderDevice *, struct NeFence *, uint64_t))Vk_WaitForFence;
	devProcs->DestroyFence = (void(*)(struct NeRenderDevice *, struct NeFence *))Vk_DestroyFence;

	devProcs->OffsetAddress = Vk_OffsetAddress;

	devProcs->CreateRenderInterface = _CreateRenderInterface;
	devProcs->DestroyRenderInterface = _DestroyRenderInterface;

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

	if (E_GetCVarBln("VulkanDrv_ForceNonCoherentStaging", false)->bln) {
		Re_deviceInfo.features.coherentMemory = false;
		Sys_LogEntry(VKDRV_MOD, LOG_DEBUG, "Forcing non-coherent staging memory");
	}

	if (!Re_deviceInfo.features.coherentMemory)
		if (!Vkd_InitStagingArea(dev))
			goto error;

#ifdef _DEBUG
	Vkd_SetObjectName(dev->dev, dev->graphicsQueue, VK_OBJECT_TYPE_QUEUE, "Graphics Queue");
	Vkd_SetObjectName(dev->dev, dev->computeQueue, VK_OBJECT_TYPE_QUEUE, "Compute Queue");
	Vkd_SetObjectName(dev->dev, dev->transferQueue, VK_OBJECT_TYPE_QUEUE, "Transfer Queue");

	Vkd_SetObjectName(dev->dev, dev->frameSemaphore, VK_OBJECT_TYPE_SEMAPHORE, "Frame Semaphore");

	Vkd_SetObjectName(dev->dev, dev->driverTransferFence, VK_OBJECT_TYPE_FENCE, "Driver Transfer Fence");
	Vkd_SetObjectName(dev->dev, dev->driverTransferPool, VK_OBJECT_TYPE_COMMAND_POOL, "Driver Transfer Pool");
#endif

	return dev;

error:
	if (dev->dev)
		vkDestroyDevice(dev->dev, Vkd_allocCb);

	Sys_Free(dev);

	return NULL;
}

bool
Vk_Execute(struct NeRenderDevice *dev, struct NeRenderContext *ctx, bool wait)
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
Vk_WaitIdle(struct NeRenderDevice *dev)
{
	vkDeviceWaitIdle(dev->dev);
}

void
Vk_DestroyDevice(struct NeRenderDevice *dev)
{
	if (!Re_deviceInfo.features.coherentMemory)
		Vkd_TermStagingArea(dev);

	Vk_TermDescriptorSet(dev);
	Vk_UnloadShaders(dev->dev);

	vkDestroyCommandPool(dev->dev, dev->driverTransferPool, Vkd_allocCb);
	vkDestroyFence(dev->dev, dev->driverTransferFence, Vkd_allocCb);
	vkDestroySemaphore(dev->dev, dev->frameSemaphore, Vkd_allocCb);

	vkDestroyDevice(dev->dev, Vkd_allocCb);

	Sys_Free(dev->frameValues);
	Sys_Free(dev);
}

static uint64_t _IFace_FrameSemaphoreValue(struct NeRenderDevice *dev) { return dev->frameValues[Re_frameId]; }
static VkCommandBuffer _IFace_CurrentCommandBuffer(struct NeRenderContext *ctx) { return ctx->cmdBuffer; }
static VkSemaphore _IFace_SemaphoreHandle(struct NeSemaphore *sem) { return sem->sem; }
static uint64_t _IFace_CurrentSemaphoreValue(struct NeSemaphore *sem) { return sem->value; }
static VkImage _IFace_Image(struct NeTexture *tex) { return tex->image; }
static VkImageView _IFace_ImageView(struct NeTexture *tex) { return tex->imageView; }
static VkBuffer _IFace_Buffer(struct NeBuffer *buff) { return buff->buff; }
static VkAccelerationStructureKHR _IFace_AccelerationStructure(struct NeAccelerationStructure *as) { return as->as; }
static VkFramebuffer _IFace_Framebuffer(struct NeFramebuffer *fb) { return fb->fb; }
static VkRenderPass _IFace_RenderPass(struct NeRenderPassDesc *rp) { return rp->rp; }
static VkSampler _IFace_Sampler(struct NeSampler *s) { return (VkSampler)s; }
static VkPipeline _IFace_Pipeline(struct NePipeline *p) { return p->pipeline; }

static struct NeRenderInterface *
_CreateRenderInterface(struct NeRenderDevice *dev)
{
	struct NeRenderInterface *iface = Sys_Alloc(sizeof(*iface), 1, MH_RenderDriver);
	if (!iface)
		return NULL;

	iface->CurrentCommandBuffer = _IFace_CurrentCommandBuffer;

	iface->FrameSemaphoreValue = _IFace_FrameSemaphoreValue;
	iface->frameSemaphore = dev->frameSemaphore;

	iface->SemaphoreHandle = _IFace_SemaphoreHandle;
	iface->CurrentSemaphoreValue = _IFace_CurrentSemaphoreValue;
	iface->Image = _IFace_Image;
	iface->ImageView = _IFace_ImageView;
	iface->Buffer = _IFace_Buffer;
	iface->Framebuffer = _IFace_Framebuffer;
	iface->Sampler = _IFace_Sampler;
	iface->RenderPass = _IFace_RenderPass;
	iface->AccelerationStructure = _IFace_AccelerationStructure;
	iface->Pipeline = _IFace_Pipeline;
	
	iface->device = dev->dev;
	iface->graphicsQueue = dev->graphicsQueue;
	iface->transferQueue = dev->transferQueue;
	iface->computeQueue = dev->computeQueue;
	iface->graphicsFamily = dev->graphicsFamily;
	iface->transferFamily = dev->transferFamily;
	iface->computeFamily = dev->computeFamily;
	iface->physicalDevice = dev->physDev;
	iface->pipelineCache = Vkd_pipelineCache;
	iface->allocationCallbacks = Vkd_allocCb;

	iface->instance = Vkd_inst;
	iface->GetInstanceProcAddr = vkGetInstanceProcAddr;

	return iface;
}

static void
_DestroyRenderInterface(struct NeRenderInterface *iface)
{
	Sys_Free(iface);
}

static inline bool
_CheckExtension(const char *name, VkExtensionProperties *extProps, uint32_t extCount)
{
	size_t len = strnlen(name, VK_MAX_EXTENSION_NAME_SIZE);
	for (uint32_t i = 0; i < extCount; ++i)
		if (!strncmp(extProps[i].extensionName, name, len))
			return true;
	return false;
}
