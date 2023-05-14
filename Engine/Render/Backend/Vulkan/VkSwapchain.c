#include <assert.h>
#include <stdlib.h>

#include <System/Log.h>
#include <System/Memory.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Runtime/Runtime.h>

#include "VulkanBackend.h"

#define VK_SWMOD	"VulkanSwapchain"

static inline bool Create(VkDevice dev, VkPhysicalDevice physDev, struct NeSwapchain *sw);

struct NeSwapchain *
Re_CreateSwapchain(struct NeSurface *surface, bool verticalSync)
{
	VkBool32 present;
	vkGetPhysicalDeviceSurfaceSupportKHR(Re_device->physDev, Re_device->graphics.family, (VkSurfaceKHR)surface, &present);
	if (!present)
		return NULL;

	struct NeSwapchain *sw = Sys_Alloc(1, sizeof(*sw), MH_RenderBackend);

	sw->surface = (VkSurfaceKHR)surface;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Re_device->physDev, (VkSurfaceKHR)surface, &sw->surfaceCapabilities);

	sw->imageCount = RE_NUM_FRAMES;
	if (sw->imageCount < sw->surfaceCapabilities.minImageCount)
		sw->imageCount = sw->surfaceCapabilities.minImageCount;

	if (sw->surfaceCapabilities.maxImageCount && sw->imageCount > sw->surfaceCapabilities.maxImageCount)
		sw->imageCount = sw->surfaceCapabilities.maxImageCount;

	uint32_t count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(Re_device->physDev, (VkSurfaceKHR)surface, &count, NULL);

	VkSurfaceFormatKHR *formats = Sys_Alloc(sizeof(*formats), count, MH_Transient);
	vkGetPhysicalDeviceSurfaceFormatsKHR(Re_device->physDev, (VkSurfaceKHR)surface, &count, formats);
	for (uint32_t i = 0; i < count; ++i) {
		if (formats[i].format == VK_FORMAT_R16G16B16A16_SFLOAT) {
			sw->surfaceFormat = formats[i];
			break;
		}

		if (formats[i].format == VK_FORMAT_R8G8B8A8_UNORM)
			sw->surfaceFormat = formats[i];
		else if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && sw->surfaceFormat.format != VK_FORMAT_R8G8B8A8_UNORM)
			sw->surfaceFormat = formats[i];
	}

	sw->presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
	if (!verticalSync) {
		vkGetPhysicalDeviceSurfacePresentModesKHR(Re_device->physDev, (VkSurfaceKHR)surface, &count, NULL);

		VkPresentModeKHR *pm = Sys_Alloc(sizeof(*pm), count, MH_Transient);
		vkGetPhysicalDeviceSurfacePresentModesKHR(Re_device->physDev, (VkSurfaceKHR)surface, &count, pm);

		for (uint32_t i = 0; i < count; ++i) {
			if (pm[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
				sw->presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}

			if (sw->presentMode != VK_PRESENT_MODE_MAILBOX_KHR && pm[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
				sw->presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	VkSemaphoreCreateInfo sci = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		if (vkCreateSemaphore(Re_device->dev, &sci, Vkd_allocCb, &sw->frameStart[i]) != VK_SUCCESS)
			goto error;
		VkBk_SetObjectName(Re_device->dev, sw->frameStart[i], VK_OBJECT_TYPE_SEMAPHORE, "Frame start");

		if (vkCreateSemaphore(Re_device->dev, &sci, Vkd_allocCb, &sw->frameEnd[i]) != VK_SUCCESS)
			goto error;
		VkBk_SetObjectName(Re_device->dev, sw->frameEnd[i], VK_OBJECT_TYPE_SEMAPHORE, "Frame end");
	}

	sw->imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

#ifdef SYS_PLATFORM_WINDOWS
	sw->imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;	// for some reason, the validation layers add this on Windows
#endif

	if (!Create(Re_device->dev, Re_device->physDev, sw))
		goto error;

	return sw;

error:
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		if (sw->frameStart[i])
			vkDestroySemaphore(Re_device->dev, sw->frameStart[i], Vkd_allocCb);

		if (sw->frameEnd[i])
			vkDestroySemaphore(Re_device->dev, sw->frameEnd[i], Vkd_allocCb);
	}

	Sys_Free(sw);

	return NULL;
}

void
Re_DestroySwapchain(struct NeSwapchain *sw)
{
	for (uint32_t i = 0; i < sw->imageCount; ++i)
		vkDestroyImageView(Re_device->dev, sw->views[i], Vkd_allocCb);

	vkDestroySwapchainKHR(Re_device->dev, sw->sw, Vkd_allocCb);

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		vkDestroySemaphore(Re_device->dev, sw->frameEnd[i], Vkd_allocCb);
		vkDestroySemaphore(Re_device->dev, sw->frameStart[i], Vkd_allocCb);
	}

	Sys_Free(sw->images);
	Sys_Free(sw->views);
	Sys_Free(sw);
}

void *
Re_AcquireNextImage(struct NeSwapchain *sw)
{
	VkSemaphoreWaitInfo waitInfo =
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
		.semaphoreCount = 1,
		.pSemaphores = &Re_device->frameSemaphore,
		.pValues = &Re_device->frameValues[Re_frameId]
	};
	vkWaitSemaphores(Re_device->dev, &waitInfo, UINT64_MAX);

	uint32_t imageId;
	VkResult rc = vkAcquireNextImageKHR(Re_device->dev, sw->sw, UINT64_MAX, sw->frameStart[Re_frameId], VK_NULL_HANDLE, &imageId);
	if (rc != VK_SUCCESS) {
		switch (rc) {
		case VK_SUBOPTIMAL_KHR:
		case VK_ERROR_OUT_OF_DATE_KHR:
			Create(Re_device->dev, Re_device->physDev, sw);
			return RE_INVALID_IMAGE;
		default: return RE_INVALID_IMAGE;
		}
	}

	if (!Re_deviceInfo.features.coherentMemory)
		VkBk_CommitStagingArea(Re_device, sw->frameStart[Re_frameId]);

	vkResetDescriptorPool(Re_device->dev, Re_device->iaDescriptorPool[Re_frameId], 0);

	return (void *)(uint64_t)imageId;
}

