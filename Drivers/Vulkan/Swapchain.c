#include <assert.h>
#include <stdlib.h>

#include <System/Log.h>
#include <System/Memory.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Runtime/Runtime.h>

#include "VulkanDriver.h"

static inline bool _Create(VkDevice dev, VkPhysicalDevice physDev, struct Swapchain *sw);

struct Swapchain *
Vk_CreateSwapchain(struct RenderDevice *dev, VkSurfaceKHR surface, bool verticalSync)
{
	VkBool32 present;
	vkGetPhysicalDeviceSurfaceSupportKHR(dev->physDev, dev->graphicsFamily, surface, &present);
	if (!present)
		return NULL;

	struct Swapchain *sw = Sys_Alloc(1, sizeof(*sw), MH_RenderDriver);

	sw->surface = surface;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev->physDev, surface, &sw->surfaceCapabilities);

	sw->imageCount = RE_NUM_FRAMES;
	if (sw->imageCount < sw->surfaceCapabilities.minImageCount)
		sw->imageCount = sw->surfaceCapabilities.minImageCount;

	if (sw->surfaceCapabilities.maxImageCount && sw->imageCount > sw->surfaceCapabilities.maxImageCount)
		sw->imageCount = sw->surfaceCapabilities.maxImageCount;

	uint32_t count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(dev->physDev, surface, &count, NULL);

	VkSurfaceFormatKHR *formats = Sys_Alloc(sizeof(*formats), count, MH_Transient);
	vkGetPhysicalDeviceSurfaceFormatsKHR(dev->physDev, surface, &count, formats);
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

	sw->presentMode = VK_PRESENT_MODE_FIFO_KHR;
	if (!verticalSync) {
		vkGetPhysicalDeviceSurfacePresentModesKHR(dev->physDev, surface, &count, NULL);

		VkPresentModeKHR *pm = Sys_Alloc(sizeof(*pm), count, MH_Transient);
		vkGetPhysicalDeviceSurfacePresentModesKHR(dev->physDev, surface, &count, pm);

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
		if (vkCreateSemaphore(dev->dev, &sci, Vkd_allocCb, &sw->frameStart[i]) != VK_SUCCESS)
			goto error;

		if (vkCreateSemaphore(dev->dev, &sci, Vkd_allocCb, &sw->frameEnd[i]) != VK_SUCCESS)
			goto error;
	}

	sw->imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT  | VK_IMAGE_USAGE_TRANSFER_DST_BIT /*FIXME: suppress VUID-VkRenderPassBeginInfo-framebuffer-04627 | VK_IMAGE_USAGE_TRANSFER_SRC_BIT*/;

	if (!_Create(dev->dev, dev->physDev, sw))
		goto error;

	return sw;

error:
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		if (sw->frameStart[i])
			vkDestroySemaphore(dev->dev, sw->frameStart[i], Vkd_allocCb);

		if (sw->frameEnd[i])
			vkDestroySemaphore(dev->dev, sw->frameEnd[i], Vkd_allocCb);
	}

	Sys_Free(sw);

	return NULL;
}

void
Vk_DestroySwapchain(struct RenderDevice *dev, struct Swapchain *sw)
{
	for (uint32_t i = 0; i < sw->imageCount; ++i)
		vkDestroyImageView(dev->dev, sw->views[i], Vkd_allocCb);

	vkDestroySwapchainKHR(dev->dev, sw->sw, Vkd_allocCb);

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		vkDestroySemaphore(dev->dev, sw->frameEnd[i], Vkd_allocCb);
		vkDestroySemaphore(dev->dev, sw->frameStart[i], Vkd_allocCb);
	}

	Sys_Free(sw->images);
	Sys_Free(sw->views);
	Sys_Free(sw);
}

void *
Vk_AcquireNextImage(struct RenderDevice *dev, struct Swapchain *sw)
{
	uint32_t imageId;
	VkResult rc = vkAcquireNextImageKHR(dev->dev, sw->sw, UINT64_MAX, sw->frameStart[Re_frameId], VK_NULL_HANDLE, &imageId);
	if (rc != VK_SUCCESS) {
		switch (rc) {
		case VK_SUBOPTIMAL_KHR:
		case VK_ERROR_OUT_OF_DATE_KHR:
			_Create(dev->dev, dev->physDev, sw);
			return RE_INVALID_IMAGE;
		default: return RE_INVALID_IMAGE;
		}
	}

	VkSemaphoreWaitInfo waitInfo =
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
		.semaphoreCount = 1,
		.pSemaphores = &dev->frameSemaphore,
		.pValues = &dev->frameValues[Re_frameId]
	};
	vkWaitSemaphores(dev->dev, &waitInfo, UINT64_MAX);

	if (!Re_deviceInfo.features.coherentMemory)
		Vkd_CommitStagingArea(dev, sw->frameStart[Re_frameId]);

	return (void *)(uint64_t)imageId;
}

