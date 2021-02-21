#include <Engine/Engine.h>

#define VOLK_IMPLEMENTATION
#define VK_USE_PLATFORM_XLIB_KHR
#include "../VulkanDriver.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

extern Display *X11_display;
extern XVisualInfo X11_visualInfo;
const char *PlatformSurfaceExtensionName = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;

bool
Vk_CheckPresentSupport(VkPhysicalDevice dev, uint32_t family)
{
	return vkGetPhysicalDeviceXlibPresentationSupportKHR(dev, family, X11_display, X11_visualInfo.visualid);
}

VkSurfaceKHR
Vk_CreateSurface(struct RenderDevice *dev, void *window)
{
	VkXlibSurfaceCreateInfoKHR surfaceInfo =
	{
		.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
		.dpy = X11_display,
		.window = (Window)window
	};

	VkSurfaceKHR surface;
	VkResult rc = vkCreateXlibSurfaceKHR(Vkd_inst, &surfaceInfo, Vkd_allocCb, &surface);
	if (rc != VK_SUCCESS)
		return NULL;

	return surface;
}


