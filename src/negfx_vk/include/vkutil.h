/* NekoEngine
 *
 * vkutil.h
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

#ifndef _NE_VK_UTIL_H_
#define _NE_VK_UTIL_H_

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <system/log.h>

#include <vkgfx.h>

#define VKUTIL_MODULE	"vkutil"

#define VKU_STRUCT(type, name, stype)		\
	type name;				\
	memset(&name, 0x0, sizeof(name));	\
	name.sType = stype;			\

static inline uint32_t
vku_get_mem_type(uint32_t filter,
	VkMemoryPropertyFlags flags)
{
	VkPhysicalDeviceMemoryProperties props;
	vkGetPhysicalDeviceMemoryProperties(vkgfx_device_info.phys_dev, &props);

	for (uint32_t i = 0; i < props.memoryTypeCount; ++i)
		if ((filter & (1 << i)) &&
			((props.memoryTypes[i].propertyFlags & flags) == flags))
			return i;

	log_entry(VKUTIL_MODULE, LOG_CRITICAL,
		"Failed to get memory type for %d, $d", filter, flags);

	return 0;
}

static inline const char *
vku_result_string(VkResult res)
{
	switch (res) {
	case VK_SUCCESS:
		return "Success";
	case VK_NOT_READY:
		return "Not Ready";
	case VK_TIMEOUT:
		return "Timeout";
	case VK_EVENT_SET:
		return "Event Set";
	case VK_EVENT_RESET:
		return "Event Reset";
	case VK_INCOMPLETE:
		return "Incomplete";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "Out of Host Memory";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "Out of Device Memory";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "Initialization Failed";
	case VK_ERROR_DEVICE_LOST:
		return "Device Lost";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "Memory Map Failed";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "Layer Not Present";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "Extension Not Present";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "Feature Not Present";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "Incompatible Driver";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "Too Many Objects";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "Format Not Supported";
	case VK_ERROR_FRAGMENTED_POOL:
		return "Fragmented Pool";
	case VK_ERROR_OUT_OF_POOL_MEMORY:
	       return "Out of Pool Memory";
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
	       return "Invalid External Handle";
	case VK_ERROR_SURFACE_LOST_KHR:
	       return "Surface Lost";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
	       return "Native Window in Use";
	case VK_SUBOPTIMAL_KHR:
	       return "Suboptimal";
	case VK_ERROR_OUT_OF_DATE_KHR:
	       return "Out of Date";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
	       return "Incompatible Display";
	case VK_ERROR_VALIDATION_FAILED_EXT:
	       return "Validation Failed";
	case VK_ERROR_INVALID_SHADER_NV:
	       return "Invalid Shader";
	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
	       return "Invalid DRM Format Modifier Plane Layout";
	case VK_ERROR_FRAGMENTATION_EXT:
	       return "Fragmentation";
	case VK_ERROR_NOT_PERMITTED_EXT:
	       return "Not Permitted";
	case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT:
	       return "Invalid Device Address";
	}

	return "Unknown";
}

// Command buffers

static inline VkCommandBuffer
vku_create_cmd_buff(VkCommandBufferLevel lvl,
	VkCommandPool pool)
{
	VkResult res;
	VkCommandBuffer cmd_buff;
	VkCommandBufferAllocateInfo ai;

	memset(&ai, 0x0, sizeof(ai));
	ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	ai.commandPool = pool;
	ai.level = lvl;
	ai.commandBufferCount = 1;

	if ((res = vkAllocateCommandBuffers(vkgfx_device, &ai, &cmd_buff))
		!= VK_SUCCESS) {
		log_entry(VKUTIL_MODULE, LOG_CRITICAL,
			"Failed to allocate command buffer: %llu", res);
		return VK_NULL_HANDLE;
	}

	return cmd_buff;
}

static inline void
vku_execute_command_buffer(VkCommandBuffer cmd_buff,
	VkQueue queue,
	VkSemaphore wait,
	VkPipelineStageFlags wait_dst)
{
	VkSubmitInfo si;
	memset(&si, 0x0, sizeof(si));
	si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	si.commandBufferCount = 1;
	si.pCommandBuffers = &cmd_buff;

	if (queue == VK_NULL_HANDLE)
		queue = vkgfx_graphics_queue;

	if (wait != VK_NULL_HANDLE) {
		VkPipelineStageFlags flags[] = { wait_dst };
		si.pWaitDstStageMask = flags;
		si.waitSemaphoreCount = 1;
		si.pWaitSemaphores = &wait;
	}

	vkQueueSubmit(queue, 1, &si, VK_NULL_HANDLE);

	if (wait == VK_NULL_HANDLE)
		vkQueueWaitIdle(queue);
}

static inline void
vku_free_command_buffer(VkCommandBuffer cmd_buff,
	VkCommandPool pool)
{
	vkFreeCommandBuffers(vkgfx_device, pool, 1, &cmd_buff);
}

static inline VkCommandBuffer
vku_create_one_shot_cmd_buffer(VkCommandPool pool)
{
	VkCommandBufferAllocateInfo ai;
	memset(&ai, 0x0, sizeof(ai));
	ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	ai.commandPool = pool;
	ai.commandBufferCount = 1;

	VkResult res;
	VkCommandBuffer cmd_buff;
	if((res = vkAllocateCommandBuffers(vkgfx_device, &ai, &cmd_buff))
		!= VK_SUCCESS) {
		log_entry(VKUTIL_MODULE, LOG_CRITICAL,
			"Failed to allocate command buffer: %llu", res);
		return VK_NULL_HANDLE;
	}

	VkCommandBufferBeginInfo bi;
	memset(&bi, 0x0, sizeof(bi));
	bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmd_buff, &bi);

	return cmd_buff;
}

static inline void
vku_execute_one_shot_cmd_buffer(VkCommandBuffer cmd_buff,
	VkCommandPool pool,
	VkQueue queue,
	VkSemaphore wait,
	VkPipelineStageFlags wait_dst)
{
	vkEndCommandBuffer(cmd_buff);
	vku_execute_command_buffer(cmd_buff, queue, wait, wait_dst);
	vkFreeCommandBuffers(vkgfx_device, pool, 1, &cmd_buff);
}

// Buffers

static inline bool
vku_create_buffer(VkBuffer *buffer,
	VkDeviceMemory *memory,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags prop,
	VkDeviceSize offset,
	VkSharingMode sharing)
{
	VkResult res;
	VkBufferCreateInfo bi;
	memset(&bi, 0x0, sizeof(bi));
	bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bi.size = size;
	bi.usage = usage;
	bi.sharingMode = sharing;

	if ((res = vkCreateBuffer(vkgfx_device, &bi, NULL, buffer))
		!= VK_SUCCESS) {
		log_entry(VKUTIL_MODULE, LOG_CRITICAL,
			"Failed to create buffer: %s",
			vku_result_string(res));
		return false;
	}

	if (*memory == VK_NULL_HANDLE) {
		VkMemoryRequirements mem_req;
		memset(&mem_req, 0x0, sizeof(mem_req));
		vkGetBufferMemoryRequirements(vkgfx_device, *buffer, &mem_req);

		VkMemoryAllocateInfo ai;
		memset(&ai, 0x0, sizeof(ai));
		ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		ai.allocationSize = mem_req.size;
		ai.memoryTypeIndex =
			vku_get_mem_type(mem_req.memoryTypeBits, prop);

		if ((res = vkAllocateMemory(vkgfx_device, &ai,
			vkgfx_allocator, memory)) != VK_SUCCESS) {
			log_entry(VKUTIL_MODULE, LOG_CRITICAL,
				"Failed to allocate buffer memory: %s",
				vku_result_string(res));
			return false;
		}
	}

	if ((res = vkBindBufferMemory(vkgfx_device, *buffer, *memory, 0))
		!= VK_SUCCESS) {
		log_entry(VKUTIL_MODULE, LOG_CRITICAL,
			"Failed to bind buffer memory: %s",
			vku_result_string(res));
		return false;
	}

	return true;
}

static inline void
vku_copy_buffer(VkBuffer src,
	VkBuffer dst,
	VkDeviceSize size,
	VkDeviceSize src_offset,
	VkDeviceSize dst_offset,
	VkCommandBuffer cmd_buff,
	bool submit,
	VkCommandPool pool,
	VkQueue queue)
{
	if (cmd_buff == VK_NULL_HANDLE) {
		if (pool == VK_NULL_HANDLE)
			pool = vkgfx_current_graphics_cmd_pool();

		cmd_buff = vku_create_one_shot_cmd_buffer(pool);
		submit = true;

		if (queue == VK_NULL_HANDLE)
			queue = vkgfx_graphics_queue;
	}

	VkBufferCopy cr;
	memset(&cr, 0x0, sizeof(cr));
	cr.srcOffset = src_offset;
	cr.dstOffset = dst_offset;
	cr.size = size;
	vkCmdCopyBuffer(cmd_buff, src, dst, 1, &cr);

	if (submit)
		vku_execute_one_shot_cmd_buffer(cmd_buff, pool, queue,
		VK_NULL_HANDLE, 0);
}

static inline void
vku_fill_buffer(VkBuffer buffer,
	VkDeviceSize offset,
	VkDeviceSize size,
	uint32_t data,
	VkCommandBuffer cmd_buff,
	bool submit,
	VkCommandPool pool,
	VkQueue queue)
{
	if (cmd_buff == VK_NULL_HANDLE) {
		if (pool == VK_NULL_HANDLE)
			pool = vkgfx_current_graphics_cmd_pool();

		cmd_buff = vku_create_one_shot_cmd_buffer(pool);
		submit = true;

		if (queue == VK_NULL_HANDLE)
			queue = vkgfx_graphics_queue;
	}

	vkCmdFillBuffer(cmd_buff, buffer, offset, size, data);

	if (submit)
		vku_execute_one_shot_cmd_buffer(cmd_buff, pool, queue,
		VK_NULL_HANDLE, 0);
}

// Images

static inline bool
vku_create_image(VkImage *image,
	VkDeviceMemory *memory,
	uint32_t width,
	uint32_t height,
	uint32_t depth,
	VkMemoryPropertyFlags props,
	VkFormat format,
	VkImageType type,
	VkImageUsageFlags usage,
	VkImageTiling tiling,
	uint32_t mip_levels,
	uint32_t array_layers,
	VkImageCreateFlags flags,
	VkSampleCountFlagBits samples)
{
	VkImageCreateInfo ci;
	memset(&ci, 0x0, sizeof(ci));
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ci.format = format;
	ci.imageType = type;
	ci.initialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	ci.mipLevels = mip_levels;
	ci.arrayLayers = array_layers;
	ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ci.samples = samples;
	ci.usage = usage;
	ci.tiling = tiling;
	ci.extent.width = width;
	ci.extent.height = height;
	ci.extent.depth = depth;
	ci.flags = flags;

	VkResult res = vkCreateImage(vkgfx_device, &ci, vkgfx_allocator, image);
	if (res != VK_SUCCESS) {
		log_entry(VKUTIL_MODULE, LOG_CRITICAL,
			"Failed to create image: %s",
			vku_result_string(res));
		return false;
	}

	if (*memory == VK_NULL_HANDLE) {
		VkMemoryRequirements mem_req;
		vkGetImageMemoryRequirements(vkgfx_device, *image, &mem_req);

		VkMemoryAllocateInfo ai;
		memset(&ai, 0x0, sizeof(ai));
		ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		ai.allocationSize = mem_req.size;
		ai.memoryTypeIndex =
			vku_get_mem_type(mem_req.memoryTypeBits, props);

		if ((res = vkAllocateMemory(vkgfx_device, &ai,
			vkgfx_allocator, memory)) != VK_SUCCESS) {
			log_entry(VKUTIL_MODULE, LOG_CRITICAL,
				"Failed to allocate image memory: %s",
				vku_result_string(res));
			return false;
		}
	}

	if ((res = vkBindImageMemory(vkgfx_device, *image, *memory, 0))
		!= VK_SUCCESS) {
		log_entry(VKUTIL_MODULE, LOG_CRITICAL,
			"Failed to bind image memory: %s",
			vku_result_string(res));
		return false;
	}

	return true;
}

static inline void
vku_blit_image(VkImage src,
	VkImage dst,
	int32_t src_width,
	int32_t src_height,
	int32_t dst_width,
	int32_t dst_height,
	VkFilter filter,
	VkCommandBuffer cmd_buff,
	bool submit,
	VkCommandPool pool,
	VkQueue queue)
{
	/*if (cmd_buff == VK_NULL_HANDLE) {
		cmd_buff = vkutil_create_one_shot_cmd_buffer();
		submit = true;
	}*/

	VkImageSubresourceLayers sub_res;
	memset(&sub_res, 0x0, sizeof(sub_res));
	sub_res.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	sub_res.baseArrayLayer = 0;
	sub_res.mipLevel = 0;
	sub_res.layerCount = 1;

	VkImageBlit region;
	memset(&region, 0x0, sizeof(region));
	region.srcSubresource = sub_res;
	region.dstSubresource = sub_res;
