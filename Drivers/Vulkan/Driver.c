#include <stdio.h>
#include <stdlib.h>

#include <Engine/Job.h>
#include <System/Log.h>
#include <System/Memory.h>
#include <Runtime/Runtime.h>
#include <Engine/Config.h>
#include <Engine/Version.h>
#include <Engine/Application.h>
#include <Render/Driver/Driver.h>

#include "VulkanDriver.h"

static bool _Init(void);
static void _Term(void);
static bool _EnumerateDevices(uint32_t *, struct NeRenderDeviceInfo *);

static struct NeRenderDriver _drv =
{
	.identifier = NE_RENDER_DRIVER_ID,
	.apiVersion = NE_RENDER_DRIVER_API,
	.driverName = "Vulkan",
	.graphicsApiId = RE_API_VULKAN,
	.Init = _Init,
	.Term = _Term,
	.EnumerateDevices = _EnumerateDevices,
	.CreateDevice = Vk_CreateDevice,
	.DestroyDevice = Vk_DestroyDevice
};

VkInstance Vkd_inst = VK_NULL_HANDLE;
struct NeArray Vkd_contexts;

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

#ifdef _WIN32
#	define EXPORT __declspec(dllexport)
#else
#	define EXPORT
#endif

EXPORT const struct NeRenderDriver *Re_LoadVulkanDriver() { return &_drv; }

static bool
_Init(void)
{
	uint32_t apiVersion;

	VkResult rc = volkInitialize();
	if (rc != VK_SUCCESS) {
		return false;
	}

	rc = vkEnumerateInstanceVersion(&apiVersion);
	if (rc != VK_SUCCESS) {
		return false;
	}

	if (apiVersion < VK_API_VERSION_1_2) {
		return false;
	}

	VkApplicationInfo appInfo =
	{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = App_applicationInfo.name,
		.applicationVersion = VK_MAKE_VERSION(App_applicationInfo.version.major,
			App_applicationInfo.version.minor, App_applicationInfo.version.build),
		.pEngineName = "NekoEngine",
		.engineVersion = VK_MAKE_VERSION(E_VER_MAJOR, E_VER_MINOR, E_VER_BUILD),
		.apiVersion = VK_API_VERSION_1_2
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
	if (E_GetCVarBln("VulkanDrv_Validation", true)->bln) {
#else
	if (E_GetCVarBln("VulkanDrv_Validation", false)->bln) {
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

	Rt_InitPtrArray(&Vkd_contexts, E_JobWorkerThreads() + 1, MH_RenderDriver);

	return true;
}

static void
_Term(void)
{
	Rt_TermArray(&Vkd_contexts);

#ifdef _DEBUG
	Vkd_TermDebug();
#endif

	vkDestroyInstance(Vkd_inst, Vkd_allocCb);
}

static bool
_EnumerateDevices(uint32_t *count, struct NeRenderDeviceInfo *info)
{
	if (!*count || !info)
		return vkEnumeratePhysicalDevices(Vkd_inst, count, NULL) == VK_SUCCESS;

	VkPhysicalDevice *dev = Sys_Alloc(sizeof(VkPhysicalDevice), *count, MH_Transient);
	if (!dev)
		return false;

	if (vkEnumeratePhysicalDevices(Vkd_inst, count, dev) != VK_SUCCESS)
		return false;

	for (uint32_t i = 0; i < *count; ++i) {
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

		snprintf(info[i].deviceName, sizeof(info[i].deviceName), "%s", props->deviceName);

		info[i].features.meshShading = msFeatures->meshShader;
		info[i].features.rayTracing = rtFeatures->rayTracingPipeline;
		info[i].features.discrete = props->deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		info[i].features.drawIndirectCount = vk12Features->drawIndirectCount;
		info[i].features.bcTextureCompression = features->features.textureCompressionBC;
		info[i].features.astcTextureCompression = features->features.textureCompressionASTC_LDR;
		info[i].features.multiDrawIndirect = vk11Features->shaderDrawParameters && vk12Features->drawIndirectCount;
		info[i].features.secondaryCommandBuffers = true;

		info[i].limits.maxTextureSize = props->limits.maxImageDimension2D;

		uint32_t familyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(dev[i], &familyCount, NULL);
		for (uint32_t j = 0; j < familyCount; ++j)
			if (Vk_CheckPresentSupport(dev[i], j))
				info[i].features.canPresent = true;

		info[i].localMemorySize = 0;
		vkGetPhysicalDeviceMemoryProperties(dev[i], memProps);
		for (uint32_t j = 0; j < memProps->memoryHeapCount; ++j)
			if (memProps->memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
				info[i].localMemorySize += memProps->memoryHeaps[j].size;

		info[i].features.coherentMemory = false;
		const VkMemoryPropertyFlags coherentFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		for (uint32_t j = 0; j < memProps->memoryTypeCount; ++j) {
			if ((memProps->memoryTypes[j].propertyFlags & coherentFlags) == coherentFlags) {
				info[i].features.coherentMemory = true;
				break;
			}
		}

		// this might not be 100% correct
		info[i].features.unifiedMemory = memProps->memoryHeapCount == 1;

		info[i].hardwareInfo.deviceId = props->deviceID;
		info[i].hardwareInfo.vendorId = props->vendorID;
		info[i].hardwareInfo.driverVersion = props->driverVersion;

		info[i].private = dev[i];
	}

	return true;
}

void
Vk_DestroySurface(struct NeRenderDevice *dev, VkSurfaceKHR surface)
{
	vkDestroySurfaceKHR(Vkd_inst, surface, Vkd_allocCb);
}
