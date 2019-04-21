/* NekoEngine
 *
 * swapchain.h
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

#ifndef _VK_GFX_SWAPCHAIN_H_
#define _VK_GFX_SWAPCHAIN_H_

#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <engine/status.h>

#define VKGFX_MAX_SWAPCHAIN_IMAGES		3

ne_status	sw_init(VkSurfaceCapabilitiesKHR caps, VkFormat fmt,
				VkColorSpaceKHR cs, VkPresentModeKHR pm, int32_t gfx_fam,
				int32_t present_fam);

bool		sw_resize(uint16_t width, uint16_t height);
int32_t		sw_next_image(VkSemaphore signal, VkFence fence, uint32_t *img);
int32_t		sw_present(VkSemaphore *s, uint32_t s_count, uint32_t id);
uint32_t	sw_image_count(void);
VkImage		sw_image(uint32_t img);
VkFramebuffer	sw_framebuffer(uint32_t img);
VkFormat	sw_format(void);
void		sw_transition_images(void);
VkResult	sw_create_framebuffers(void);

void		sw_destroy(void);

#endif /* _VK_GFX_SWAPCHAIN_H_ */