bool
Vk_Present(struct RenderDevice *dev, struct RenderContext *ctx, struct Swapchain *sw, void *image)
{
	dev->frameValues[Re_frameId] = ++dev->semaphoreValue;

	uint64_t waitValues[] = { 0 };
	uint64_t signalValues[] = { dev->frameValues[Re_frameId], 0 };
	VkSemaphore wait[] = { sw->frameStart[Re_frameId] };
	VkSemaphore signal[] = { dev->frameSemaphore, sw->frameEnd[Re_frameId] };
	VkPipelineStageFlags waitMasks[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };

	if (!Re_deviceInfo.features.coherentMemory)
		wait[0] = Vkd_stagingSignal;

	VkTimelineSemaphoreSubmitInfo timelineInfo =
	{
		.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
		.waitSemaphoreValueCount = 1,
		.pWaitSemaphoreValues = waitValues,
		.signalSemaphoreValueCount = 2,
		.pSignalSemaphoreValues = signalValues
	};
	VkSubmitInfo si =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = &timelineInfo,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = wait,
		.pWaitDstStageMask = waitMasks,
		.commandBufferCount = (uint32_t)ctx->graphicsCmdBuffers[Re_frameId].count,
		.pCommandBuffers = (const VkCommandBuffer *)ctx->graphicsCmdBuffers[Re_frameId].data,
		.signalSemaphoreCount = 2,
		.pSignalSemaphores = signal
	};
	vkQueueSubmit(dev->graphicsQueue, 1, &si, VK_NULL_HANDLE);

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
	VkResult rc = vkQueuePresentKHR(dev->graphicsQueue, &pi);

	switch (rc) {
	case VK_SUCCESS: return true;
	case VK_SUBOPTIMAL_KHR:
	case VK_ERROR_OUT_OF_DATE_KHR:
		return _Create(dev->dev, dev->physDev, sw);
	default: return false;
	}

	return rc == VK_SUCCESS;
}

enum TextureFormat
Vk_SwapchainFormat(struct Swapchain *sw)
{
	return VkToNeTextureFormat(sw->surfaceFormat.format);
}

void
Vk_SwapchainDesc(struct Swapchain *sw, struct FramebufferAttachmentDesc *desc)
{
	desc->format = VkToNeTextureFormat(sw->surfaceFormat.format);
	desc->usage = sw->imageUsage;
}

struct Texture *
Vk_SwapchainTexture(struct Swapchain *sw, void *image)
{
	uint32_t id = (uint32_t)(uint64_t)image;

	struct Texture *t = Sys_Alloc(sizeof(*t), 1, MH_Transient);
	t->image = sw->images[id];
	t->imageView = sw->views[id];

	return t;
}

void
Vk_ScreenResized(struct RenderDevice *dev, struct Swapchain *sw)
{
	if (!_Create(dev->dev, dev->physDev, sw)) {
		Sys_LogEntry(VKDRV_MOD, LOG_CRITICAL, L"Failed to resize swapchain");
		E_Shutdown();
	}
}

static inline bool
_Create(VkDevice dev, VkPhysicalDevice physDev, struct Swapchain *sw)
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
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
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
	if (vkCreateSwapchainKHR(dev, &createInfo, Vkd_allocCb, &new) != VK_SUCCESS)
		return false;

	sw->sw = new;

	uint32_t count;
	vkGetSwapchainImagesKHR(dev, sw->sw, &count, NULL);

	if (sw->imageCount != count || !sw->images || !sw->views) {
		void *ptr;

		ptr = Sys_ReAlloc(sw->images, count, sizeof(*sw->images), MH_RenderDriver);
		assert(ptr); sw->images = ptr;

		ptr = Sys_ReAlloc(sw->views, count, sizeof(*sw->views), MH_RenderDriver);
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