bool
Re_Present(struct NeSwapchain *sw, void *image, struct NeSemaphore *waitSemaphore)
{
	VkBk_ExecuteCommands(sw, waitSemaphore);

	uint32_t imageId = (uint32_t)(uint64_t)image;

	VkPresentInfoKHR pi =
	{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &sw->frameEnd[Re_frameId],
		.swapchainCount = 1,
		.pSwapchains = &sw->sw,
		.pImageIndices = &imageId
	};
	VkResult rc = vkQueuePresentKHR(Re_device->graphics.queue, &pi);

	switch (rc) {
	case VK_SUCCESS: return true;
	case VK_SUBOPTIMAL_KHR:
	case VK_ERROR_OUT_OF_DATE_KHR:
		return Create(Re_device->dev, Re_device->physDev, sw);
	default: return false;
	}

	return rc == VK_SUCCESS;
}

enum NeTextureFormat
Re_SwapchainFormat(struct NeSwapchain *sw)
{
	return VkToNeTextureFormat(sw->surfaceFormat.format);
}

void
Re_SwapchainDesc(struct NeSwapchain *sw, struct NeFramebufferAttachmentDesc *desc)
{
	desc->format = VkToNeTextureFormat(sw->surfaceFormat.format);
	desc->usage = sw->imageUsage;
}

void
Re_SwapchainTextureDesc(struct NeSwapchain *sw, struct NeTextureDesc *desc)
{
	desc->width = *E_screenWidth;
	desc->height = *E_screenHeight;
	desc->depth = 1;
	desc->format = VkToNeTextureFormat(sw->surfaceFormat.format);
	desc->usage = sw->imageUsage;
	desc->type = TT_2D;
	desc->arrayLayers = 1;
	desc->mipLevels = 1;
	desc->gpuOptimalTiling = true;
	desc->memoryType = MT_GPU_LOCAL;
}

