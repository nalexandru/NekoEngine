/* NekoEngine
 *
 * swapchain.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Vulkan Graphics Module
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

#include <assert.h>
#include <string.h>

#include <vulkan/vulkan.h>

#include <system/log.h>
#include <system/config.h>
#include <runtime/runtime.h>

#include <vkgfx.h>
#include <vkutil.h>
#include <swapchain.h>
#include <renderpass.h>

#define VK_SW_MODULE	"VulkanSwapchain"

struct vk_gfx_swapchain
{
	VkSwapchainKHR swapchain;
	VkExtent2D extent;
	VkSurfaceFormatKHR fmt;
	VkSurfaceCapabilitiesKHR caps;
	VkPresentModeKHR present_mode;
	int32_t gfx_fam;
	int32_t present_fam;
	rt_array images;
	rt_array image_views;
	rt_array framebuffers;
};

static struct vk_gfx_swapchain vk_swapchain;

static inline VkResult
_sw_create(
	uint16_t width,
	uint16_t height)
{
	VkResult res = VK_SUCCESS;

	if (width != 0)
		vk_swapchain.extent.width = width;

	if (height != 0)
		vk_swapchain.extent.height = height;

	vkDeviceWaitIdle(vkgfx_device);

	VkSwapchainCreateInfoKHR ci;
	memset(&ci, 0x0, sizeof(ci));

	uint32_t img_count = 3;
	if (img_count < vk_swapchain.caps.minImageCount)
		img_count = vk_swapchain.caps.minImageCount + 1;
	if (img_count > vk_swapchain.caps.maxImageCount)
		img_count = vk_swapchain.caps.maxImageCount;

	ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	ci.surface = vkgfx_surface;
	ci.minImageCount = img_count;
	ci.imageFormat = vk_swapchain.fmt.format;
	ci.imageColorSpace = vk_swapchain.fmt.colorSpace;
	ci.imageExtent = vk_swapchain.extent;
	ci.imageArrayLayers = 1;
	ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	uint32_t qfi[2] =
	{
		vk_swapchain.gfx_fam,
		vk_swapchain.present_fam
	};

	if (qfi[0] != qfi[1]) {
		ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		ci.queueFamilyIndexCount = 2;
		ci.pQueueFamilyIndices = qfi;
	} else {
		ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ci.queueFamilyIndexCount = 0;
		ci.pQueueFamilyIndices = NULL;
	}

	ci.preTransform = vk_swapchain.caps.currentTransform;
	ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	ci.presentMode = vk_swapchain.present_mode;
	ci.clipped = VK_TRUE;
	ci.oldSwapchain = vk_swapchain.swapchain;

	VkSwapchainKHR new;
	if ((res = vkCreateSwapchainKHR(vkgfx_device, &ci, vkgfx_allocator,
		&new)) != VK_SUCCESS) {
		log_entry(VK_SW_MODULE, LOG_CRITICAL,
			"Failed to create swapchain: %s",
			vku_result_string(res));
		return res;
	}

	vkDestroySwapchainKHR(vkgfx_device, vk_swapchain.swapchain, vkgfx_allocator);
	vk_swapchain.swapchain = new;

	if ((vkGetSwapchainImagesKHR(vkgfx_device, vk_swapchain.swapchain,
		&img_count, NULL)) != VK_SUCCESS) {
		log_entry(VK_SW_MODULE, LOG_CRITICAL,
			"Failed to retrieve swapchain images: %s",
			vku_result_string(res));
		return res;
	}

	rt_array_resize(&vk_swapchain.images, img_count);
	rt_array_fill(&vk_swapchain.images);
	if ((res = vkGetSwapchainImagesKHR(vkgfx_device, vk_swapchain.swapchain,
		&img_count, (VkImage *)vk_swapchain.images.data))
		!= VK_SUCCESS) {
		log_entry(VK_SW_MODULE, LOG_CRITICAL,
			"Failed to retrieve swapchain images: %s",
			vku_result_string(res));
		return res;
	}

	assert(img_count <= VKGFX_MAX_SWAPCHAIN_IMAGES);

	VkImageViewCreateInfo ivci;
	memset(&ivci, 0x0, sizeof(ivci));
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.pNext = NULL;
	ivci.format = vk_swapchain.fmt.format;
	ivci.components.r = VK_COMPONENT_SWIZZLE_R;
	ivci.components.g = VK_COMPONENT_SWIZZLE_G;
	ivci.components.b = VK_COMPONENT_SWIZZLE_B;
	ivci.components.a = VK_COMPONENT_SWIZZLE_A;
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = 1;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = 1;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ivci.flags = 0;

	rt_array_resize(&vk_swapchain.image_views, img_count);
	rt_array_fill(&vk_swapchain.image_views);
	for (uint32_t i = 0; i < img_count; ++i) {
		ivci.image = rt_array_get_ptr(&vk_swapchain.images, i);
		VkImageView *iv = rt_array_get(&vk_swapchain.image_views, i);
		if ((res = vkCreateImageView(vkgfx_device, &ivci,
			vkgfx_allocator, iv)) != VK_SUCCESS) {
			log_entry(VK_SW_MODULE, LOG_CRITICAL,
				"Failed to create image view: %s",
				vku_result_string(res));
			return res;
		}
	}


	if (vkgfx_current_graphics_cmd_pool() != VK_NULL_HANDLE)
		sw_transition_images();

	return res;
}

static inline void
_destroy_framebuffers(void)
{
	for (uint32_t i = 0; i < vk_swapchain.image_views.count; ++i) {
		VkImageView iv = rt_array_get_ptr(&vk_swapchain.image_views, i);
		vkDestroyImageView(vkgfx_device, iv, vkgfx_allocator);
	}

	for (uint32_t i = 0; i < vk_swapchain.framebuffers.count; ++i) {
		VkFramebuffer fb = rt_array_get_ptr(&vk_swapchain.framebuffers, i);
		vkDestroyFramebuffer(vkgfx_device, fb, vkgfx_allocator);
	}
}

ne_status
sw_init(VkSurfaceCapabilitiesKHR caps,
	VkFormat fmt,
	VkColorSpaceKHR cs,
	VkPresentModeKHR pm,
	int32_t gfx_fam,
	int32_t present_fam)
{
	vk_swapchain.caps = caps;
	vk_swapchain.fmt.format = fmt;
	vk_swapchain.fmt.colorSpace = cs;
	vk_swapchain.present_mode = pm;
	vk_swapchain.gfx_fam = gfx_fam;
	vk_swapchain.present_fam = present_fam;

	assert(rt_array_init(&vk_swapchain.images,
		3, sizeof(VkImage)) == SYS_OK);
	assert(rt_array_init(&vk_swapchain.image_views,
		3, sizeof(VkImageView)) == SYS_OK);
	assert(rt_array_init(&vk_swapchain.framebuffers,
		3, sizeof(VkFramebuffer)) == SYS_OK);

	if (_sw_create(ne_gfx_screen_width,
		ne_gfx_screen_height) != VK_SUCCESS)
		return NE_FAIL;

	return NE_OK;
}

static inline VkResult
_recreate(void)
{
	VkResult res;
	_destroy_framebuffers();
	res = _sw_create(0, 0);
	if (res != VK_SUCCESS)
		return res;
	return sw_create_framebuffers();
}

bool
sw_resize(
	uint16_t width,
	uint16_t height)
{
	_destroy_framebuffers();
	if (_sw_create(width, height) != VK_SUCCESS)
		return false;
	return sw_create_framebuffers() == VK_SUCCESS;
}

int32_t
sw_next_image(
	VkSemaphore signal,
	VkFence fence,
	uint32_t *img)
{
	VkResult res;

	res = vkAcquireNextImageKHR(vkgfx_device, vk_swapchain.swapchain,
		UINT64_MAX, signal, fence, img);

	if (res == VK_SUCCESS)
		return VK_SW_OK;
	else if (res == VK_TIMEOUT || res == VK_NOT_READY)
		return VK_SW_SKIP;

	if (res == VK_ERROR_OUT_OF_DATE_KHR) {
		return VK_SW_SKIP;
	} else if (res == VK_SUBOPTIMAL_KHR) {
		if (_recreate() != VK_SUCCESS)
			return VK_SW_STOP;
		return VK_SW_REBUILD;
	}

	return VK_SW_STOP;
}

int32_t
sw_present(
	VkSemaphore *s,
	uint32_t s_count,
	uint32_t id)
{
	VkPresentInfoKHR pi;
	memset(&pi, 0x0, sizeof(pi));
	pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	pi.waitSemaphoreCount = s_count;
	pi.pWaitSemaphores = s;
	pi.swapchainCount = 1;
	pi.pSwapchains = &vk_swapchain.swapchain;
	pi.pImageIndices = &id;
	pi.pResults = NULL;

	VkResult res = vkQueuePresentKHR(vkgfx_present_queue, &pi);

	if (res == VK_SUCCESS)
		return VK_SW_OK;

	if (res == VK_ERROR_OUT_OF_DATE_KHR) {
		return VK_SW_SKIP;
	} else if (res == VK_SUBOPTIMAL_KHR) {
		if (_recreate() != VK_SUCCESS)
			return VK_SW_STOP;
		return VK_SW_REBUILD;
	}

	return VK_SW_STOP;
}

uint32_t
sw_image_count(void)
{
	return vk_swapchain.images.count;
}

VkImage
sw_image(uint32_t img)
{
	return rt_array_get_ptr(&vk_swapchain.images, img);
}

VkFramebuffer
sw_framebuffer(uint32_t img)
{
	return rt_array_get_ptr(&vk_swapchain.framebuffers, img);
}

VkFormat
sw_format(void)
{
	return vk_swapchain.fmt.format;
}

void
sw_transition_images(void)
{
	VkCommandBuffer cmd_buff =
		vku_create_one_shot_cmd_buffer(vkgfx_current_graphics_cmd_pool());

	for (uint32_t i = 0; i < vk_swapchain.images.count; ++i)
		vku_transition_image_layout(
			rt_array_get_ptr(&vk_swapchain.images, i),
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_IMAGE_ASPECT_COLOR_BIT,
			cmd_buff,
			false,
			vkgfx_current_graphics_cmd_pool(),
			vkgfx_graphics_queue);

	vku_execute_one_shot_cmd_buffer(cmd_buff, vkgfx_current_graphics_cmd_pool(),
		vkgfx_graphics_queue, VK_NULL_HANDLE, 0);
}

VkResult
sw_create_framebuffers(void)
{
	VkResult res;
	VkFramebufferCreateInfo fbci;
	memset(&fbci, 0x0, sizeof(fbci));
	fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbci.renderPass = rp_get(RP_GUI);
	fbci.attachmentCount = 1;
	fbci.width = vk_swapchain.extent.width;
	fbci.height = vk_swapchain.extent.height;
	fbci.layers = 1;

	rt_array_resize(&vk_swapchain.framebuffers, vk_swapchain.images.count);
	rt_array_fill(&vk_swapchain.framebuffers);
	for (uint32_t i = 0; i < vk_swapchain.framebuffers.count; ++i) {
		fbci.pAttachments = rt_array_get(&vk_swapchain.image_views, i);
		VkFramebuffer *fb = rt_array_get(&vk_swapchain.framebuffers, i);
		if ((res = vkCreateFramebuffer(vkgfx_device, &fbci,
			vkgfx_allocator, fb)) != VK_SUCCESS) {
			log_entry(VK_SW_MODULE, LOG_CRITICAL,
				"Failed to create framebuffer: %s",
				vku_result_string(res));
			return res;
		}
	}

	return VK_SUCCESS;
}

void
sw_destroy(void)
{
	if (vk_swapchain.swapchain == VK_NULL_HANDLE)
		return;

	_destroy_framebuffers();
	vkDestroySwapchainKHR(vkgfx_device, vk_swapchain.swapchain, vkgfx_allocator);
}

