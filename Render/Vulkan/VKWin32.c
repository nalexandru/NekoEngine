#include <Windows.h>
#include <Engine/Engine.h>

#define VK_USE_PLATFORM_WIN32_KHR
#define VOLK_IMPLEMENTATION

#include "VKRender.h"

const char *__VK_PlatformSurfaceExtension = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;

bool
VK_CheckPresentSupport(VkPhysicalDevice dev, uint32_t family)
{
	return vkGetPhysicalDeviceWin32PresentationSupportKHR(dev, family);
}

bool
VK_CreateSurface(void)
{
	VkWin32SurfaceCreateInfoKHR sci =
	{
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hinstance = GetModuleHandle(NULL),
		.hwnd = (HWND)E_Screen
	};
	VkResult rc = vkCreateWin32SurfaceKHR(VK_Instance, &sci, VK_CPUAllocator, &VK_Swapchain.surface);
	return rc == VK_SUCCESS;
}