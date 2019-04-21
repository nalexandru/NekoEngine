/* NekoEngine
 *
 * buffer.c
 * Author: Alexandru Naiman
 *
 * Vulkan Graphics Subsystem
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
#include <stdlib.h>

#include <vkutil.h>
#include <buffer.h>

#define VKBUFF_MODULE	"Vulkan_Buffer"
#define BUFFER_MODULE	"Vulkan_Buffer"

static ne_buffer_access _access[] =
{
	NE_GPU_LOCAL,
	NE_CPU_VISIBLE,
	NE_CPU_COHERENT,
	NE_CPU_CACHED
};
#define NUM_ACCESS sizeof(_access) / sizeof(ne_buffer_access)

static ne_buffer_usage _usage[] =
{
	NE_VERTEX_BUFFER,
	NE_INDEX_BUFFER,
	NE_UNIFORM_BUFFER,
	NE_STORAGE_BUFFER,
	NE_UNIFORM_TEXEL_BUFFER,
	NE_STORAGE_TEXEL_BUFFER,
	NE_INDIRECT_BUFFER,
	NE_TRANSFORM_FEEDBACK,
	NE_TRANSFORM_FEEDBACK_COUNTER,
	NE_CONDITIONAL_RENDERING,
	NE_RAY_TRACING,
	NE_SHADER_DEVICE_ADDRESS,
	NE_TRANSFER_SRC,
	NE_TRANSFER_DST
};
#define NUM_USAGE sizeof(_usage) / sizeof(ne_buffer_usage)

static inline VkMemoryPropertyFlagBits
_vk_access(ne_buffer_access access)
{
	switch (access) {
	case NE_GPU_LOCAL:
		return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	case NE_CPU_VISIBLE:
		return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	case NE_CPU_COHERENT:
		return VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	case NE_CPU_CACHED:
		return VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
	}

	return 0;
}

static inline VkBufferUsageFlagBits
_vk_usage(ne_buffer_usage usage)
{
	switch (usage) {
	case NE_VERTEX_BUFFER:
		return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	case NE_INDEX_BUFFER:
		return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	case NE_UNIFORM_BUFFER:
		return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	case NE_STORAGE_BUFFER:
		return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	case NE_UNIFORM_TEXEL_BUFFER:
		return VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
	case NE_STORAGE_TEXEL_BUFFER:
		return VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
	case NE_INDIRECT_BUFFER:
		return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	case NE_TRANSFORM_FEEDBACK:
		return VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT;
	case NE_TRANSFORM_FEEDBACK_COUNTER:
		return VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT;
	case NE_CONDITIONAL_RENDERING:
		return VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT;
	case NE_RAY_TRACING:
		return VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
	case NE_SHADER_DEVICE_ADDRESS:
		return VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT;
	case NE_TRANSFER_SRC:
		return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	case NE_TRANSFER_DST:
		return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}

	return 0;
}

struct ne_buffer *
vkgfx_create_buffer(
	struct ne_buffer_create_info *ci,
	const void *data,
	uint64_t offset,
	uint64_t size)
{
	VkMemoryPropertyFlags prop = 0;
	VkBufferUsageFlags usage = 0;
	VkDeviceMemory mem = VK_NULL_HANDLE;

	for (uint32_t i = 0; i < NUM_ACCESS; ++i)
		if (ci->access & _access[i])
			prop |= _vk_access(_access[i]);

	for (uint32_t i = 0; i < NUM_USAGE; ++i)
		if (ci->usage & _usage[i])
			usage |= _vk_usage(_usage[i]);

	if (ci->master) {
		mem = ci->master->mem;
		offset += ci->master->offset;
	}

	return vkgfx_create_buffer_internal(ci->size, usage, data, mem, offset,
						prop, 1);
}

struct ne_buffer *
vkgfx_create_buffer_internal(
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	const void *data,
	VkDeviceMemory mem,
	VkDeviceSize offset,
	VkMemoryPropertyFlags props,
	int32_t num_buffers)
{
	VkDeviceSize align_size[3] = { 1, 1, 1 }, align = 1;
	struct ne_buffer *buff = calloc(1, sizeof(*buff));
	assert(buff);

	buff->mem = mem;
	buff->offset = offset;
	buff->size = size;
	buff->num_buffers = 1;// num_buffers;

	if ((usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT))
		align_size[0] =
			vkgfx_device_info.limits.minUniformBufferOffsetAlignment;

	if ((usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
		align_size[1] =
			vkgfx_device_info.limits.minStorageBufferOffsetAlignment;

	if ((usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) ||
		(usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT))
		align_size[2] =
			vkgfx_device_info.limits.minTexelBufferOffsetAlignment;

	for (uint8_t i = 0; i < 3; ++i)
		align = align_size[i] > align ? align_size[i] : align;

	buff->aligned_size = size / align;
	buff->aligned_size = align * (buff->aligned_size + 1);
	buff->total_size = buff->aligned_size * num_buffers;

	if (!vku_create_buffer(&buff->handle, &buff->mem, buff->total_size,
		usage, props, buff->offset, VK_SHARING_MODE_EXCLUSIVE)) {
		log_entry(VKBUFF_MODULE, LOG_CRITICAL,
			"Failed to create buffer");
		goto error;
	}

	if (!data)
		return buff;

	struct ne_buffer *staging = vkgfx_create_buffer_internal(size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			NULL, VK_NULL_HANDLE, 0,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);

	uint8_t *ptr = NULL;
	if (vkgfx_map_buffer(staging, 0, size, &ptr) != NE_OK)
		goto error;

	memcpy(ptr, data, size);
	vkgfx_unmap_buffer(staging);

	vku_copy_buffer(staging->handle, buff->handle, size, 0, 0,
			VK_NULL_HANDLE, true, VK_NULL_HANDLE, VK_NULL_HANDLE);

	vkgfx_destroy_buffer(staging);

	return buff;

error:
	free(buff);

	return NULL;
}

ne_status
vkgfx_map_buffer(
	struct ne_buffer *buff,
	uint64_t offset,
	uint64_t size,
	void **ptr)
{
	if (!size)
		size = VK_WHOLE_SIZE;

	buff->map.offset = buff->offset + offset;
	buff->map.size = size;

	VkResult res = vkMapMemory(vkgfx_device, buff->mem,
			buff->map.offset, buff->map.size, 0, ptr);
	if (res != VK_SUCCESS) {
		log_entry(BUFFER_MODULE, LOG_CRITICAL,
			"Failed to map memory: %s", vku_result_string(res));
		return NE_MAP_FAIL;
	}

	buff->map.ptr = *ptr;

	return NE_OK;
}

ne_status
vkgfx_map_ptr(struct ne_buffer *buff)
{
	buff->map.offset = buff->offset;
	buff->map.size = VK_WHOLE_SIZE;

	VkResult res = vkMapMemory(vkgfx_device, buff->mem,
			buff->map.offset, buff->map.size, 0, &buff->map.ptr);
	if (res != VK_SUCCESS) {
		log_entry(BUFFER_MODULE, LOG_CRITICAL,
			"Failed to map memory: %s", vku_result_string(res));
		return NE_MAP_FAIL;
	}

	buff->current_ptr = buff->map.ptr;

	return NE_OK;
}

void
vkgfx_unmap_buffer(struct ne_buffer *buff)
{
	vkUnmapMemory(vkgfx_device, buff->mem);
	buff->map.ptr = buff->current_ptr = 0;
}

void
vkgfx_next_buffer(struct ne_buffer *buff)
{
	buff->current_buffer = (buff->current_buffer + 1) % buff->num_buffers;
	buff->current_offset = buff->aligned_size * buff->current_buffer;

	if (buff->map.ptr)
		buff->current_ptr = ((uint8_t *)buff->map.ptr) +
				buff->current_offset;
}

void *
vkgfx_current_ptr(struct ne_buffer *buff)
{
	return buff->current_ptr;
}

VkDeviceSize
vkgfx_current_offset(struct ne_buffer *buff)
{
	return buff->current_offset;
}

ne_status
vkgfx_upload_buffer(
	struct ne_buffer *buff,
	const void *data,
	uint64_t offset,
	uint64_t size)
{
	return NE_FAIL;
}

ne_status
vkgfx_copy_buffer(
	struct ne_buffer *dst,
	struct ne_buffer *src,
	uint64_t size,
	uint64_t dst_offset,
	uint64_t src_offset)
{
	VkDeviceSize s_offset = 0, d_offset = 0;

	if (!size)
		size = VK_WHOLE_SIZE;

	if (src->num_buffers > 1)
		s_offset = src->size * src->current_buffer;
	s_offset += src->offset + src_offset;

	if (dst->num_buffers > 1)
		d_offset = src->size * src->current_buffer;
	d_offset += dst->offset + dst_offset;

	vku_copy_buffer(src->handle, dst->handle, size, s_offset, d_offset,
		VK_NULL_HANDLE, true, VK_NULL_HANDLE, VK_NULL_HANDLE);

	return NE_OK;
}

ne_status
vkgfx_copy_buffer_internal(
	struct ne_buffer *dst,
	struct ne_buffer *src,
	uint64_t size,
	uint64_t dst_offset,
	uint64_t src_offset,
	VkCommandBuffer cb,
	bool submit,
	VkCommandPool pool,
	VkQueue queue)
{
	if (!size)
		size = VK_WHOLE_SIZE;

	if (dst->master)
		dst_offset += dst->master->offset;

	vku_copy_buffer(src->handle, dst->handle, size, src->offset + src_offset,
		dst->offset + dst_offset, cb, submit,
		pool, queue);

	return NE_OK;
}

ne_status
vkgfx_flush_buffer(struct ne_buffer *buff)
{
	VKU_STRUCT(VkMappedMemoryRange, mmr,
		VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE);

	mmr.memory = buff->mem;
	mmr.offset = buff->map.offset;
	mmr.size = buff->map.size;

	VkResult res = vkFlushMappedMemoryRanges(vkgfx_device, 1, &mmr);
	if (res != VK_SUCCESS) {
		log_entry(BUFFER_MODULE, LOG_WARNING,
			"Failed to flush mapped memory: %s",
			vku_result_string(res));
		return NE_FAIL;
	}

	return NE_OK;
}

ne_status
vkgfx_invalidate_buffer(struct ne_buffer *buff)
{
	VKU_STRUCT(VkMappedMemoryRange, mmr,
		VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE);

	mmr.memory = buff->mem;
	mmr.offset = buff->map.offset;
	mmr.size = buff->map.size;

	VkResult res = vkInvalidateMappedMemoryRanges(vkgfx_device, 1, &mmr);
	if (res != VK_SUCCESS) {
		log_entry(BUFFER_MODULE, LOG_WARNING,
			"Failed to invalidate mapped memory: %s",
			vku_result_string(res));
		return NE_FAIL;
	}

	return NE_OK;
}

VkDeviceSize
vkgfx_min_buffer_size(
	VkDeviceSize desired_size,
	VkBufferUsageFlags usage)
{
	VkDeviceSize size = 0;
	VkDeviceSize align = 1;
	VkDeviceSize align_size[3] = { 1, 1, 1 };

	if ((usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT))
		align_size[0] =
			vkgfx_device_info.limits.minUniformBufferOffsetAlignment;

	if ((usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
		align_size[1] =
			vkgfx_device_info.limits.minStorageBufferOffsetAlignment;

	if ((usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) ||
		(usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT))
		align_size[2] =
			vkgfx_device_info.limits.minTexelBufferOffsetAlignment;

	for (uint8_t i = 0; i < 3; ++i)
		align = align_size[i] > align ? align_size[i] : align;

	size = desired_size / align;
	return align * (size + 1);
}

void
vkgfx_destroy_buffer(struct ne_buffer *buff)
{
	if (!buff)
		return;

	if (!buff->master)
		vkFreeMemory(vkgfx_device, buff->mem, vkgfx_allocator);

	vkDestroyBuffer(vkgfx_device, buff->handle, vkgfx_allocator);

	free(buff);
}

