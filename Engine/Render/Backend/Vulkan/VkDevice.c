#include <stdlib.h>

#include <System/Log.h>
#include <System/Memory.h>
#include <Engine/Config.h>

#include "VulkanBackend.h"

struct FamilyInfo
{
	uint32_t id, available, count;
};

static const char *f_devExtensions[10] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
static uint32_t f_devExtensionCount = 1;

static inline bool CheckExtension(const char *name, VkExtensionProperties *extProps, uint32_t extCount);
static inline bool InitQueues(struct NeRenderDevice *dev, VkDeviceCreateInfo *ci);

struct NeRenderDevice *
Re_CreateDevice(struct NeRenderDeviceInfo *info)
{
	VkPhysicalDevice physDev = (VkPhysicalDevice)info->reserved;
	struct NeRenderDevice *dev = Sys_Alloc(1, sizeof(*dev), MH_RenderBackend);

	if (!dev)
		return NULL;

	dev->physDev = physDev;
	vkGetPhysicalDeviceProperties(physDev, &dev->physDevProps);
	vkGetPhysicalDeviceMemoryProperties(physDev, &dev->physDevMemProps);

	void *pNext = NULL;

	if (!CVAR_BOOL("Vulkan_DisableRTX")) {
		VkPhysicalDeviceMeshShaderFeaturesNV *msFeatures = Sys_Alloc(sizeof(*msFeatures), 1, MH_Transient);
		msFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
		msFeatures->pNext = pNext;

		if (info->features.meshShading) {
			pNext = msFeatures;
			msFeatures->taskShader = VK_TRUE;
			msFeatures->meshShader = VK_TRUE;
			f_devExtensions[f_devExtensionCount++] = VK_NV_MESH_SHADER_EXTENSION_NAME;
		}

		VkPhysicalDeviceRayTracingPipelineFeaturesKHR *rtFeatures = Sys_Alloc(sizeof(*rtFeatures), 1, MH_Transient);
		rtFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		rtFeatures->pNext = pNext;

		if (info->features.rayTracing) {
			pNext = rtFeatures;
			rtFeatures->rayTracingPipeline = VK_TRUE;
			rtFeatures->rayTracingPipelineTraceRaysIndirect = info->features.indirectRayTracing ? VK_TRUE : VK_FALSE;
			f_devExtensions[f_devExtensionCount++] = VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME;
			f_devExtensions[f_devExtensionCount++] = VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME;
			f_devExtensions[f_devExtensionCount++] = VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME;
		}
	}

	if (dev->physDevProps.apiVersion < VK_MAKE_VERSION(1, 3, 0)) {
		VkPhysicalDeviceSynchronization2FeaturesKHR *s2Features = Sys_Alloc(sizeof(*s2Features), 1, MH_Transient);
		s2Features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
		s2Features->pNext = pNext;

		s2Features->synchronization2 = true;
		
		pNext = s2Features;
		f_devExtensions[f_devExtensionCount++] = VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME;
	} else {
		VkPhysicalDeviceVulkan13Features *vk13Features = Sys_Alloc(sizeof(*vk13Features), 1, MH_Transient);
		vk13Features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		vk13Features->pNext = pNext;

		vk13Features->synchronization2 = true;

		pNext = vk13Features;
	}

	VkPhysicalDeviceVulkan11Features *vk11Features = Sys_Alloc(sizeof(*vk11Features), 1, MH_Transient);
	vk11Features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	vk11Features->pNext = pNext;

	if (info->features.multiDrawIndirect)
		vk11Features->shaderDrawParameters = true;

	VkPhysicalDeviceVulkan12Features *vk12Features = Sys_Alloc(sizeof(*vk12Features), 1, MH_Transient);
	vk12Features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	vk12Features->pNext = vk11Features;

	vk12Features->shaderInt8 = VK_TRUE;
	vk12Features->storageBuffer8BitAccess = VK_TRUE;

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
	features->features.fillModeNonSolid = VK_TRUE;

	if (info->features.bcTextureCompression)
		features->features.textureCompressionBC = VK_TRUE;

	if (info->features.astcTextureCompression)
		features->features.textureCompressionASTC_LDR = VK_TRUE;

	// check extension support
	uint32_t extCount = 0;
	vkEnumerateDeviceExtensionProperties((VkPhysicalDevice)info->reserved, NULL, &extCount, NULL);

	VkExtensionProperties *extProps = Sys_Alloc(sizeof(*extProps), extCount, MH_Transient);
	vkEnumerateDeviceExtensionProperties((VkPhysicalDevice)info->reserved, NULL, &extCount, extProps);

	for (uint32_t i = 0; i < f_devExtensionCount; ++i)
		if (!CheckExtension(f_devExtensions[i], extProps, extCount)) {
			Sys_LogEntry(VKDRV_MOD, LOG_CRITICAL, "Selected device missing required extension %s", f_devExtensions[i]);
			goto error;
		}

	if (E_GetCVarBln("Vulkan_EnableDeviceDiagnostics", false)->bln &&
			CheckExtension(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME, extProps, extCount)) {
		f_devExtensions[f_devExtensionCount++] = VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME;
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

	VkDeviceCreateInfo devInfo =
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = features,
		.enabledExtensionCount = f_devExtensionCount,
		.ppEnabledExtensionNames = f_devExtensions
	};

	if (!InitQueues(dev, &devInfo))
		goto error;

	VkResult rc = vkCreateDevice((VkPhysicalDevice)info->reserved, &devInfo, Vkd_allocCb, &dev->dev);
	if (rc != VK_SUCCESS) {
		Sys_LogEntry(VKDRV_MOD, LOG_CRITICAL, "vkCreateDevice = %d", rc);
		goto error;
	}

	Sys_LogEntry(VKDRV_MOD, LOG_INFORMATION, "Device API version %d.%d.%d",
		VK_VERSION_MAJOR(dev->physDevProps.apiVersion),
		VK_VERSION_MINOR(dev->physDevProps.apiVersion),
		VK_VERSION_PATCH(dev->physDevProps.apiVersion));

	volkLoadDevice(dev->dev);

	if (!vkQueueSubmit2KHR)
		vkQueueSubmit2KHR = vkQueueSubmit2;

	if (!vkCmdPipelineBarrier2KHR)
		vkCmdPipelineBarrier2KHR = vkCmdPipelineBarrier2;

	vkGetDeviceQueue(dev->dev, dev->graphics.family, dev->graphics.id, &dev->graphics.queue);
	Sys_InitFutex(&dev->graphics.ftx);

	if (dev->compute.family != dev->graphics.family || dev->compute.id != dev->graphics.id) {
		vkGetDeviceQueue(dev->dev, dev->compute.family, dev->compute.id, &dev->compute.queue);
		Sys_InitFutex(&dev->compute.ftx);
	} else {
		dev->compute.queue = dev->graphics.queue;
		dev->compute.ftx = dev->graphics.ftx;
	}

	if (dev->transfer.family != dev->compute.family || dev->transfer.id != dev->compute.id) {
		vkGetDeviceQueue(dev->dev, dev->transfer.family, dev->transfer.id, &dev->transfer.queue);
		Sys_InitFutex(&dev->transfer.ftx);
	} else {
		dev->transfer.queue = dev->compute.queue;
		dev->transfer.ftx = dev->compute.ftx;
	}

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
		.queueFamilyIndex = dev->transfer.family
	};
	if (vkCreateCommandPool(dev->dev, &poolInfo, Vkd_allocCb, &dev->driverTransferPool) != VK_SUCCESS)
		goto error;

	if (E_GetCVarBln("Vulkan_ForceNonCoherentStaging", false)->bln) {
		Re_deviceInfo.features.coherentMemory = false;
		Sys_LogEntry(VKDRV_MOD, LOG_DEBUG, "Forcing non-coherent staging memory");
	}

	if (!Re_deviceInfo.features.coherentMemory)
		if (!VkBk_InitStagingArea(dev))
			goto error;

