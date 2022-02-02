#include <System/Log.h>

#include "OpenGLDriver.h"

/*static VkDebugUtilsMessengerEXT _msg;

static VkBool32 VKAPI_CALL _debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
											const VkDebugUtilsMessengerCallbackDataEXT *, void *);

static inline const char *
_objectType(VkObjectType type)
{
	switch (type) {
	case VK_OBJECT_TYPE_INSTANCE: return "Instance";
	case VK_OBJECT_TYPE_PHYSICAL_DEVICE: return "Physical Device";
	case VK_OBJECT_TYPE_DEVICE: return "Device";
	case VK_OBJECT_TYPE_QUEUE: return "Queue";
	case VK_OBJECT_TYPE_SEMAPHORE: return "Semaphore";
	case VK_OBJECT_TYPE_COMMAND_BUFFER: return "Command Buffer";
	case VK_OBJECT_TYPE_FENCE: return "Fence";
	case VK_OBJECT_TYPE_DEVICE_MEMORY: return "Device Memory";
	case VK_OBJECT_TYPE_BUFFER: return "Buffer";
	case VK_OBJECT_TYPE_IMAGE: return "Image";
	case VK_OBJECT_TYPE_EVENT: return "Event";
	case VK_OBJECT_TYPE_QUERY_POOL: return "Query Pool";
	case VK_OBJECT_TYPE_BUFFER_VIEW: return "Buffer View";
	case VK_OBJECT_TYPE_IMAGE_VIEW: return "Image View";
	case VK_OBJECT_TYPE_SHADER_MODULE: return "Shader Module";
	case VK_OBJECT_TYPE_PIPELINE_CACHE: return "Pipeline Cache";
	case VK_OBJECT_TYPE_PIPELINE_LAYOUT: return "Pipeline Layout";
	case VK_OBJECT_TYPE_RENDER_PASS: return "Render Pass";
	case VK_OBJECT_TYPE_PIPELINE: return "Pipeline";
	case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT: return "Descriptor Set Layout";
	case VK_OBJECT_TYPE_SAMPLER: return "Sampler";
	case VK_OBJECT_TYPE_DESCRIPTOR_POOL: return "Descriptor Pool";
	case VK_OBJECT_TYPE_DESCRIPTOR_SET: return "Descriptor Set";
	case VK_OBJECT_TYPE_FRAMEBUFFER: return "Framebuffer";
	case VK_OBJECT_TYPE_COMMAND_POOL: return "Command Pool";
	case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION: return "Sampler YCbCr Conversion";
	case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE: return "Descriptor Update Template";
	case VK_OBJECT_TYPE_SURFACE_KHR: return "Surface";
	case VK_OBJECT_TYPE_SWAPCHAIN_KHR: return "Swapchain";
	case VK_OBJECT_TYPE_DISPLAY_KHR: return "Display";
	case VK_OBJECT_TYPE_DISPLAY_MODE_KHR: return "Display Mode";
	case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT: return "Debug Report Callback";
	case VK_OBJECT_TYPE_VIDEO_SESSION_KHR: return "Video Session";
	case VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR: return "Video Session Parameters";
	case VK_OBJECT_TYPE_CU_MODULE_NVX: return "Cuda Module";
	case VK_OBJECT_TYPE_CU_FUNCTION_NVX: return "Cuda Function";
	case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT: return "Debug Utils Messenger";
	case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR: return "Acceleration Structure";
	case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT: return "Validation Cache";
	case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV: return "Acceleration Structure NV";
	case VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL: return "Performance Configuration";
	case VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR: return "Deferred Operation";
	case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV: return "Indirect Commands Layout";
	case VK_OBJECT_TYPE_PRIVATE_DATA_SLOT_EXT: return "Private Data Slot";
	case VK_OBJECT_TYPE_UNKNOWN: return "Unknown";
	}

	return "Unknown";
}

bool
Vkd_InitDebug(void)
{
	VkDebugUtilsMessengerCreateInfoEXT ci =
	{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
						VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
							VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.pfnUserCallback = _debugCallback
	};
	return vkCreateDebugUtilsMessengerEXT(Vkd_inst, &ci, Vkd_allocCb, &_msg) == VK_SUCCESS;
}

bool
Vkd_SetObjectName(VkDevice dev, void *handle, VkObjectType type, const char *name)
{
	VkDebugUtilsObjectNameInfoEXT info =
	{
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
		.objectHandle = (uint64_t)handle,
		.objectType = type,
		.pObjectName = name
	};
	return vkSetDebugUtilsObjectNameEXT(dev, &info) == VK_SUCCESS;
}

void
Vkd_TermDebug(void)
{
	vkDestroyDebugUtilsMessengerEXT(Vkd_inst, _msg, Vkd_allocCb);
}

static VkBool32 VKAPI_CALL _debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT flags,
	VkDebugUtilsMessageTypeFlagsEXT type,
	const VkDebugUtilsMessengerCallbackDataEXT *data,
	void *userData)
{
	if (!data->messageIdNumber)
		return VK_FALSE;

	const int severity = (flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ? LOG_CRITICAL :
		(flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ? LOG_WARNING :
			(flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT ? LOG_INFORMATION : LOG_DEBUG));

  /*  uint32_t                                     queueLabelCount;
    const VkDebugUtilsLabelEXT*                  pQueueLabels;
    uint32_t                                     cmdBufLabelCount;
    const VkDebugUtilsLabelEXT*                  pCmdBufLabels;*

	Sys_LogEntry(VKDRV_MOD, severity, L"[%d]: %hs", data->messageIdNumber, data->pMessage);

	for (uint32_t i = 0; i < data->objectCount; ++i)
		Sys_LogEntry(VKDRV_MOD, severity, L"\tObject [%llu][%hs][%hs]", data->pObjects[i].objectHandle,
										_objectType(data->pObjects[i].objectType), data->pObjects[i].pObjectName);

	return VK_FALSE;
}*/
