#include <Engine/Engine.h>

#define VOLK_IMPLEMENTATION
#define VK_USE_PLATFORM_XLIB_KHR
#include "../VulkanBackend.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <System/PlatformDetect.h>

extern Display *X11_display;
extern XVisualInfo X11_visualInfo;
const char *Vkd_PlatformSurfaceExtensionName = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;

bool
Vk_CheckPresentSupport(VkPhysicalDevice dev, uint32_t family)
{
#ifndef SYS_PLATFORM_OPENBSD
	return vkGetPhysicalDeviceXlibPresentationSupportKHR(dev, family, X11_display, X11_visualInfo.visualid);
#else
	return true;
#endif
}

struct NeSurface *
Re_CreateSurface(void *window)
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

	return (struct NeSurface *)surface;
}

/* NekoEngine
 *
 * VkX11.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
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
