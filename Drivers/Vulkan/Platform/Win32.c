#include <Engine/Engine.h>

#define VOLK_IMPLEMENTATION
#define VK_USE_PLATFORM_WIN32_KHR
#include "../VulkanDriver.h"

#include <Windows.h>

const char *PlatformSurfaceExtensionName = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;

bool
Vk_CheckPresentSupport(VkPhysicalDevice dev, uint32_t family)
{
	return vkGetPhysicalDeviceWin32PresentationSupportKHR(dev, family);
}

VkSurfaceKHR
Vk_CreateSurface(struct RenderDevice *dev, void *window)
{
	VkWin32SurfaceCreateInfoKHR surfaceInfo =
	{
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hinstance = GetModuleHandle(NULL),
		.hwnd = (HWND)window
	};

	VkSurfaceKHR surface;
	VkResult rc = vkCreateWin32SurfaceKHR(Vkd_inst, &surfaceInfo, Vkd_allocCb, &surface);
	if (rc != VK_SUCCESS)
		return NULL;

	return surface;
}