struct NeTexture *
Re_SwapchainTexture(struct NeSwapchain *sw, void *image)
{
	uint32_t id = (uint32_t)(uint64_t)image;

	struct NeTexture *t = Sys_Alloc(sizeof(*t), 1, MH_Transient);
	t->image = sw->images[id];
	t->imageView = sw->views[id];

	return t;
}

void
Re_ScreenResized(struct NeSwapchain *sw)
{
	if (!Create(Re_device->dev, Re_device->physDev, sw)) {
		Sys_LogEntry(VKDRV_MOD, LOG_CRITICAL, "Failed to resize swapchain");
		E_Shutdown();
	}
}

static inline bool
Create(VkDevice dev, VkPhysicalDevice physDev, struct NeSwapchain *sw)
{
	vkDeviceWaitIdle(dev);

	VkSwapchainCreateInfoKHR createInfo =
	{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = sw->surface,
		.minImageCount = sw->imageCount,
		.imageFormat = sw->surfaceFormat.format,
		.imageColorSpace = sw->surfaceFormat.colorSpace,
		.imageExtent = { *E_screenWidth, *E_screenHeight },
		.imageArrayLayers = 1,
		.imageUsage = sw->imageUsage,
		.preTransform = sw->surfaceCapabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.clipped = false,
		.oldSwapchain = sw->sw,
		.presentMode = sw->presentMode
	};

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDev, sw->surface, &sw->surfaceCapabilities);

	if (createInfo.imageExtent.width < sw->surfaceCapabilities.minImageExtent.width)
		createInfo.imageExtent.width = sw->surfaceCapabilities.minImageExtent.width;
	else if (createInfo.imageExtent.width > sw->surfaceCapabilities.maxImageExtent.width)
		createInfo.imageExtent.width = sw->surfaceCapabilities.maxImageExtent.width;

	if (createInfo.imageExtent.height < sw->surfaceCapabilities.minImageExtent.height)
		createInfo.imageExtent.height = sw->surfaceCapabilities.minImageExtent.height;
	else if (createInfo.imageExtent.height > sw->surfaceCapabilities.maxImageExtent.height)
		createInfo.imageExtent.height = sw->surfaceCapabilities.maxImageExtent.height;

	VkSwapchainKHR new;
	VkResult rc = vkCreateSwapchainKHR(dev, &createInfo, Vkd_allocCb, &new);
	if (rc != VK_SUCCESS) {
		Sys_LogEntry(VK_SWMOD, LOG_CRITICAL, "Failed to create swapchain: 0x%x", rc);
		return false;
	}

	sw->sw = new;

	uint32_t count;
	vkGetSwapchainImagesKHR(dev, sw->sw, &count, NULL);

	if (sw->imageCount != count || !sw->images || !sw->views) {
		void *ptr;

		ptr = Sys_ReAlloc(sw->images, count, sizeof(*sw->images), MH_RenderBackend);
		assert(ptr); sw->images = ptr;

		ptr = Sys_ReAlloc(sw->views, count, sizeof(*sw->views), MH_RenderBackend);
		assert(ptr); sw->views = ptr;

		sw->imageCount = count;
	}

	vkGetSwapchainImagesKHR(dev, sw->sw, &sw->imageCount, sw->images);

	VkImageViewCreateInfo viewInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.format = sw->surfaceFormat.format,
		.components =
		{
			.r = VK_COMPONENT_SWIZZLE_R,
			.g = VK_COMPONENT_SWIZZLE_G,
			.b = VK_COMPONENT_SWIZZLE_B,
			.a = VK_COMPONENT_SWIZZLE_A
		},
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		},
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.flags = 0
	};

	for (uint32_t i = 0; i < sw->imageCount; ++i) {
		viewInfo.image = sw->images[i];
		if (vkCreateImageView(dev, &viewInfo, Vkd_allocCb, &sw->views[i]) != VK_SUCCESS)
			return false;
	}

	return true;
}

/* NekoEngine
 *
 * VkSwapchain.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
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
