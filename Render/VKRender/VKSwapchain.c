#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <System/Memory.h>

#include "VKRender.h"

/*	VkSwapchainKHR sw;
	VkSurfaceKHR surface;
	VkExtent2D extent;
	VkSurfaceFormatKHR format;
	VkSurfaceCapabilitiesKHR caps;
	VkPresentModeKHR presentMode;
	VkImage *images;
	VkImageView *views;
	uint32_t imgCount;*/

struct Swapchain VK_Swapchain;

bool
VK_CreateSwapchain(void)
{
	vkDeviceWaitIdle(Re_Device.dev);

	if (!VK_Swapchain.sw) {
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Re_Device.physicalDevice, VK_Swapchain.surface, &VK_Swapchain.caps);

		VK_Swapchain.imgCount = RE_NUM_BUFFERS;
		if (VK_Swapchain.imgCount < VK_Swapchain.caps.minImageCount)
			VK_Swapchain.imgCount = VK_Swapchain.caps.minImageCount;

		if (VK_Swapchain.caps.maxImageCount && VK_Swapchain.imgCount > VK_Swapchain.caps.maxImageCount)
			VK_Swapchain.imgCount = VK_Swapchain.caps.maxImageCount;

		uint32_t count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(Re_Device.physicalDevice, VK_Swapchain.surface, &count, NULL);

		VkSurfaceFormatKHR *sf = Sys_Alloc(sizeof(*sf), count, MH_Transient);
		vkGetPhysicalDeviceSurfaceFormatsKHR(Re_Device.physicalDevice, VK_Swapchain.surface, &count, sf);

		VK_Swapchain.format.format = VK_FORMAT_UNDEFINED;
		for (uint32_t i = 0; i < count; ++i) {
			if (sf[i].format == VK_FORMAT_R16G16B16A16_SFLOAT) {
				VK_Swapchain.format = sf[i];
				break;
			}

			if (sf[i].format == VK_FORMAT_A2B10G10R10_UNORM_PACK32)
				VK_Swapchain.format = sf[i];

			/*	if (sw->fmt.format != VK_FORMAT_A2B10G10R10_UNORM_PACK32 &&
						fmts[i].surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB) {
					sw->fmt = fmts[i].surfaceFormat;
				}*/

			if (VK_Swapchain.format.format != VK_FORMAT_A2B10G10R10_UNORM_PACK32 &&
			//		sw->fmt.format != VK_FORMAT_B8G8R8A8_SRGB &&
					sf[i].format == VK_FORMAT_B8G8R8A8_UNORM) {
				VK_Swapchain.format = sf[i];
			}
		}

		VK_Swapchain.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		if (!E_GetCVarBln(L"Render_VerticalSync", false)->bln) {
			uint32_t count = 0;
			vkGetPhysicalDeviceSurfacePresentModesKHR(Re_Device.physicalDevice, VK_Swapchain.surface, &count, NULL);

			VkPresentModeKHR *pm = Sys_Alloc(sizeof(*pm), count, MH_Transient);
			vkGetPhysicalDeviceSurfacePresentModesKHR(Re_Device.physicalDevice, VK_Swapchain.surface, &count, pm);

			for (uint32_t i = 0; i < count; ++i) {
				if (pm[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
					VK_Swapchain.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}

				if (VK_Swapchain.presentMode != VK_PRESENT_MODE_MAILBOX_KHR && pm[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
					VK_Swapchain.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
	}

	VkSwapchainCreateInfoKHR ci =
	{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = VK_Swapchain.surface,
		.minImageCount = VK_Swapchain.imgCount,
		.imageFormat = VK_Swapchain.format.format,
		.imageColorSpace = VK_Swapchain.format.colorSpace,
		.imageExtent = { *E_ScreenWidth, *E_ScreenHeight },
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.clipped = false,
		.oldSwapchain = VK_Swapchain.sw,
		.presentMode = VK_Swapchain.presentMode
	};

	VkSwapchainKHR new;
	vkCreateSwapchainKHR(Re_Device.dev, &ci, NULL, &new);

	if (VK_Swapchain.sw)
		vkDestroySwapchainKHR(Re_Device.dev, VK_Swapchain.sw, NULL);
	VK_Swapchain.sw = new;

	vkGetSwapchainImagesKHR(Re_Device.dev, VK_Swapchain.sw, &VK_Swapchain.imgCount, NULL);

	VK_Swapchain.images = calloc(VK_Swapchain.imgCount, sizeof(*VK_Swapchain.images));
	VK_Swapchain.views = calloc(VK_Swapchain.imgCount, sizeof(*VK_Swapchain.views));
	VK_Swapchain.imgRdy = calloc(VK_Swapchain.imgCount, sizeof(*VK_Swapchain.imgRdy));
	VK_Swapchain.imgDone = calloc(VK_Swapchain.imgCount, sizeof(*VK_Swapchain.imgDone));

	if (!VK_Swapchain.images || !VK_Swapchain.views || !VK_Swapchain.imgRdy || !VK_Swapchain.imgDone)
		return false;

	vkGetSwapchainImagesKHR(Re_Device.dev, VK_Swapchain.sw, &VK_Swapchain.imgCount, VK_Swapchain.images);

	VkImageViewCreateInfo ivci =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.format = VK_Swapchain.format.format,
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

	VkSemaphoreCreateInfo sci = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

	for (uint32_t i = 0; i < VK_Swapchain.imgCount; ++i) {
		ivci.image = VK_Swapchain.images[i];

		vkCreateImageView(Re_Device.dev, &ivci, NULL, &VK_Swapchain.views[i]);
		vkCreateSemaphore(Re_Device.dev, &sci, NULL, &VK_Swapchain.imgRdy[i]);
		vkCreateSemaphore(Re_Device.dev, &sci, NULL, &VK_Swapchain.imgDone[i]);
	}

	VkCommandBufferAllocateInfo cbai =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandBufferCount = 1,
		.commandPool = Re_MainThreadWorker.graphicsCmdPools[Re_Device.frame],
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY
	};
	VkCommandBuffer cmdBuff;
	vkAllocateCommandBuffers(Re_Device.dev, &cbai, &cmdBuff);

	VkCommandBufferBeginInfo cbbi =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	vkBeginCommandBuffer(cmdBuff, &cbbi);

	for (uint32_t i = 0; i < VK_Swapchain.imgCount; ++i)
		VK_TransitionImage(VK_Swapchain.images[i], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT, cmdBuff);

	vkEndCommandBuffer(cmdBuff);

	VkFence fence;
	VkFenceCreateInfo fci = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	vkCreateFence(Re_Device.dev, &fci, NULL, &fence);

	VkPipelineStageFlags wait = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkSubmitInfo si =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmdBuff,
		.pWaitDstStageMask = &wait
	};
	vkQueueSubmit(Re_Device.graphicsQueue, 1, &si, fence);

	vkWaitForFences(Re_Device.dev, 1, &fence, VK_TRUE, UINT64_MAX);

	vkDestroyFence(Re_Device.dev, fence, NULL);
	vkFreeCommandBuffers(1, Re_MainThreadWorker.graphicsCmdPools[Re_Device.frame], 1, &cmdBuff);

	return true;
}
