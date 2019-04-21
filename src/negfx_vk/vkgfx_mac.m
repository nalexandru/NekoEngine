/* NekoEngine
 *
 * vk_gfx_unix.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Vulkan Graphics Subsystem
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
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
 */

#import <Cocoa/Cocoa.h>

#include <string.h>

#include <system/log.h>

#define VK_USE_PLATFORM_MACOS_MVK
#include <vkgfx.h>

#define VK_MAC_GFX_MODULE	"Mac_Vulkan_Graphics"

extern NSView *mac_ns_view;

const char *vk_gfx_platform_ext[3] =
{
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
	NULL
};

ne_status
vkgfx_init_surface()
{
	VkResult err = 0;
	VkMacOSSurfaceCreateInfoMVK ci;
	PFN_vkCreateMacOSSurfaceMVK vkCreateMacOSSurfaceMVK = NULL;

	memset(&ci, 0x0, sizeof(ci));

	vkCreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK)
		vkGetInstanceProcAddr(vkgfx_instance, "vkCreateMacOSSurfaceMVK");

	if (!vkCreateMacOSSurfaceMVK) {
		log_entry(VK_MAC_GFX_MODULE, LOG_CRITICAL,
			"Vulkan instance missing VK_MVK_macos_surface extension");
		return NE_FAIL;
	}

	ci.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
	ci.pView = mac_ns_view;

	err = vkCreateMacOSSurfaceMVK(vkgfx_instance, &ci,
								vkgfx_allocator, &vkgfx_surface);
	if (err) {
		log_entry(VK_MAC_GFX_MODULE, LOG_CRITICAL,
			"Failed to create Vulkan surface: %d", err);
		return NE_FAIL;
	}

	return NE_OK;
}

void
vkgfx_release_surface(void)
{
	if (vkgfx_surface != VK_NULL_HANDLE)
		vkDestroySurfaceKHR(vkgfx_instance, vkgfx_surface, vkgfx_allocator);
	vkgfx_surface = VK_NULL_HANDLE;
}
