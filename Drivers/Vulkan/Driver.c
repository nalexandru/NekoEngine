#include <stdio.h>
#include <stdlib.h>

#include <Engine/Job.h>
#include <System/Memory.h>
#include <Render/Driver.h>
#include <Render/Device.h>
#include <Render/Context.h>
#include <Runtime/Runtime.h>
#include <Engine/Config.h>
#include <Engine/Version.h>
#include <Engine/Application.h>

#include "VulkanDriver.h"

static bool _Init(void);
static void _Term(void);
static bool _EnumerateDevices(uint32_t *, struct RenderDeviceInfo *);

static struct RenderDriver _drv =
{
	NE_RENDER_DRIVER_ID,
	NE_RENDER_DRIVER_API,
	L"Vulkan",
	_Init,
	_Term,
	_EnumerateDevices,
	Vk_CreateDevice,
	Vk_DestroyDevice
};

VkInstance Vkd_inst = VK_NULL_HANDLE;
VkAllocationCallbacks *Vkd_allocCb = NULL;
struct Array Vkd_contexts;

static const char *_instLayers[10] = { 0 };
static uint32_t _instLayerCount = 0;
static const char *_instExtensions[10] =
{
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME
};
static uint32_t _instExtensionCount = 2;

extern const char *PlatformSurfaceExtensionName;

#ifdef RENDER_DRIVER_BUILTIN
const struct RenderDriver *Re_LoadBuiltinDriver() { return &_drv; }
#else
#ifdef _WIN32
#	define EXPORT __declspec(dllexport)
#else
#	define EXPORT
#endif

EXPORT const struct RenderDriver *Re_LoadDriver(void) { return &_drv; }
#endif

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

	char *appName = Sys_Alloc(sizeof(char), wcslen(App_applicationInfo.name), MH_Transient);
	wcstombs(appName, App_applicationInfo.name, wcslen(App_applicationInfo.name));
	VkApplicationInfo appInfo = 
	{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = appName,
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
	if (E_GetCVarBln(L"VulaknDrv_ValidationLayers", true)->bln)
#else
	if (E_GetCVarBln(L"VulaknDrv_ValidationLayers", false)->bln)
#endif
		_instLayers[_instLayerCount++] = "VK_LAYER_KHRONOS_validation";

	instInfo.enabledLayerCount = _instLayerCount;
	instInfo.enabledExtensionCount = _instExtensionCount;

	rc = vkCreateInstance(&instInfo, Vkd_allocCb, &Vkd_inst);
	if (rc != VK_SUCCESS) {
		return false;
	}

	volkLoadInstance(Vkd_inst);

	Rt_InitPtrArray(&Vkd_contexts, E_JobWorkerThreads() + 1);

	return true;
}

static void
_Term(void)
{
	vkDestroyInstance(Vkd_inst, Vkd_allocCb);
}

static bool
_EnumerateDevices(uint32_t *count, struct RenderDeviceInfo *info)
{
	if (!*count || !info) 
		return vkEnumeratePhysicalDevices(Vkd_inst, count, NULL) == VK_SUCCESS;

	VkPhysicalDevice *dev = Sys_Alloc(sizeof(VkPhysicalDevice), *count, MH_Transient);
	if (!dev)
		return false;

	if (vkEnumeratePhysicalDevices(Vkd_inst, count, dev) != VK_SUCCESS)
		return false;

	VkPhysicalDeviceProperties *props = Sys_Alloc(sizeof(*props), 1, MH_Transient);
	VkPhysicalDeviceMemoryProperties *memProps = Sys_Alloc(sizeof(*memProps), 1, MH_Transient);
	VkPhysicalDeviceFeatures2 *features = Sys_Alloc(sizeof(*features), 1, MH_Transient);
	VkPhysicalDeviceVulkan11Features *vk11Features = Sys_Alloc(sizeof(*vk11Features), 1, MH_Transient);
	VkPhysicalDeviceVulkan12Features *vk12Features = Sys_Alloc(sizeof(*vk12Features), 1, MH_Transient);
	VkPhysicalDeviceMeshShaderFeaturesNV *msFeatures = Sys_Alloc(sizeof(*msFeatures), 1, MH_Transient);
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR *rtFeatures = Sys_Alloc(sizeof(*rtFeatures), 1, MH_Transient);
	VkPhysicalDeviceExtendedDynamicStateFeaturesEXT *edsFeatures = Sys_Alloc(sizeof(*edsFeatures), 1, MH_Transient);

	features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features->pNext = vk11Features;

	vk11Features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	vk11Features->pNext = vk12Features;

	vk12Features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	vk12Features->pNext = msFeatures;

	msFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
	msFeatures->pNext = rtFeatures;

	rtFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	rtFeatures->pNext = edsFeatures;

	edsFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
	edsFeatures->pNext = NULL;

	for (uint32_t i = 0; i < *count; ++i) {
		vkGetPhysicalDeviceProperties(dev[i], props);
		vkGetPhysicalDeviceFeatures2(dev[i], features);

		// check requirements
		if (!features->features.fullDrawIndexUint32 || !features->features.samplerAnisotropy)
			continue;

		if (!vk12Features->imagelessFramebuffer || !vk12Features->descriptorIndexing ||
				!vk12Features->descriptorBindingPartiallyBound || !vk12Features->timelineSemaphore)
			continue;

		if (!edsFeatures->extendedDynamicState)
			continue;

		snprintf(info[i].deviceName, sizeof(info[i].deviceName), "%s", props->deviceName);

		info[i].features.meshShading = msFeatures->meshShader;
		info[i].features.rayTracing = rtFeatures->rayTracingPipeline && vk12Features->bufferDeviceAddress;
		info[i].features.discrete = props->deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		info[i].features.drawIndirectCount = vk12Features->drawIndirectCount;
		info[i].features.textureCompression = features->features.textureCompressionBC;

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

		// this might not be 100% correct
		info[i].features.unifiedMemory = memProps->memoryHeapCount == 1;

		info[i].private = dev[i];
	}
	
	return true;
}

void
Vk_DestroySurface(struct RenderDevice *dev, VkSurfaceKHR surface)
{
	vkDestroySurfaceKHR(Vkd_inst, surface, Vkd_allocCb);
}