#ifdef _DEBUG
	VkBk_SetObjectName(dev->dev, dev->graphics.queue, VK_OBJECT_TYPE_QUEUE, "Graphics Queue");
	VkBk_SetObjectName(dev->dev, dev->compute.queue, VK_OBJECT_TYPE_QUEUE, "Compute Queue");
	VkBk_SetObjectName(dev->dev, dev->transfer.queue, VK_OBJECT_TYPE_QUEUE, "Transfer Queue");

	VkBk_SetObjectName(dev->dev, dev->frameSemaphore, VK_OBJECT_TYPE_SEMAPHORE, "Frame Semaphore");

	VkBk_SetObjectName(dev->dev, dev->driverTransferFence, VK_OBJECT_TYPE_FENCE, "Driver Transfer Fence");
	VkBk_SetObjectName(dev->dev, dev->driverTransferPool, VK_OBJECT_TYPE_COMMAND_POOL, "Driver Transfer Pool");
#endif

	VkBk_InitDStorage();

	return dev;

error:
	if (dev->dev)
		vkDestroyDevice(dev->dev, Vkd_allocCb);

	Sys_Free(dev);

	return NULL;
}

bool
Re_Execute(struct NeRenderContext *ctx, bool wait)
{
	VkCommandBufferSubmitInfoKHR cbsi =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR,
		.commandBuffer = ctx->cmdBuffer
	};
	VkSubmitInfo2KHR si =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cbsi
	};
	if (Vkd_QueueSubmit(&Re_device->graphics, 1, &si, wait ? ctx->executeFence : VK_NULL_HANDLE) != VK_SUCCESS)
		return false;

	if (wait)
		if (vkWaitForFences(Re_device->dev, 1, &ctx->executeFence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
			return false;

	return true;
}

void
Re_WaitIdle(void)
{
	vkDeviceWaitIdle(Re_device->dev);
}

void
Re_DestroyDevice(struct NeRenderDevice *dev)
{
	if (!Re_deviceInfo.features.coherentMemory)
		VkBk_TermStagingArea(dev);

	VkBk_TermDStorage();

	Vk_TermDescriptorSet(dev);
	Vk_UnloadShaders(dev->dev);

	vkDestroyCommandPool(dev->dev, dev->driverTransferPool, Vkd_allocCb);
	vkDestroyFence(dev->dev, dev->driverTransferFence, Vkd_allocCb);
	vkDestroySemaphore(dev->dev, dev->frameSemaphore, Vkd_allocCb);

	vkDestroyDevice(dev->dev, Vkd_allocCb);

	Sys_TermFutex(dev->graphics.ftx);
	Sys_TermFutex(dev->transfer.ftx);
	Sys_TermFutex(dev->compute.ftx);

	Sys_Free(dev->frameValues);
	Sys_Free(dev);
}

static uint64_t IFace_FrameSemaphoreValue(struct NeRenderDevice *dev) { return dev->frameValues[Re_frameId]; }
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
}

