#include <stdio.h>
#include <stdlib.h>

#include <Engine/XR.h>
#include <Engine/Job.h>
#include <System/Log.h>
#include <System/Memory.h>
#include <Runtime/Runtime.h>
#include <Engine/Config.h>
#include <Engine/Version.h>
#include <Engine/Application.h>
#include <Render/Backend.h>

#include "VulkanBackend.h"

const char *Re_backendName = "Vulkan";

VkInstance Vkd_inst = VK_NULL_HANDLE;
struct NeArray Vkd_contexts;
uint32_t Vkd_instanceVersion;

static const char *_instLayers[10] = { 0 };
static uint32_t _instLayerCount = 0;
static const char *_instExtensions[10] =
{
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
#ifdef _DEBUG
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
};
#ifdef _DEBUG
static uint32_t _instExtensionCount = 3;
#else
static uint32_t _instExtensionCount = 2;
#endif

extern const char *PlatformSurfaceExtensionName;

bool
Re_InitBackend(void)
{
	VkResult rc = volkInitialize();
	if (rc != VK_SUCCESS) {
		return false;
	}

	rc = vkEnumerateInstanceVersion(&Vkd_instanceVersion);
	if (rc != VK_SUCCESS) {
		return false;
	}

	if (Vkd_instanceVersion < VK_API_VERSION_1_2) {
		return false;
	}

	Sys_LogEntry(VKDRV_MOD, LOG_INFORMATION, "Instance API version %d.%d.%d",
		VK_VERSION_MAJOR(Vkd_instanceVersion), VK_VERSION_MINOR(Vkd_instanceVersion), VK_VERSION_PATCH(Vkd_instanceVersion));

#if ENABLE_OPENXR
	if (E_xrInstance) {
		PFN_xrGetVulkanDeviceExtensionsKHR xrGetVkDevExt = NULL;
		PFN_xrGetVulkanInstanceExtensionsKHR xrGetVkInstExt = NULL;
		PFN_xrGetVulkanGraphicsRequirementsKHR xrGetVkGfxReq = NULL;

		xrGetInstanceProcAddr(E_xrInstance, "xrGetVulkanDeviceExtensionsKHR", (PFN_xrVoidFunction *)&xrGetVkDevExt);
		xrGetInstanceProcAddr(E_xrInstance, "xrGetVulkanInstanceExtensionsKHR", (PFN_xrVoidFunction *)&xrGetVkInstExt);
		xrGetInstanceProcAddr(E_xrInstance, "xrGetVulkanGraphicsRequirementsKHR", (PFN_xrVoidFunction *)&xrGetVkGfxReq);

		if (!xrGetVkDevExt || !xrGetVkInstExt || !xrGetVkGfxReq) {
			Sys_LogEntry(VKDRV_MOD, LOG_CRITICAL, "Required extensions for OpenXR support not found");
			return false;
		}

		XrGraphicsRequirementsVulkanKHR gr = { .type = XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR };
		xrGetVkGfxReq(E_xrInstance, E_xrSystemId, &gr);

		char* buff = NULL;
		uint32_t count = 0;

		xrGetVkInstExt(E_xrInstance, E_xrSystemId, 0, &count, NULL);
		buff = Sys_Alloc(sizeof(*buff), count, MH_Transient);
		xrGetVkInstExt(E_xrInstance, E_xrSystemId, count, NULL, buff);

		xrGetVkDevExt(E_xrInstance, E_xrSystemId, 0, &count, NULL);
		buff = Sys_Alloc(sizeof(*buff), count, MH_Transient);
		xrGetVkDevExt(E_xrInstance, E_xrSystemId, count, NULL, buff);

		// FIXME
	}
#endif

	VkApplicationInfo appInfo =
	{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = App_applicationInfo.name,
		.applicationVersion = VK_MAKE_VERSION(App_applicationInfo.version.major,
			App_applicationInfo.version.minor, App_applicationInfo.version.build),
		.pEngineName = "NekoEngine",
		.engineVersion = VK_MAKE_VERSION(E_VER_MAJOR, E_VER_MINOR, E_VER_BUILD),
		.apiVersion = Vkd_instanceVersion > VK_API_VERSION_1_2 ? VK_API_VERSION_1_3 : VK_API_VERSION_1_2
	};
	VkInstanceCreateInfo instInfo =
	{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = _instLayers,
		.enabledExtensionCount = 0,
		.ppEnabledExtensionNames = _instExtensions
	};

	_instExtensions[_instExtensionCount++] = PlatformSurfaceExtensionName;

#ifdef _DEBUG
	if (E_GetCVarBln("VulkanBackend_Validation", true)->bln) {
#else
	if (E_GetCVarBln("VulkanBackend_Validation", false)->bln) {
#endif
		_instLayers[_instLayerCount++] = "VK_LAYER_KHRONOS_validation";
		Sys_LogEntry(VKDRV_MOD, LOG_INFORMATION, "Validation enabled");
	}

	instInfo.enabledLayerCount = _instLayerCount;
	instInfo.enabledExtensionCount = _instExtensionCount;

	rc = vkCreateInstance(&instInfo, Vkd_allocCb, &Vkd_inst);
	if (rc != VK_SUCCESS) {
		if (rc == VK_ERROR_LAYER_NOT_PRESENT) {
			Sys_LogEntry(VKDRV_MOD, LOG_WARNING, "Failed to create instance with validation layers, disabling layers.");
			instInfo.enabledLayerCount = 0;

			rc = vkCreateInstance(&instInfo, Vkd_allocCb, &Vkd_inst);
			if (rc != VK_SUCCESS)
				return false;
		} else {
			return false;
		}
	}

	volkLoadInstance(Vkd_inst);

#ifdef _DEBUG
	Vkd_InitDebug();
#endif

	Rt_InitPtrArray(&Vkd_contexts, (size_t)E_JobWorkerThreads() + 1, MH_RenderDriver);

	return true;
}

void
Re_TermBackend(void)
{
	Rt_TermArray(&Vkd_contexts);

#ifdef _DEBUG
	Vkd_TermDebug();
#endif

	vkDestroyInstance(Vkd_inst, Vkd_allocCb);
}

bool
Re_EnumerateDevices(uint32_t *count, struct NeRenderDeviceInfo *info)
{
	uint32_t devCount = 0;
	if (vkEnumeratePhysicalDevices(Vkd_inst, &devCount, NULL) != VK_SUCCESS)
		return false;

	VkPhysicalDevice *dev = Sys_Alloc(sizeof(VkPhysicalDevice), devCount, MH_Transient);
	if (!dev)
		return false;

	if (vkEnumeratePhysicalDevices(Vkd_inst, &devCount, dev) != VK_SUCCESS)
		return false;

	if (!*count || !info) {
		for (uint32_t i = 0; i < devCount; ++i) {
			VkPhysicalDeviceVulkan12Features *vk12Features = Sys_Alloc(sizeof(*vk12Features), 1, MH_Transient);
			vk12Features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

			VkPhysicalDeviceFeatures2 *features = Sys_Alloc(sizeof(*features), 1, MH_Transient);
			features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			features->pNext = vk12Features;

			vkGetPhysicalDeviceFeatures2(dev[i], features);

			if (!features->features.fullDrawIndexUint32 || !features->features.samplerAnisotropy)
				continue;
	
			if (!vk12Features->imagelessFramebuffer || !vk12Features->descriptorIndexing ||
					!vk12Features->descriptorBindingPartiallyBound || !vk12Features->timelineSemaphore ||
					!vk12Features->shaderSampledImageArrayNonUniformIndexing || !vk12Features->runtimeDescriptorArray ||
					!vk12Features->descriptorBindingSampledImageUpdateAfterBind || !vk12Features->descriptorBindingStorageBufferUpdateAfterBind ||
					!vk12Features->shaderStorageBufferArrayNonUniformIndexing || !vk12Features->bufferDeviceAddress || !vk12Features->separateDepthStencilLayouts)
				continue;

			++(*count);
		}

		return count;
	}

	uint32_t oi = 0;
	for (uint32_t i = 0; i < devCount; ++i) {
		VkPhysicalDeviceProperties *props = Sys_Alloc(sizeof(*props), 1, MH_Transient);
		VkPhysicalDeviceFeatures2 *features = Sys_Alloc(sizeof(*features), 1, MH_Transient);
		VkPhysicalDeviceMemoryProperties *memProps = Sys_Alloc(sizeof(*memProps), 1, MH_Transient);
		VkPhysicalDeviceVulkan11Features *vk11Features = Sys_Alloc(sizeof(*vk11Features), 1, MH_Transient);
		VkPhysicalDeviceVulkan12Features *vk12Features = Sys_Alloc(sizeof(*vk12Features), 1, MH_Transient);
		VkPhysicalDeviceMeshShaderFeaturesNV *msFeatures = Sys_Alloc(sizeof(*msFeatures), 1, MH_Transient);
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR *rtFeatures = Sys_Alloc(sizeof(*rtFeatures), 1, MH_Transient);
		
		features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features->pNext = vk11Features;

		vk11Features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
		vk11Features->pNext = vk12Features;

		vk12Features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		vk12Features->pNext = msFeatures;

		msFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
		msFeatures->pNext = rtFeatures;

		rtFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		rtFeatures->pNext = NULL;

		vkGetPhysicalDeviceProperties(dev[i], props);
		vkGetPhysicalDeviceFeatures2(dev[i], features);

		// check requirements
		if (!features->features.fullDrawIndexUint32 || !features->features.samplerAnisotropy)
			continue;

		if (!vk12Features->imagelessFramebuffer || !vk12Features->descriptorIndexing ||
				!vk12Features->descriptorBindingPartiallyBound || !vk12Features->timelineSemaphore ||
				!vk12Features->shaderSampledImageArrayNonUniformIndexing || !vk12Features->runtimeDescriptorArray ||
				!vk12Features->descriptorBindingSampledImageUpdateAfterBind || !vk12Features->descriptorBindingStorageBufferUpdateAfterBind ||
				!vk12Features->shaderStorageBufferArrayNonUniformIndexing || !vk12Features->bufferDeviceAddress || !vk12Features->separateDepthStencilLayouts)
			continue;

		snprintf(info[oi].deviceName, sizeof(info[oi].deviceName), "%s", props->deviceName);

		info[oi].features.meshShading = msFeatures->meshShader;
		info[oi].features.rayTracing = rtFeatures->rayTracingPipeline;
		info[oi].features.discrete = props->deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		info[oi].features.drawIndirectCount = vk12Features->drawIndirectCount;
		info[oi].features.bcTextureCompression = features->features.textureCompressionBC;
		info[oi].features.astcTextureCompression = features->features.textureCompressionASTC_LDR;
		info[oi].features.multiDrawIndirect = vk11Features->shaderDrawParameters && vk12Features->drawIndirectCount;
		info[oi].features.secondaryCommandBuffers = true;
		info[oi].features.directIO = false;

		info[oi].limits.maxTextureSize = props->limits.maxImageDimension2D;

		uint32_t familyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(dev[i], &familyCount, NULL);
		for (uint32_t j = 0; j < familyCount; ++j)
			if (Vk_CheckPresentSupport(dev[i], j))
				info[oi].features.canPresent = true;

		info[oi].localMemorySize = 0;
		vkGetPhysicalDeviceMemoryProperties(dev[i], memProps);
		for (uint32_t j = 0; j < memProps->memoryHeapCount; ++j)
			if (memProps->memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
				info[oi].localMemorySize += memProps->memoryHeaps[j].size;

		info[oi].features.coherentMemory = false;
		const VkMemoryPropertyFlags coherentFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		for (uint32_t j = 0; j < memProps->memoryTypeCount; ++j) {
			if ((memProps->memoryTypes[j].propertyFlags & coherentFlags) == coherentFlags) {
				info[oi].features.coherentMemory = true;
				break;
			}
		}

		// this might not be 100% correct
		info[oi].features.unifiedMemory = memProps->memoryHeapCount == 1;

		info[oi].hardwareInfo.deviceId = props->deviceID;
		info[oi].hardwareInfo.vendorId = props->vendorID;
		info[oi].hardwareInfo.driverVersion = props->driverVersion;

		info[oi].reserved = dev[i];

		if (++oi == *count)
			break;
	}

	return true;
}

void
Re_DestroySurface(struct NeSurface *surface)
{
	vkDestroySurfaceKHR(Vkd_inst, (VkSurfaceKHR)surface, Vkd_allocCb);
}

NeDirectIOHandle
Re_BkOpenFile(const char *path)
{
	return NULL;
}

void
Re_BkCloseFile(NeDirectIOHandle handle)
{
	//
}

/* NekoEngine
 *
 * VkBackend.c
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
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
