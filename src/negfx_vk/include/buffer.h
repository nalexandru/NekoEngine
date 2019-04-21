/* NekoEngine
 *
 * buffer.h
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

#ifndef _VK_GFX_BUFFER_H_
#define _VK_GFX_BUFFER_H_

#include <vkutil.h>
#include <graphics/buffer.h>

struct ne_buffer
{
	VkBuffer handle;
	VkDeviceMemory mem;
	VkDeviceSize size;
	VkDeviceSize offset;
	struct ne_buffer *master;
	struct {
		void *ptr;
		VkDeviceSize offset;
		VkDeviceSize size;
	} map;
	int32_t num_buffers;
	int32_t current_buffer;
	VkDeviceSize aligned_size;
	VkDeviceSize current_offset;
	VkDeviceSize total_size;
	void *current_ptr;
};

struct ne_buffer *
vkgfx_create_buffer(struct ne_buffer_create_info *ci,
	const void *data,
	uint64_t offset,
	uint64_t size);

struct ne_buffer *
vkgfx_create_buffer_internal(VkDeviceSize size,
	VkBufferUsageFlags usage,
	const void *data,
	VkDeviceMemory mem,
	VkDeviceSize offset,
	VkMemoryPropertyFlags props,
	int32_t num_buffers);

ne_status
vkgfx_map_buffer(struct ne_buffer *buff,
	uint64_t offset,
	uint64_t size,
	void **ptr);

ne_status
vkgfx_map_ptr(struct ne_buffer *buff);

void
vkgfx_unmap_buffer(struct ne_buffer *buff);

void
vkgfx_next_buffer(struct ne_buffer *buff);

void *
vkgfx_current_ptr(struct ne_buffer *buff);

VkDeviceSize
vkgfx_current_offset(struct ne_buffer *buff);

ne_status
vkgfx_upload_buffer(struct ne_buffer *buff,
	const void *data,
	uint64_t offset,
	uint64_t size);

ne_status
vkgfx_copy_buffer(struct ne_buffer *dst,
	struct ne_buffer *src,
	uint64_t size,
	uint64_t dst_offset,
	uint64_t src_offset);

ne_status
vkgfx_copy_buffer_internal(struct ne_buffer *dst,
	struct ne_buffer *src,
	uint64_t size,
	uint64_t dst_offset,
	uint64_t src_offset,
	VkCommandBuffer cb,
	bool submit,
	VkCommandPool pool,
	VkQueue queue);

ne_status
vkgfx_flush_buffer(struct ne_buffer *buff);

ne_status
vkgfx_invalidate_buffer(struct ne_buffer *buff);

VkDeviceSize
vkgfx_min_buffer_size(VkDeviceSize desired_size, VkBufferUsageFlags flags);

void
vkgfx_destroy_buffer(struct ne_buffer *buff);

#endif /* _VKGFX_BUFFER_H_ */