static inline bool
CheckExtension(const char *name, VkExtensionProperties *extProps, uint32_t extCount)
{
	size_t len = strnlen(name, VK_MAX_EXTENSION_NAME_SIZE);
	for (uint32_t i = 0; i < extCount; ++i)
		if (!strncmp(extProps[i].extensionName, name, len))
			return true;
	return false;
}

static inline bool
InitQueues(struct NeRenderDevice *dev, VkDeviceCreateInfo *ci)
{
	uint32_t count;
	vkGetPhysicalDeviceQueueFamilyProperties(dev->physDev, &count, NULL);

	VkQueueFamilyProperties *queueProperties = Sys_Alloc(sizeof(*queueProperties), count, MH_Transient);
	vkGetPhysicalDeviceQueueFamilyProperties(dev->physDev, &count, queueProperties);

	dev->graphics.family = dev->compute.family = dev->transfer.family = UINT32_MAX;
	struct FamilyInfo graphics = { UINT32_MAX, 0, 0 },
		compute = { UINT32_MAX, 0, 0 }, transfer = { UINT32_MAX, 0, 0 };

	if (count == 1) {
		graphics.id = compute.id = transfer.id = 0;
		graphics.available = compute.available = transfer.available = queueProperties[0].queueCount;
	} else {
		for (uint32_t i = 0; i < count; ++i) {
			if (graphics.id == UINT32_MAX && queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				graphics.id = i;
				graphics.available = queueProperties[i].queueCount;
			}

			if (compute.id == UINT32_MAX && queueProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT
					&& !(queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
				compute.id = i;
				compute.available = queueProperties[i].queueCount;
			}

			if (transfer.id == UINT32_MAX && queueProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT
					&& !(queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
					&& !(queueProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)) {
				transfer.id = i;
				transfer.available = queueProperties[i].queueCount;
			}
		}

		if (graphics.id == UINT32_MAX)
			return false;

		if (compute.id == UINT32_MAX)
			compute.id = graphics.id;

		if (transfer.id == UINT32_MAX)
			transfer.id = compute.id;

		if (!E_GetCVarBln("Vulkan_UseComputeQueue", true)->bln)
			compute.id = graphics.id;
	}

	graphics.count = 1;
	dev->graphics.family = graphics.id;

	if (compute.id == graphics.id) {
		if (graphics.available >= 2) {
			dev->compute.id = 1;
			graphics.count = 2;
		}

		dev->compute.family = graphics.id;
	} else {
		dev->compute.family = compute.id;
		compute.count = 1;
	}

	if (transfer.id == compute.id) {
		if (compute.id == graphics.id) {
			if (graphics.available >= 3) {
				dev->transfer.id = 2;
				graphics.count = 3;
			} else {
				dev->transfer.id = dev->compute.id;
			}
		} else if (compute.available >= 2) {
			dev->transfer.id = 1;
			compute.count = 2;
		}

		dev->transfer.family = dev->compute.family;
	} else {
		dev->transfer.family = transfer.id;
		transfer.count = 1;
	}

	float *priority = Sys_Alloc(sizeof(*priority), 1, MH_Transient);
	*priority = 1.f;

	uint32_t families[3] = { dev->graphics.family, dev->compute.family, dev->transfer.family };
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
		queueInfo[queueInfoCount++].pQueuePriorities = priority;
	}

	for (uint32_t i = 0; i < queueInfoCount; ++i) {
		if (queueInfo[i].queueFamilyIndex == graphics.id)
			queueInfo[i].queueCount = graphics.count;
		else if (queueInfo[i].queueFamilyIndex == compute.id)
			queueInfo[i].queueCount = compute.count;
		else if (queueInfo[i].queueFamilyIndex == transfer.id)
			queueInfo[i].queueCount = transfer.count;
	}

	ci->queueCreateInfoCount = queueInfoCount;
	ci->pQueueCreateInfos = queueInfo;

	return true;
}

/* NekoEngine
 *
 * VkDevice.c
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
