#include <System/Memory.h>
#include <Render/Device.h>

#include "VKRender.h"

static const char *_devExt[20] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_NV_MESH_SHADER_EXTENSION_NAME,
	VK_KHR_RAY_TRACING_EXTENSION_NAME,
	VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
	VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
	NULL, NULL, NULL, NULL, NULL
};
static uint32_t _devExtCount = 5;

bool
VK_InitDevice(void)
{
	uint32_t deviceCount;
	vkEnumeratePhysicalDevices(VK_Instance, &deviceCount, NULL);

	VkPhysicalDevice *devices = Sys_Alloc(sizeof(*devices), deviceCount, MH_Transient);
	vkEnumeratePhysicalDevices(VK_Instance, &deviceCount, devices);

	VkPhysicalDeviceProperties *deviceProps = Sys_Alloc(sizeof(*deviceProps), 1, MH_Transient);
	VkPhysicalDeviceMemoryProperties *memProps = Sys_Alloc(sizeof(*memProps), 1, MH_Transient);
	VkPhysicalDeviceFeatures2 *deviceFeatures = Sys_Alloc(sizeof(*deviceFeatures), 1, MH_Transient);
	VkPhysicalDeviceVulkan11Features *vk11Features = Sys_Alloc(sizeof(*vk11Features), 1, MH_Transient);
	VkPhysicalDeviceVulkan12Features *vk12Features = Sys_Alloc(sizeof(*vk12Features), 1, MH_Transient);
	VkPhysicalDeviceMeshShaderFeaturesNV *msFeatures = Sys_Alloc(sizeof(*msFeatures), 1, MH_Transient);
	VkPhysicalDeviceRayTracingFeaturesKHR *rtFeatures = Sys_Alloc(sizeof(*rtFeatures), 1, MH_Transient);

	deviceFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures->pNext = vk11Features;

	vk11Features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	vk11Features->pNext = vk12Features;

	vk12Features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	vk12Features->pNext = rtFeatures;

	rtFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_FEATURES_KHR;
	rtFeatures->pNext = msFeatures;

	msFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
	msFeatures->pNext = NULL;

	VkDeviceSize vRam = 0;
	for (uint32_t i = 0; i < deviceCount; ++i) {
		vkGetPhysicalDeviceProperties(devices[i], deviceProps);

		if (deviceProps->apiVersion < VK_VERSION_1_2 ||
				deviceProps->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			continue;

		vkGetPhysicalDeviceFeatures2(devices[i], deviceFeatures);

		if (!deviceFeatures->features.independentBlend ||
				!deviceFeatures->features.samplerAnisotropy ||
				!deviceFeatures->features.fullDrawIndexUint32 ||
				!deviceFeatures->features.textureCompressionBC ||
				!deviceFeatures->features.shaderSampledImageArrayDynamicIndexing)
			continue;

		if (!vk11Features->shaderDrawParameters)
			continue;

		if (!vk12Features->imagelessFramebuffer ||
				!vk12Features->drawIndirectCount ||
				!vk12Features->descriptorIndexing ||
				!vk12Features->runtimeDescriptorArray ||
				!vk12Features->descriptorBindingPartiallyBound ||
				!vk12Features->descriptorBindingVariableDescriptorCount ||
				!vk12Features->shaderSampledImageArrayNonUniformIndexing ||
				!vk12Features->descriptorBindingSampledImageUpdateAfterBind)
			continue;

		if (!rtFeatures->rayTracing ||
				!msFeatures->taskShader ||
				!msFeatures->meshShader)
			continue;

		uint32_t familyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &familyCount, NULL);
		
		bool present = false;
		for (uint32_t j = 0; j < familyCount; ++j)
			if (VK_CheckPresentSupport(devices[i], j))
				present = true;
		
		if (!present)
			continue;

		VkDeviceSize mem = 0;
		vkGetPhysicalDeviceMemoryProperties(devices[i], memProps);

		for (uint32_t j = 0; j < memProps->memoryHeapCount; ++j)
			if (memProps->memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
				mem += memProps->memoryHeaps[j].size;

		if (mem > vRam) {
			vRam = mem;
			Re_Device.physicalDevice = devices[i];

			if (memProps->memoryTypeCount == 1) {
				Re_Device.deviceLocalMemoryType = 0;
			} else {
				uint32_t preferredType = (uint32_t)-1, acceptableType = (uint32_t)-1;

				for (uint32_t j = 0; j < memProps->memoryTypeCount; ++j) {
					if (memProps->memoryTypes[j].propertyFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
						preferredType = j;
						break;
					} else if (memProps->memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
						acceptableType = j;
					}
				}

				Re_Device.deviceLocalMemoryType = preferredType != (uint32_t)-1 ? preferredType : acceptableType;
			}
		}
	}

	if (Re_Device.physicalDevice == VK_NULL_HANDLE) {
		Sys_MessageBox(L"FATAL ERROR", L"No compatible Vulkan device found !", MSG_ICON_ERROR);
		return false;
	}

	vkGetPhysicalDeviceProperties(Re_Device.physicalDevice, &Re_Device.deviceProperties);
	vkGetPhysicalDeviceMemoryProperties(Re_Device.physicalDevice, &Re_Device.memoryProperties);

	mbstowcs(Re.info.device, Re_Device.deviceProperties.deviceName, 64);
	swprintf(Re.info.name, sizeof(Re.info.name) / sizeof(wchar_t), L"Vulkan %d.%d.%d",
		VK_VERSION_MAJOR(Re_Device.deviceProperties.apiVersion), VK_VERSION_MINOR(Re_Device.deviceProperties.apiVersion),
		VK_VERSION_PATCH(Re_Device.deviceProperties.apiVersion));

	uint32_t familyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(Re_Device.physicalDevice, &familyCount, NULL);
	VkQueueFamilyProperties *familyProperties = Sys_Alloc(sizeof(*familyProperties), familyCount, MH_Transient);
	vkGetPhysicalDeviceQueueFamilyProperties(Re_Device.physicalDevice, &familyCount, familyProperties);

	Re_Device.graphicsQueueFamily = Re_Device.computeQueueFamily = Re_Device.transferQueueFamily = -1;

	for (uint32_t i = 0; i < familyCount; ++i) {
		if (Re_Device.graphicsQueueFamily == -1 && familyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			Re_Device.graphicsQueueFamily = i;

		if (Re_Device.computeQueueFamily == -1 && familyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT &&
				!(familyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
			Re_Device.computeQueueFamily = i;

		if (Re_Device.computeQueueFamily == -1 && familyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT &&
				!(familyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
				!(familyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
			Re_Device.transferQueueFamily = i;
	}

	if (Re_Device.computeQueueFamily == -1)
		Re_Device.computeQueueFamily = Re_Device.graphicsQueueFamily;

	if (Re_Device.transferQueueFamily == -1)
		Re_Device.transferQueueFamily = Re_Device.computeQueueFamily;

	memset(deviceFeatures, 0x0, sizeof(*deviceFeatures));
	deviceFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
	deviceFeatures->features.independentBlend = VK_TRUE;
	deviceFeatures->features.samplerAnisotropy = VK_TRUE;
	deviceFeatures->features.fullDrawIndexUint32 = VK_TRUE;
	deviceFeatures->features.textureCompressionBC = VK_TRUE;
	deviceFeatures->features.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
	deviceFeatures->pNext = vk11Features;

	memset(vk11Features, 0x0, sizeof(*vk11Features));
	vk11Features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	vk11Features->shaderDrawParameters = VK_TRUE;
	vk11Features->pNext = vk12Features;

	memset(vk12Features, 0x0, sizeof(*vk12Features));
	vk12Features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	vk12Features->imagelessFramebuffer = VK_TRUE;
	vk12Features->drawIndirectCount = VK_TRUE;
	vk12Features->descriptorIndexing = VK_TRUE;
	vk12Features->runtimeDescriptorArray = VK_TRUE;
	vk12Features->descriptorBindingPartiallyBound = VK_TRUE;
	vk12Features->descriptorBindingVariableDescriptorCount = VK_TRUE;
	vk12Features->shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	vk12Features->descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
	vk12Features->pNext = rtFeatures;

	memset(rtFeatures, 0x0, sizeof(*rtFeatures));
	rtFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_FEATURES_KHR;
	rtFeatures->rayTracing = VK_TRUE;
	rtFeatures->pNext = msFeatures;

	memset(msFeatures, 0x0, sizeof(*msFeatures));
	msFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
	msFeatures->taskShader = VK_TRUE;
	msFeatures->meshShader = VK_TRUE;
	msFeatures->pNext = NULL;

	float priority = 1.f;
	uint32_t families[3] =
	{
		Re_Device.graphicsQueueFamily,
		Re_Device.computeQueueFamily,
		Re_Device.transferQueueFamily
	};

	uint32_t queueCount = 0;
	VkDeviceQueueCreateInfo *qci = Sys_Alloc(sizeof(VkDeviceQueueCreateInfo), 3, MH_Transient);
	for (uint32_t i = 0; i < 3; ++i) {
		bool add = true;

		for (uint8_t j = 0; j < queueCount; ++j) {
			if (qci[j].queueFamilyIndex == families[i]) {
				add = false;
				break;
			}
		}

		if (!add)
			continue;

		qci[queueCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		qci[queueCount].queueFamilyIndex = families[i];
		qci[queueCount].queueCount = 1;
		qci[queueCount].pQueuePriorities = &priority;

		++queueCount;
	}

	VkDeviceCreateInfo dci =
	{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = deviceFeatures,
		.queueCreateInfoCount = queueCount,
		.pQueueCreateInfos = qci,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = NULL,
		.enabledExtensionCount = _devExtCount,
		.ppEnabledExtensionNames = _devExt
	};
	VkResult rc = vkCreateDevice(Re_Device.physicalDevice, &dci, VK_CPUAllocator, &Re_Device.dev);
	if (rc != VK_SUCCESS)
		return false;

	vkGetDeviceQueue(Re_Device.dev, Re_Device.graphicsQueueFamily, 0, &Re_Device.graphicsQueue);
	vkGetDeviceQueue(Re_Device.dev, Re_Device.computeQueueFamily, 0, &Re_Device.computeQueue);
	vkGetDeviceQueue(Re_Device.dev, Re_Device.transferQueueFamily, 0, &Re_Device.transferQueue);

	volkLoadDevice(Re_Device.dev);

	VkSemaphoreCreateInfo sci = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	rc = vkCreateSemaphore(Re_Device.dev, &sci, VK_CPUAllocator, &Re_Device.frameStart);
	if (rc != VK_SUCCESS)
		return false;

	rc = vkCreateSemaphore(Re_Device.dev, &sci, VK_CPUAllocator, &Re_Device.frameEnd);
	if (rc != VK_SUCCESS)
		return false;

	return true;
}

void
VK_TermDevice(void)
{
	if (Re_Device.dev != VK_NULL_HANDLE)
		vkDestroyDevice(Re_Device.dev, VK_CPUAllocator);
}