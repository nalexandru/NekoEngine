#include "VulkanBackend.h"

void
Re_AppendXrExtensions(struct NeArray *a)
{
#if ENABLE_OPENXR
	Rt_ArrayAddPtr(a, XR_KHR_VULKAN_ENABLE_EXTENSION_NAME);
#endif
}

void *
Re_XrGraphicsBinding(void)
{
#if ENABLE_OPENXR
	XrGraphicsBindingVulkanKHR *gb = Sys_Alloc(sizeof(*gb), 1, MH_Transient);

	gb->type = XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR;
	gb->instance = Vkd_inst;
	gb->device = Re_device->dev;
	gb->physicalDevice = Re_device->physDev;
	gb->queueFamilyIndex = Re_device->graphicsFamily;
	gb->queueIndex = 0;

	return gb;
#else
	return NULL;
#endif
}
