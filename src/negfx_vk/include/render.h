/* NekoEngine
 *
 * scene.h
 * Author: Alexandru Naiman
 *
 * Vulkan Graphics Subsystem Scene
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

#ifndef _VKGFX_RENDER_H_
#define _VKGFX_RENDER_H_

#include <vulkan/vulkan.h>

#include <engine/status.h>

ne_status	vkgfx_init_render(void);
void		vkgfx_render_build_cb(VkCommandBuffer *, VkCommandBuffer *);
void		vkgfx_render_submit(VkSemaphore wait, VkSemaphore signal);
void		vkgfx_render_screen_resized(void);
void		vkgfx_release_render(void);

// Classic graphics pipeline
ne_status	vkgfx_init_classic_render(void);
void		vkgfx_classic_render_build_cb(VkCommandBuffer *, VkCommandBuffer *);
void		vkgfx_classic_render_submit(VkSemaphore wait, VkSemaphore signal);
void		vkgfx_classic_render_screen_resized(void);
void		vkgfx_release_classic_render(void);

// Raytracing pipeline
ne_status	vkgfx_init_rt_render(void);
void		vkgfx_rt_render_build_cb(VkCommandBuffer *, VkCommandBuffer *);
void		vkgfx_rt_render_submit(VkSemaphore wait, VkSemaphore signal);
void		vkgfx_rt_render_screen_resized(void);
void		vkgfx_release_rt_render(void);

#endif /* _VKGFX_RENDER_H_ */