/*	region.srcOffsets[0] = { 0, 0, 0 };
	region.srcOffsets[1] = { src_width, src_height, 1 };
	region.dstOffsets[0] = { 0, 0, 0 };
	region.dstOffsets[1] = { dst_width, dst_height, 1 };*/

	vkCmdBlitImage(cmd_buff, src, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, filter);

//	if (submit)
//		vkutil_execute_one_shot_cmd_buffer(cmdBuffer, pool, queue);
}

static inline void
vku_transition_image_layout_range(VkImage image,
	VkImageLayout old,
	VkImageLayout new,
	VkImageSubresourceRange *range,
	VkCommandBuffer cmd_buff,
	bool submit,
	VkCommandPool pool,
	VkQueue queue)
{
	if (cmd_buff == VK_NULL_HANDLE) {
		if (pool == VK_NULL_HANDLE)
			pool = vkgfx_current_graphics_cmd_pool();

		cmd_buff = vku_create_one_shot_cmd_buffer(pool);
		submit = true;

		if (queue == VK_NULL_HANDLE)
			queue = vkgfx_graphics_queue;
	}

	VkPipelineStageFlags src_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dst_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	VkImageMemoryBarrier barrier;
	memset(&barrier, 0x0, sizeof(barrier));
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = old;
	barrier.newLayout = new;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange = *range;

	switch (old) {
		case VK_IMAGE_LAYOUT_PREINITIALIZED: {
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			src_flags = VK_PIPELINE_STAGE_HOST_BIT;
		} break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			src_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			src_flags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			src_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			src_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			src_flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		break;
		default:
			barrier.srcAccessMask = 0;
		break;
	}

	switch (new) {
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			dst_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			dst_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dst_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		break;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dst_flags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			dst_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
		default:
			barrier.dstAccessMask = 0;
		break;
	}

	vkCmdPipelineBarrier(cmd_buff, src_flags, dst_flags, 0, 0, NULL,
		0, NULL, 1, &barrier);

	if (submit)
		vku_execute_one_shot_cmd_buffer(cmd_buff, pool, queue,
		VK_NULL_HANDLE, 0);
}

static inline void
vku_transition_image_layout(VkImage image,
	VkImageLayout old,
	VkImageLayout new,
	VkImageAspectFlags aspect,
	VkCommandBuffer cmd_buff,
	bool submit,
	VkCommandPool pool,
	VkQueue queue)
{
	VkImageSubresourceRange range;
	memset(&range, 0x0, sizeof(range));
	range.aspectMask = aspect;
	range.baseMipLevel = 0;
	range.levelCount = 1;
	range.baseArrayLayer = 0;
	range.layerCount = 1;
	vku_transition_image_layout_range(image, old, new, &range, cmd_buff, submit, pool, queue);
}

#undef VKUTIL_MODULE

#endif /* _NE_VK_UTIL_H_ */

