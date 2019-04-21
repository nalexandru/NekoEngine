/* NekoEngine
 *
 * render.c
 * Author: Alexandru Naiman
 *
 * Vulkan Graphics Subsystem Render Interface
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

#include <system/config.h>

#include <vkgfx.h>
#include <render.h>

#define VK_SCENE_MODULE	"VulkanRender"

struct vkgfx_render_module
{
	ne_status (*init)(void);
	void (*build_cb)(VkCommandBuffer *, VkCommandBuffer *);
	void (*submit)(VkSemaphore, VkSemaphore);
	void (*screen_resized)(void);
	void (*release)(void);
};

struct vkgfx_render_module classic_render =
{
	vkgfx_init_classic_render,
	vkgfx_classic_render_build_cb,
	vkgfx_classic_render_submit,
	vkgfx_classic_render_screen_resized,
	vkgfx_release_classic_render
};

struct vkgfx_render_module rt_render =
{
	vkgfx_init_rt_render,
	vkgfx_rt_render_build_cb,
	vkgfx_rt_render_submit,
	vkgfx_rt_render_screen_resized,
	vkgfx_release_rt_render
};

struct vkgfx_render_module *render = NULL;

ne_status
vkgfx_init_render(void)
{
	const char *module = sys_config_get_string("vkgfx_render_module", "classic");
	size_t len = strlen(module);

	if (!strncmp("classic", module, len))
		render = &classic_render;
	else if (!strncmp("ray_tracing", module, len))
		render = &rt_render;
	else
		return NE_FAIL;

	return render->init();
}

void
vkgfx_render_build_cb(
	VkCommandBuffer *depth,
	VkCommandBuffer *lighting)
{
	render->build_cb(depth, lighting);
}

void
vkgfx_render_screen_resized(void)
{
	render->screen_resized();
}

void
vkgfx_release_render(void)
{
	render->release();
}
