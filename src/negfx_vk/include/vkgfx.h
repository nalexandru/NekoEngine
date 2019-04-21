/* NekoEngine
 *
 * vkgfx.h
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

#ifndef _NE_VK_GRAPHICS_H_
#define _NE_VK_GRAPHICS_H_

#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <engine/status.h>
#include <runtime/array.h>
#include <graphics/graphics.h>

#include <swapchain.h>

#define VK_SW_OK		0
#define VK_SW_REBUILD		1
#define VK_SW_STOP		2
#define VK_SW_SKIP		3

#define VK_VERSION_SIZE		20

#define RES_SHADER		"vk_shader"
#define RES_SHADER_MODULE	"vk_shader_module"

#define VKGFX_UPDATE_SYS_PRI	3000
#define VKGFX_DRAW_SYS_PRI	6000

extern const char *vk_gfx_platform_ext[];

struct vkgfx_device_info
{
	uint32_t id;
	VkPresentModeKHR pm;
	VkSurfaceCapabilitiesKHR caps;
	int32_t gfx_fam;
	int32_t present_fam;
	int32_t compute_fam;
	int32_t transfer_fam;
	VkPhysicalDevice phys_dev;
	VkPhysicalDeviceType phys_dev_type;
	VkPhysicalDeviceLimits limits;
	char name[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
	char version[VK_VERSION_SIZE];
	bool have_debug_ext;
};

typedef struct vkgfx_framebuffer_image
{
	VkImage image;
	VkImageView image_view;
	VkFormat format;
} vkgfx_framebuffer_image;

typedef struct vkgfx_render_framebuffer
{
	vkgfx_framebuffer_image color;
	vkgfx_framebuffer_image normal;
	vkgfx_framebuffer_image depth;
	vkgfx_framebuffer_image resolve;
	VkFramebuffer lighting_fb;
	VkFramebuffer depth_fb;
	VkFramebuffer resolve_fb;
	VkSampleCountFlagBits samples;
	VkDeviceMemory memory;
	uint32_t width;
	uint32_t height;
} vkgfx_render_framebuffer;

typedef struct vkgfx_thread_cp
{
	VkCommandPool static_xfer_pool;
	VkCommandPool dynamic_xfer_pools[VKGFX_MAX_SWAPCHAIN_IMAGES];
	VkCommandPool static_gfx_pool;
	VkCommandPool dynamic_gfx_pools[VKGFX_MAX_SWAPCHAIN_IMAGES];
	VkCommandPool static_compute_pool;
	VkCommandPool dynamic_compute_pools[VKGFX_MAX_SWAPCHAIN_IMAGES];
	VkFence fences[VKGFX_MAX_SWAPCHAIN_IMAGES];
	rt_array gfx_free_list[VKGFX_MAX_SWAPCHAIN_IMAGES];
	rt_array xfer_free_list[VKGFX_MAX_SWAPCHAIN_IMAGES];
	rt_array compute_free_list[VKGFX_MAX_SWAPCHAIN_IMAGES];
} vkgfx_thread_cp;

struct vkgfx_device_info vkgfx_device_info;
extern VkInstance vkgfx_instance;
extern VkSurfaceKHR vkgfx_surface;
extern VkDevice vkgfx_device;
extern VkAllocationCallbacks *vkgfx_allocator;
extern VkQueue vkgfx_graphics_queue;
extern VkQueue vkgfx_compute_queue;
extern VkQueue vkgfx_transfer_queue;
extern VkQueue vkgfx_present_queue;
extern VkSemaphore vkgfx_image_available_sem;
extern VkSemaphore vkgfx_transfer_complete_sem;
extern VkSemaphore vkgfx_scene_complete_sem;
extern VkSemaphore vkgfx_render_complete_sem;
extern vkgfx_render_framebuffer vkgfx_render_target;

extern VkSampler vkgfx_default_sampler;

void vkgfx_stage_update(const void *src, struct ne_buffer *dst, VkDeviceSize size, VkDeviceSize offset);

VkCommandPool	vkgfx_static_gfx_cmd_pool(void);
VkCommandPool	vkgfx_static_xfer_cmd_pool(void);
VkCommandPool	vkgfx_static_compute_cmd_pool(void);

VkCommandPool	vkgfx_gfx_cmd_pool(void);
VkCommandPool	vkgfx_xfer_cmd_pool(void);
VkCommandPool	vkgfx_compute_cmd_pool(void);

VkCommandBuffer vkgfx_alloc_gfx_cmd_buff(VkCommandBufferLevel level);
VkCommandBuffer vkgfx_alloc_xfer_cmd_buff(VkCommandBufferLevel level);
VkCommandBuffer vkgfx_alloc_compute_cmd_buff(VkCommandBufferLevel level);

VkCommandPool	vkgfx_current_transfer_cmd_pool(void);
VkCommandPool	vkgfx_current_graphics_cmd_pool(void);
void		vkgfx_register_update_cb(VkCommandBuffer cb);
void		vkgfx_set_default_viewport(VkCommandBuffer cb);

extern struct ne_gfx_module vkgfx_module;

// Early init, not Vulkan related
ne_status	vkgfx_init_types(void);
ne_status	vkgfx_init_render_target_info(void);

// Platform specific Vulkan initialization & teardown
ne_status	vkgfx_init_surface(void);
void		vkgfx_release_surface(void);

// Vulkan initialization
ne_status 	vkgfx_init_instance(void);
ne_status 	vkgfx_init_device(void);
ne_status	vkgfx_init_swapchain(void);
ne_status 	vkgfx_init_debug(void);
ne_status 	vkgfx_init_cmd_pools(void);
ne_status 	vkgfx_init_sem(void);
ne_status	vkgfx_init_framebuffers(void);
ne_status	vkgfx_init_synchronization(void);
ne_status	vkgfx_init_cmd_buffers(void);
ne_status	vkgfx_init_blank_texture(void);

// Vulkan teardown
void		vkgfx_release_instance(void);
void		vkgfx_release_device(void);
void		vkgfx_release_swapchain(void);
void		vkgfx_release_debug(void);
void		vkgfx_release_cmd_pools(void);
void		vkgfx_release_sem(void);
void		vkgfx_release_framebuffers(void);
void		vkgfx_release_synchronization(void);
void		vkgfx_release_cmd_buffers(void);
void		vkgfx_release_blank_texture(void);

#endif /* _NE_VK_GRAPHICS_H_ */

