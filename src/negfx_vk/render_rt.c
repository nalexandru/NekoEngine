/* NekoEngine
 *
 * render_rt.c
 * Author: Alexandru Naiman
 *
 * Vulkan Graphics Subsystem Raytracing render
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

#include <gui/gui.h>
#include <system/log.h>
#include <system/mutex.h>
#include <ecs/ecsys.h>
#include <engine/math.h>
#include <engine/resource.h>

#include <debug.h>
#include <vkgfx.h>
#include <render.h>
#include <vkutil.h>
#include <buffer.h>
#include <pipeline.h>
#include <swapchain.h>
#include <renderpass.h>

#define RT_MAX_LIGHTS		1024
#define RT_RENDER_MODULE	"VulkanRayTracingRender"

ne_status rt_init_material(struct ne_material *);
void rt_release_material(struct ne_material *);
ne_status rt_init_drawable_mesh(struct ne_drawable_mesh_comp *);
void rt_release_drawable_mesh(struct ne_drawable_mesh_comp *);
void rt_add_mesh(struct ne_drawable_mesh_comp *, size_t);
struct ne_light *rt_create_light(void);
void rt_destroy_light(struct ne_light *);

// Render interface

ne_status
vkgfx_init_rt_render(void)
{
	vkgfx_module.init_material = rt_init_material;
	vkgfx_module.release_material = rt_release_material;
	vkgfx_module.init_drawable_mesh = rt_init_drawable_mesh;
	vkgfx_module.release_drawable_mesh = rt_release_drawable_mesh;
	vkgfx_module.add_mesh = rt_add_mesh;
	vkgfx_module.create_light = rt_create_light;
	vkgfx_module.destroy_light = rt_destroy_light;
	vkgfx_module.max_lights = RT_MAX_LIGHTS;

	return NE_OK;
}

void
vkgfx_rt_render_build_cb(
	VkCommandBuffer *depth,
	VkCommandBuffer *lighting)
{
	//
}

void
vkgfx_rt_render_submit(
	VkSemaphore wait,
	VkSemaphore signal)
{
}

void
vkgfx_rt_render_screen_resized(void)
{
}

void
vkgfx_release_rt_render(void)
{
	//
}

// Ray tracing rendering functions

ne_status
rt_init_material(struct ne_material *mat)
{
	return NE_OK;
}

void
rt_release_material(struct ne_material *mat)
{
	//
}

ne_status
rt_init_drawable_mesh(struct ne_drawable_mesh_comp *comp)
{
	return NE_OK;
}

void
rt_release_drawable_mesh(struct ne_drawable_mesh_comp *comp)
{
	//
}

void
rt_add_mesh(struct ne_drawable_mesh_comp *comp, size_t group)
{
	//
}

// Light
struct ne_light *
rt_create_light(void)
{
	return NULL;
}

void
rt_destroy_light(struct ne_light *l)
{
	//
}
