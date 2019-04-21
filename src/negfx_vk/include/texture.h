/* NekoEngine
 *
 * texture.h
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

#ifndef _NE_VKGFX_TEXTURE_H_
#define _NE_VKGFX_TEXTURE_H_

#include <vulkan/vulkan.h>

#include <engine/status.h>
#include <graphics/texture.h>

struct ne_texture
{
	VkImage image;
	VkImageView image_view;
	VkDeviceMemory memory;
	VkSampler sampler;
	VkFormat format;
	VkImageType type;
	bool own_memory;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t levels;
	uint32_t layers;
};

struct ne_texture *
vkgfx_create_texture(const struct ne_texture_create_info *, const void *, uint64_t);

ne_status
vkgfx_upload_image(const struct ne_texture *, const void *, uint64_t);

void
vkgfx_destroy_texture(const struct ne_texture *);

#endif /* _NE_SHADER_ */

