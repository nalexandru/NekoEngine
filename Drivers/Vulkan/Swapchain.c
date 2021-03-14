#include <assert.h>
#include <stdlib.h>

#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <System/Memory.h>
#include <Runtime/Runtime.h>
#include <Render/Swapchain.h>

#include "VulkanDriver.h"

static inline bool _Create(VkDevice dev, struct Swapchain *sw);

struct Swapchain *
Vk_CreateSwapchain(struct RenderDevice *dev, VkSurfaceKHR surface)
{
	VkBool32 present;
	vkGetPhysicalDeviceSurfaceSupportKHR(dev->physDev, dev->graphicsFamily, surface, &present);
	if (!present)
		return NULL;

	struct Swapchain *sw = calloc(1, sizeof(*sw));
	
	sw->surface = surface;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev->physDev, surface, &sw->surfaceCapabilities);

	sw->imageCount = 3;
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
	if (!E_GetCVarBln(L"Render_VerticalSync", false)->bln) {
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
	if (vkCreateSemaphore(dev->dev, &sci, Vkd_allocCb, &sw->frameStart) != VK_SUCCESS)
		goto error;

	if (vkCreateSemaphore(dev->dev, &sci, Vkd_allocCb, &sw->frameEnd) != VK_SUCCESS)
		goto error;

	if (!_Create(dev->dev, sw))
		goto error;

	return sw;

error:
	if (sw->frameStart)
		vkDestroySemaphore(dev->dev, sw->frameStart, Vkd_allocCb);
		
	if (sw->frameEnd)
		vkDestroySemaphore(dev->dev, sw->frameEnd, Vkd_allocCb);

	free(sw);
	return NULL;
}

void
Vk_DestroySwapchain(struct RenderDevice *dev, struct Swapchain *sw)
{
	vkDestroySemaphore(dev->dev, sw->frameEnd, Vkd_allocCb);
	vkDestroySemaphore(dev->dev, sw->frameStart, Vkd_allocCb);
	for (uint32_t i = 0; i < sw->imageCount; ++i)
		vkDestroyImageView(dev->dev, sw->views[i], Vkd_allocCb);
	vkDestroySwapchainKHR(dev->dev, sw->sw, Vkd_allocCb);

	free(sw->images);
	free(sw->views);
	free(sw);
}

void *
Vk_AcquireNextImage(struct RenderDevice *dev, struct Swapchain *sw)
{
	uint32_t imageId;
	VkResult rc = vkAcquireNextImageKHR(dev->dev, sw->sw, UINT64_MAX, sw->frameStart, VK_NULL_HANDLE, &imageId);
	if (rc != VK_SUCCESS) {
		switch (rc) {
		case VK_SUBOPTIMAL_KHR:
		case VK_ERROR_OUT_OF_DATE_KHR:
			_Create(dev->dev, sw);
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

	struct RenderContext *ctx;
	Rt_ArrayForEachPtr(ctx, &Vkd_contexts) {
		if (ctx->graphicsCmdBuffers[Re_frameId].count) {
			vkFreeCommandBuffers(dev->dev, ctx->graphicsPools[Re_frameId],
				(uint32_t)ctx->graphicsCmdBuffers[Re_frameId].count, (const VkCommandBuffer *)ctx->graphicsCmdBuffers[Re_frameId].data);
			vkResetCommandPool(dev->dev, ctx->graphicsPools[Re_frameId], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
			Rt_ClearArray(&ctx->graphicsCmdBuffers[Re_frameId], false);
		}
	
		if (ctx->transferCmdBuffers[Re_frameId].count) {
			vkFreeCommandBuffers(dev->dev, ctx->transferPools[Re_frameId],
				(uint32_t)ctx->transferCmdBuffers[Re_frameId].count, (const VkCommandBuffer *)ctx->transferCmdBuffers[Re_frameId].data);
			vkResetCommandPool(dev->dev, ctx->transferPools[Re_frameId], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
			Rt_ClearArray(&ctx->transferCmdBuffers[Re_frameId], false);
		}
	
		if (ctx->computeCmdBuffers[Re_frameId].count) {
			vkFreeCommandBuffers(dev->dev, ctx->computePools[Re_frameId],
				(uint32_t)ctx->computeCmdBuffers[Re_frameId].count, (const VkCommandBuffer *)ctx->computeCmdBuffers[Re_frameId].data);
			vkResetCommandPool(dev->dev, ctx->computePools[Re_frameId], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
			Rt_ClearArray(&ctx->computeCmdBuffers[Re_frameId], false);
		}
	}

	return (void *)(uint64_t)imageId;
}

bool
Vk_Present(struct RenderDevice *dev, struct RenderContext *ctx, struct Swapchain *sw, void *image)
{
	dev->frameValues[Re_frameId] = ++dev->semaphoreValue;

	uint64_t waitValues[] = { 0 };
	uint64_t signalValues[] = { dev->frameValues[Re_frameId], 0 };
	VkSemaphore wait[] = { sw->frameStart };
	VkSemaphore signal[] = { dev->frameSemaphore, sw->frameEnd };
	VkPipelineStageFlags waitMasks[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };

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
		.pWaitSemaphores = &sw->frameEnd,
		.swapchainCount = 1,
		.pSwapchains = &sw->sw,
		.pImageIndices = &imageId
	};
	VkResult rc = vkQueuePresentKHR(dev->graphicsQueue, &pi);

	switch (rc) {
	case VK_SUCCESS: return true;
	case VK_SUBOPTIMAL_KHR:
	case VK_ERROR_OUT_OF_DATE_KHR:
		return _Create(dev->dev, sw);
	default: return false;
	}

	return rc == VK_SUCCESS;
}

enum TextureFormat
Vk_SwapchainFormat(struct Swapchain *sw)
{
	return VkToNeTextureFormat(sw->surfaceFormat.format);
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

static inline bool
_Create(VkDevice dev, struct Swapchain *sw)
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
		.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.clipped = false,
		.oldSwapchain = sw->sw,
		.presentMode = sw->presentMode
	};

	VkSwapchainKHR new;
	if (vkCreateSwapchainKHR(dev, &createInfo, Vkd_allocCb, &new) != VK_SUCCESS)
		return false;

	sw->sw = new;

	uint32_t count;
	vkGetSwapchainImagesKHR(dev, sw->sw, &count, NULL);

	if (sw->imageCount != count || !sw->images || !sw->views) {
		void *ptr;
		
		ptr = reallocarray(sw->images, count, sizeof(*sw->images));
		assert(ptr); sw->images = ptr;

		ptr = reallocarray(sw->views, count, sizeof(*sw->views));
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
		assert(vkCreateImageView(dev, &viewInfo, Vkd_allocCb, &sw->views[i]) == VK_SUCCESS);
	}

	return true;
}

