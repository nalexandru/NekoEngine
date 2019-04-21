/* NekoEngine
 *
 * texture.c
 * Author: Alexandru Naiman
 *
 * Vulkan Graphics Subsystem Texture
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

#include <string.h>
#include <stdlib.h>

#include <system/log.h>

#include <vkgfx.h>
#include <debug.h>
#include <vkutil.h>
#include <buffer.h>
#include <texture.h>

#define TEXTURE_MODULE	"Vulkan_Texture"

static VkImageType _ne_to_vk_type[NE_TEXTURE_TYPE_COUNT] =
{
	VK_IMAGE_TYPE_1D,
	VK_IMAGE_TYPE_2D,
	VK_IMAGE_TYPE_3D,
	VK_IMAGE_TYPE_2D
};

static VkFormat _ne_to_vk_format[NE_IMAGE_FORMAT_COUNT] =
{
	VK_FORMAT_UNDEFINED,
	VK_FORMAT_R8_UNORM,
	VK_FORMAT_R8_UINT,
	VK_FORMAT_R8G8_UNORM,
	VK_FORMAT_R8G8B8_UNORM,
	VK_FORMAT_R8G8B8_SRGB,
	VK_FORMAT_R8G8B8A8_UNORM,
	VK_FORMAT_R8G8B8A8_SRGB,
	VK_FORMAT_R16G16B16A16_UINT,
	VK_FORMAT_R16G16B16A16_UNORM,
	VK_FORMAT_R16G16B16A16_SFLOAT,
	VK_FORMAT_R32G32B32A32_UINT,
	VK_FORMAT_R32G32B32A32_SFLOAT,
	VK_FORMAT_R64G64B64A64_UINT,
	VK_FORMAT_R64G64B64A64_SFLOAT,
	VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
	VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
	VK_FORMAT_BC2_UNORM_BLOCK,
	VK_FORMAT_BC2_SRGB_BLOCK,
	VK_FORMAT_BC3_UNORM_BLOCK,
	VK_FORMAT_BC3_SRGB_BLOCK,
	VK_FORMAT_BC4_UNORM_BLOCK,
	VK_FORMAT_BC4_SNORM_BLOCK,
	VK_FORMAT_BC5_UNORM_BLOCK,
	VK_FORMAT_BC5_SNORM_BLOCK,
	VK_FORMAT_BC6H_UFLOAT_BLOCK,
	VK_FORMAT_BC6H_SFLOAT_BLOCK,
	VK_FORMAT_BC7_UNORM_BLOCK,
	VK_FORMAT_BC7_SRGB_BLOCK
};

struct ne_texture *
vkgfx_create_texture(
	const struct ne_texture_create_info *ci,
	const void *data,
	uint64_t size)
{
	struct ne_texture *tex = calloc(1, sizeof(*tex));

	if (!tex)
		return NULL;

	tex->format = _ne_to_vk_format[ci->format];
	tex->type = _ne_to_vk_type[ci->type];
	tex->width = ci->width;
	tex->height = ci->height;
	tex->depth = ci->depth;
	tex->levels = ci->levels;
	tex->layers = ci->layers;

	VkImageCreateInfo ici;
	memset(&ici, 0x0, sizeof(ici));
	ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ici.format = tex->format;
	ici.imageType = tex->type;
	ici.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	ici.mipLevels = tex->levels;
	ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ici.samples = 1;
	ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT |
		VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	ici.tiling = VK_IMAGE_TILING_LINEAR;
	ici.extent.width = tex->width;
	ici.extent.height = tex->height;
	ici.extent.depth = tex->depth;

	if (tex->type == NE_TEXTURE_CUBEMAP) {
		tex->layers = 6;
		ici.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	ici.arrayLayers = tex->layers;

	VkResult res = vkCreateImage(vkgfx_device, &ici, vkgfx_allocator, &tex->image);
	if (res != VK_SUCCESS) {
		log_entry(TEXTURE_MODULE, LOG_CRITICAL,
			"Failed to create image: %s",
			vku_result_string(res));
		goto error;
	}

	VkMemoryRequirements mem_req;
	vkGetImageMemoryRequirements(vkgfx_device, tex->image, &mem_req);

	VkMemoryAllocateInfo ai;
	memset(&ai, 0x0, sizeof(ai));
	ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	ai.allocationSize = mem_req.size;
	ai.memoryTypeIndex = vku_get_mem_type(mem_req.memoryTypeBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// FIXME
	tex->own_memory = true;
	if ((res = vkAllocateMemory(vkgfx_device, &ai,
		vkgfx_allocator, &tex->memory)) != VK_SUCCESS) {
		log_entry(TEXTURE_MODULE, LOG_CRITICAL,
			"Failed to allocate image memory: %s",
			vku_result_string(res));
		goto error;
	}

	if ((res = vkBindImageMemory(vkgfx_device, tex->image, tex->memory, 0))
		!= VK_SUCCESS) {
		log_entry(TEXTURE_MODULE, LOG_CRITICAL,
			"Failed to bind image memory: %s",
			vku_result_string(res));
		goto error;
	}

	VkImageViewCreateInfo ivci;
	memset(&ivci, 0x0, sizeof(ivci));
	ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivci.image = tex->image;
	ivci.viewType = ci->type == NE_TEXTURE_CUBEMAP ?
		VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
	ivci.format = tex->format;
	ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = tex->levels;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = tex->layers;
	ivci.components.r = VK_COMPONENT_SWIZZLE_R;
	ivci.components.g = VK_COMPONENT_SWIZZLE_G;
	ivci.components.b = VK_COMPONENT_SWIZZLE_B;
	ivci.components.a = VK_COMPONENT_SWIZZLE_A;

	if ((res = vkCreateImageView(vkgfx_device, &ivci, vkgfx_allocator,
		&tex->image_view)) != VK_SUCCESS) {
		log_entry(TEXTURE_MODULE, LOG_CRITICAL,
			"Failed to create image view: %s",
			vku_result_string(res));
		goto error;
	}

	VK_DBG_SET_OBJECT_NAME((uint64_t)tex->image,
		VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "image");

	if (!data)
		return tex;

	if (vkgfx_upload_image(tex, data, size) != NE_OK)
		goto error;

	return tex;
error:
	vkgfx_destroy_texture(tex);
	return NULL;
}

ne_status
vkgfx_upload_image(
	const struct ne_texture *tex,
	const void *data,
	uint64_t size)
{
	ne_status ret;
	struct ne_buffer *staging = NULL;
	staging = vkgfx_create_buffer_internal(size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, NULL, VK_NULL_HANDLE, 0,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	if (!staging) {
		ret = NE_NO_MEMORY;
		goto error;
	}

	uint8_t *ptr = NULL;
	if (vkgfx_map_buffer(staging, 0, size, &ptr) != NE_OK) {
		ret = NE_MAP_FAIL;
		goto error;
	}

	memcpy(ptr, data, size);
	vkgfx_unmap_buffer(staging);

	VkCommandBuffer cb =
		vku_create_one_shot_cmd_buffer(vkgfx_current_transfer_cmd_pool());

	if (!cb) {
		ret = NE_FAIL;
		goto error;
	}

	VkImageSubresourceRange r;
	memset(&r, 0x0, sizeof(r));
	r.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	r.baseMipLevel = 0;
	r.baseArrayLayer = 0;
	r.levelCount = tex->levels;
	r.layerCount = tex->layers;

	vku_transition_image_layout_range(tex->image,
		VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		&r,
		cb,
		false,
		VK_NULL_HANDLE,
		VK_NULL_HANDLE);

	VkDeviceSize offset = 0;
	for (uint32_t i = 0; i < r.layerCount; ++i) {
		for (uint32_t j = 0; j < r.levelCount; ++j) {
			VkDeviceSize size = 0;

			switch (tex->format) {
			case VK_FORMAT_R8_UNORM:
				size = tex->width * tex->height;
				break;
			case VK_FORMAT_R8G8_UNORM:
				size = tex->width * tex->height * 2;
				break;
			case VK_FORMAT_B8G8R8_UNORM:
			case VK_FORMAT_R8G8B8_UNORM:
				size = tex->width * tex->height * 3;
				break;
			case VK_FORMAT_B8G8R8A8_UNORM:
			case VK_FORMAT_R8G8B8A8_UNORM:
				size = tex->width * tex->height * 4;
				break;
			case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
			case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
			case VK_FORMAT_BC4_UNORM_BLOCK:
			case VK_FORMAT_BC4_SNORM_BLOCK:
				size = ((tex->width + 3) / 4) * ((tex->height + 3) / 4) * 8;
				break;
			case VK_FORMAT_BC2_UNORM_BLOCK:
			case VK_FORMAT_BC2_SRGB_BLOCK:
			case VK_FORMAT_BC3_UNORM_BLOCK:
			case VK_FORMAT_BC3_SRGB_BLOCK:
			case VK_FORMAT_BC5_UNORM_BLOCK:
			case VK_FORMAT_BC5_SNORM_BLOCK:
			case VK_FORMAT_BC6H_UFLOAT_BLOCK:
			case VK_FORMAT_BC6H_SFLOAT_BLOCK:
			case VK_FORMAT_BC7_UNORM_BLOCK:
			case VK_FORMAT_BC7_SRGB_BLOCK:
				size = ((tex->width + 3) / 4) * ((tex->height + 3) / 4) * 16;
				break;
			}


			uint32_t w = j ? tex->width >> j : tex->width;
			uint32_t h = j ? tex->height >> j : tex->height;

			VkBufferImageCopy bic;
			memset(&bic, 0x0, sizeof(bic));
			bic.bufferImageHeight = h;
			bic.bufferRowLength = w;
			bic.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bic.imageSubresource.baseArrayLayer = i;
			bic.imageSubresource.mipLevel = j;
			bic.imageSubresource.layerCount = 1;
			bic.imageOffset.x = bic.imageOffset.y = bic.imageOffset.z = 0;
			bic.bufferOffset = offset;
			bic.imageExtent.width = w;
			bic.imageExtent.height = h;
			bic.imageExtent.depth = 1;

			vkCmdCopyBufferToImage(cb, staging->handle, tex->image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bic);

			offset += size;
		}
	}

	vku_transition_image_layout_range(tex->image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		&r,
		cb,
		false,
		VK_NULL_HANDLE,
		VK_NULL_HANDLE);

	vku_execute_one_shot_cmd_buffer(cb, vkgfx_current_transfer_cmd_pool(),
		vkgfx_transfer_queue, VK_NULL_HANDLE, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

	vkgfx_destroy_buffer(staging);

	return NE_OK;

error:
	vkgfx_destroy_buffer(staging);
	return ret;
}

void
vkgfx_destroy_texture(const struct ne_texture *tex)
{
	if (!tex)
		return;

	if (tex->sampler != VK_NULL_HANDLE)
		vkDestroySampler(vkgfx_device, tex->sampler, vkgfx_allocator);

	if (tex->image_view != VK_NULL_HANDLE)
		vkDestroyImageView(vkgfx_device, tex->image_view, vkgfx_allocator);

	if (tex->image != VK_NULL_HANDLE)
		vkDestroyImage(vkgfx_device, tex->image, vkgfx_allocator);

	if (tex->own_memory)
		vkFreeMemory(vkgfx_device, tex->memory, vkgfx_allocator);

	free((struct ne_texture *)tex);
}

