/* NekoEngine
 *
 * vkgfx.c
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

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <system/log.h>
#include <system/mutex.h>
#include <system/compat.h>
#include <system/config.h>
#include <ecs/ecsys.h>
#include <engine/task.h>
#include <graphics/graphics.h>

#include <gui.h>
#include <dpool.h>
#include <debug.h>
#include <vkgfx.h>
#include <render.h>
#include <vkutil.h>
#include <buffer.h>
#include <texture.h>
#include <pipeline.h>
#include <swapchain.h>
#include <renderpass.h>

#define VK_VERSION_SIZE	20
#define VKGFX_MODULE	"Vulkan_Graphics"

VkInstance vkgfx_instance = VK_NULL_HANDLE;
VkSurfaceKHR vkgfx_surface = VK_NULL_HANDLE;
VkDevice vkgfx_device = VK_NULL_HANDLE;
VkAllocationCallbacks *vkgfx_allocator = NULL;
VkQueue vkgfx_graphics_queue = VK_NULL_HANDLE;
VkQueue vkgfx_compute_queue = VK_NULL_HANDLE;
VkQueue vkgfx_transfer_queue = VK_NULL_HANDLE;
VkQueue vkgfx_present_queue = VK_NULL_HANDLE;
VkCommandPool vkgfx_graphics_pool = VK_NULL_HANDLE;
VkCommandPool vkgfx_compute_pool = VK_NULL_HANDLE;
VkCommandPool vkgfx_transfer_pool = VK_NULL_HANDLE;

VkSemaphore vkgfx_image_available_sem = VK_NULL_HANDLE;
VkSemaphore vkgfx_transfer_complete_sem = VK_NULL_HANDLE;
VkSemaphore vkgfx_scene_complete_sem = VK_NULL_HANDLE;
VkSemaphore vkgfx_render_complete_sem = VK_NULL_HANDLE;

// Command Pools
rt_array _cmd_pools;

// Frame sincronization fences
VkFence vkgfx_cb_fences[VKGFX_MAX_SWAPCHAIN_IMAGES];

vkgfx_render_framebuffer vkgfx_render_target;

// Update data buffers
VkDeviceSize _staging_offset = 0;
struct ne_buffer *_staging_buffer = NULL;
sys_mutex *_update_cb_mutex = NULL;
rt_array _update_cmd_buffers;

static uint32_t _current_image;
bool _stop = false, _cb_submitted[VKGFX_MAX_SWAPCHAIN_IMAGES];
static VkCommandBuffer _xfer_cmd_buff, _draw_cmd_buff;

ne_status vkgfx_init(void);
void vkgfx_draw(void);
void vkgfx_swap_interval(int);
void vkgfx_screen_resized(uint16_t, uint16_t);
void vkgfx_wait_idle(void);
void vkgfx_destroy_temp(void);
void vkgfx_release(void);

struct ne_gfx_module vkgfx_module =
{
	NE_GFX_API_VER,
	vkgfx_init,
	vkgfx_draw,
	vkgfx_screen_resized,
	vkgfx_swap_interval,
	vkgfx_wait_idle,
	vkgfx_init_gui_drawable,
	vkgfx_free_gui_drawable,

	vkgfx_create_texture,
	vkgfx_upload_image,
	vkgfx_destroy_texture,

	vkgfx_register_font,
	vkgfx_unregister_font,

	vkgfx_create_buffer,
	vkgfx_map_buffer,
	vkgfx_unmap_buffer,
	vkgfx_copy_buffer,
	vkgfx_upload_buffer,
	vkgfx_flush_buffer,
	vkgfx_invalidate_buffer,
	vkgfx_destroy_buffer,

	// These functions are set by the render module
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,

	vkgfx_release,

	true,
	0	// Set by the render module
};

const struct ne_gfx_module *
create_gfx_module(void)
{
	return &vkgfx_module;
}

struct vkgfx_init_func
{
	ne_status (*init)(void);
	void (*release)(void);
};

static struct vkgfx_init_func _init_funcs[] =
{
	{ vkgfx_init_types, NULL },
	{ vkgfx_init_render_target_info, NULL },
	{ vkgfx_init_instance, vkgfx_release_instance },
	{ vkgfx_init_surface, vkgfx_release_surface },
	{ vkgfx_init_device, vkgfx_release_device },
	{ vkgfx_init_swapchain, vkgfx_release_swapchain },
#ifdef _DEBUG
	{ vkgfx_init_debug, vkgfx_release_debug },
#endif
	{ vkgfx_init_dpool, vkgfx_release_dpool },
	{ vkgfx_init_cmd_pools, vkgfx_release_cmd_pools },
	{ vkgfx_init_sem, vkgfx_release_sem },
	{ vkgfx_init_pipeline, vkgfx_release_pipeline },
	{ vkgfx_init_renderpass, vkgfx_release_renderpass },
	{ vkgfx_init_framebuffers, vkgfx_release_framebuffers },
	{ vkgfx_init_gui, vkgfx_release_gui },
	{ vkgfx_init_render, vkgfx_release_render },
	{ vkgfx_init_synchronization, vkgfx_release_synchronization },
	{ vkgfx_init_cmd_buffers, vkgfx_release_cmd_buffers }
};
#define INIT_FUNC_COUNT	(sizeof(_init_funcs) / sizeof(struct vkgfx_init_func))

static inline VkCommandBuffer
_alloc_cb(
	VkCommandBufferLevel level,
	VkCommandPool pool,
	rt_array *free_list)
{
	VkResult res;
	VkCommandBuffer cmd_buff;
	VkCommandBufferAllocateInfo ai;
	memset(&ai, 0x0, sizeof(ai));
	ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	ai.commandPool = pool;
	ai.level = level;
	ai.commandBufferCount = 1;

	if ((res = vkAllocateCommandBuffers(vkgfx_device, &ai, &cmd_buff))
		!= VK_SUCCESS) {
		log_entry(VKGFX_MODULE, LOG_CRITICAL,
				"Failed to allocate command buffer: %s",
				vku_result_string(res));
		return VK_NULL_HANDLE;
	}

	rt_array_add_ptr(free_list, cmd_buff);

	return cmd_buff;
}

static inline void
_build_update_cb(void)
{
	vkgfx_gui_update();

	if (!_update_cmd_buffers.count) {
		_xfer_cmd_buff = VK_NULL_HANDLE;
		return;
	}

	/*VKU_STRUCT(VkMappedMemoryRange, mem_range,
		VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE);
	mem_range.memory = _staging_buffer->handle;
	mem_range.offset = 0;
	mem_range.size = VK_WHOLE_SIZE;

	vkFlushMappedMemoryRanges(vkgfx_device, 1, &mem_range);*/

	_xfer_cmd_buff =
		vkgfx_alloc_xfer_cmd_buff(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VKU_STRUCT(VkCommandBufferBeginInfo, bi,
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
	bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	assert(vkBeginCommandBuffer(_xfer_cmd_buff, &bi) == VK_SUCCESS);

	vkCmdExecuteCommands(_xfer_cmd_buff, (uint32_t)_update_cmd_buffers.count,
		(const VkCommandBuffer *)_update_cmd_buffers.data);

	assert(vkEndCommandBuffer(_xfer_cmd_buff) == VK_SUCCESS);
}

static inline void
_reset_thread_cp(
	VkCommandPool pool,
	rt_array *free_list)
{
	vkFreeCommandBuffers(vkgfx_device, pool,
		free_list->count, (const VkCommandBuffer *)free_list->data);
	vkResetCommandPool(vkgfx_device, pool,
		VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	free_list->count = 0;
}

static inline struct vkgfx_thread_cp *
_thread_cp(void)
{
	uint32_t id = task_worker_id();
	return rt_array_get(&_cmd_pools, id == ENGINE_MAIN_THREAD ?
		       task_num_workers() : id);
}


ne_status
vkgfx_init(void)
{
	log_entry(VKGFX_MODULE, LOG_INFORMATION, "Starting up...");

	for (uint32_t i = 0; i < INIT_FUNC_COUNT; ++i) {
		if (!_init_funcs[i].init)
			continue;

		ne_status ret = _init_funcs[i].init();

		if (ret != NE_OK)
			return ret;
	}

	sw_transition_images();
	sw_create_framebuffers();

/*	VKU_STRUCT(VkSamplerCreateInfo, sci,
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
	sci.minFilter = VK_FILTER_NEAREST;
	sci.magFilter = VK_FILTER_NEAREST;
	sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sci.addressModeU = sci.addressModeV = sci.addressModeW =
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sci.anisotropyEnable = VK_FALSE;
	sci.compareEnable = VK_FALSE;

	vkCreateSampler(vkgfx_device, &sci, vkgfx_allocator, &_gui_sampler);*/

	VkDeviceSize size = sys_config_get_int("vkgfx_staging_buffer_size", 64);
	_staging_buffer = vkgfx_create_buffer_internal(size * 1024 * 1024,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				NULL, VK_NULL_HANDLE, 0,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 3);
	vkgfx_map_ptr(_staging_buffer);

	log_entry(VKGFX_MODULE, LOG_INFORMATION, "Startup complete");

	return NE_OK;
}

void
vkgfx_draw(void)
{
	int32_t res;
	VkSubmitInfo si;
	VkPipelineStageFlags stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	if (_stop)
		return;

	memset(&si, 0x0, sizeof(si));
	si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	res = sw_next_image(vkgfx_image_available_sem, VK_NULL_HANDLE,
			&_current_image);
	if (res == VK_SW_REBUILD) {
		vkgfx_screen_resized(vkgfx_render_target.width,
			vkgfx_render_target.height);
	} else if (res == VK_SW_SKIP) {
		return;
	} else if (res == VK_SW_STOP) {
		_stop = true;
		return;
	}

	if (_cb_submitted[_current_image]) {
		vkWaitForFences(vkgfx_device, 1, &vkgfx_cb_fences[_current_image],
				VK_TRUE, UINT64_MAX);
		vkResetFences(vkgfx_device, 1, &vkgfx_cb_fences[_current_image]);

		for (uint32_t i = 0; i < _cmd_pools.count; ++i) {
			struct vkgfx_thread_cp *cp = rt_array_get(&_cmd_pools, i);

			_reset_thread_cp(
				cp->dynamic_gfx_pools[_current_image],
				&cp->gfx_free_list[_current_image]);

			_reset_thread_cp(
				cp->dynamic_xfer_pools[_current_image],
				&cp->xfer_free_list[_current_image]);

			_reset_thread_cp(
				cp->dynamic_compute_pools[_current_image],
				&cp->compute_free_list[_current_image]);
		}
	}

	ecsys_update_group(ECSYS_GROUP_RENDER);

	stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	si.waitSemaphoreCount = 1;
	si.pWaitSemaphores = &vkgfx_image_available_sem;
	si.pWaitDstStageMask = &stage;

	// Build command buffers
	int update_cb_count = 0;
	VkCommandBuffer update_cb[3];
	VkCommandBuffer render_cb[3];

	vkgfx_render_build_cb(&render_cb[0], &render_cb[1]);

	_build_update_cb();
	if (_xfer_cmd_buff != VK_NULL_HANDLE) {
		update_cb[0] = _xfer_cmd_buff;
		++update_cb_count;
	}

	VkCommandBuffer gui_cb = vkgfx_gui_build_cb(_current_image);

	// Update data
	si.commandBufferCount = update_cb_count;
	si.pCommandBuffers = update_cb;
	si.signalSemaphoreCount = 1;
	si.pSignalSemaphores = &vkgfx_transfer_complete_sem;

	assert(vkQueueSubmit(vkgfx_graphics_queue, 1, &si, VK_NULL_HANDLE)
		== VK_SUCCESS);

	stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	si.pWaitSemaphores = &vkgfx_transfer_complete_sem;

	// Scene render
	si.commandBufferCount = 2;
	si.pCommandBuffers = render_cb;
	si.signalSemaphoreCount = 1;
	si.pSignalSemaphores = &vkgfx_scene_complete_sem;

	assert(vkQueueSubmit(vkgfx_graphics_queue, 1, &si, VK_NULL_HANDLE)
		== VK_SUCCESS);

	// GUI
	stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	si.pWaitSemaphores = &vkgfx_scene_complete_sem;
	si.commandBufferCount = 1;
	si.pCommandBuffers = &gui_cb;
	si.signalSemaphoreCount = 1;
	si.pSignalSemaphores = &vkgfx_render_complete_sem;

	assert(vkQueueSubmit(vkgfx_graphics_queue, 1, &si,
		vkgfx_cb_fences[_current_image]) == VK_SUCCESS);

	_cb_submitted[_current_image] = true;
	_update_cmd_buffers.count = 0;

	res = sw_present(&vkgfx_render_complete_sem, 1, _current_image);

	vkgfx_next_buffer(_staging_buffer);
	_staging_offset = 0;

	if (res == VK_SW_REBUILD)
		vkgfx_screen_resized(vkgfx_render_target.width,
			vkgfx_render_target.height);
	else if (res == VK_SW_STOP)
		_stop = true;
}

void
vkgfx_swap_interval(int swi)
{
	//
}

void
vkgfx_screen_resized(
	uint16_t width,
	uint16_t height)
{
	if (vkgfx_instance == VK_NULL_HANDLE)
		return;

	vkgfx_render_target.width = width;
	vkgfx_render_target.height = height;

	vkDeviceWaitIdle(vkgfx_device);

	sw_resize(width, height);

	vkgfx_destroy_temp();
	vkgfx_init_framebuffers();

	vkgfx_render_screen_resized();

	vkDeviceWaitIdle(vkgfx_device);
}

void
vkgfx_stage_update(
	const void *src,
	struct ne_buffer *dst,
	VkDeviceSize size,
	VkDeviceSize offset)
{
	VkCommandBuffer cb =
		vkgfx_alloc_xfer_cmd_buff(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

	VkCommandBufferInheritanceInfo cbii;
	memset(&cbii, 0x0, sizeof(cbii));
	cbii.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

	VkCommandBufferBeginInfo tbi;
	memset(&tbi, 0x0, sizeof(tbi));
	tbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	tbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	tbi.pInheritanceInfo = &cbii;

	assert(vkBeginCommandBuffer(cb, &tbi) == VK_SUCCESS);

	VK_DBG_MARKER_INSERT(cb, "Staging Update Buffer", 1.f, .5f, 0.f, 1.f);

	memcpy((uint8_t *)vkgfx_current_ptr(_staging_buffer) + _staging_offset,
		src, size);

	vkgfx_copy_buffer_internal(dst, _staging_buffer,
		size, offset, _staging_offset, cb, false,
		VK_NULL_HANDLE, VK_NULL_HANDLE);

	_staging_offset += size;

	assert(vkEndCommandBuffer(cb) == VK_SUCCESS);

	vkgfx_register_update_cb(cb);
}

VkCommandPool
vkgfx_static_gfx_cmd_pool(void)
{
	return _thread_cp()->static_gfx_pool;
}

VkCommandPool
vkgfx_static_xfer_cmd_pool(void)
{
	return _thread_cp()->static_xfer_pool;
}

VkCommandPool
vkgfx_static_compute_cmd_pool(void)
{
	return _thread_cp()->static_compute_pool;
}

VkCommandPool
vkgfx_gfx_cmd_pool(void)
{
	return _thread_cp()->dynamic_gfx_pools[_current_image];
}

VkCommandPool
vkgfx_xfer_cmd_pool(void)
{
	return _thread_cp()->dynamic_xfer_pools[_current_image];
}

VkCommandPool
vkgfx_compute_cmd_pool(void)
{
	return _thread_cp()->dynamic_compute_pools[_current_image];
}

VkCommandBuffer
vkgfx_alloc_gfx_cmd_buff(VkCommandBufferLevel level)
{
	struct vkgfx_thread_cp *pool = _thread_cp();
	return _alloc_cb(level,
			pool->dynamic_gfx_pools[_current_image],
			&pool->gfx_free_list[_current_image]);
}

VkCommandBuffer
vkgfx_alloc_xfer_cmd_buff(VkCommandBufferLevel level)
{
	struct vkgfx_thread_cp *pool = _thread_cp();
	return _alloc_cb(level,
			pool->dynamic_xfer_pools[_current_image],
			&pool->xfer_free_list[_current_image]);
}

VkCommandBuffer
vkgfx_alloc_compute_cmd_buff(VkCommandBufferLevel level)
{
	struct vkgfx_thread_cp *pool = _thread_cp();
	return _alloc_cb(level,
			pool->dynamic_compute_pools[_current_image],
			&pool->compute_free_list[_current_image]);
}

VkCommandPool
vkgfx_current_transfer_cmd_pool(void)
{
	return vkgfx_graphics_pool;
}

VkCommandPool
vkgfx_current_graphics_cmd_pool(void)
{
	return vkgfx_graphics_pool;
}

void
vkgfx_register_update_cb(VkCommandBuffer cb)
{
	sys_mutex_lock(_update_cb_mutex);
	rt_array_add_ptr(&_update_cmd_buffers, cb);
	sys_mutex_unlock(_update_cb_mutex);
}

void
vkgfx_set_default_viewport(VkCommandBuffer cb)
{
	VkViewport vp;
	vp.x = 0;
	vp.y = 0;
	vp.width = (float)vkgfx_render_target.width;
	vp.height = (float)vkgfx_render_target.height;
	vp.minDepth = 0.f;
	vp.maxDepth = 1.f;
	VkRect2D sc;
	sc.offset.x = 0;
	sc.offset.y = 0;
	sc.extent.width = vkgfx_render_target.width;
	sc.extent.height = vkgfx_render_target.height;

	vkCmdSetViewport(cb, 0, 1, &vp);
	vkCmdSetScissor(cb, 0, 1, &sc);
}

void
vkgfx_destroy_temp(void)
{
	vkgfx_release_framebuffers();
}

void
vkgfx_wait_idle(void)
{
	vkDeviceWaitIdle(vkgfx_device);
}

void
vkgfx_release(void)
{
	if (vkgfx_instance == VK_NULL_HANDLE)
		return;

	log_entry(VKGFX_MODULE, LOG_INFORMATION, "Shutting down...");

	vkgfx_destroy_buffer(_staging_buffer);

	if (vkgfx_device != VK_NULL_HANDLE)
		vkDeviceWaitIdle(vkgfx_device);

	for (int32_t i = INIT_FUNC_COUNT - 1; i >= 0; --i) {
		if (!_init_funcs[i].release)
			continue;

		_init_funcs[i].release();
	}

	log_entry(VKGFX_MODULE, LOG_INFORMATION, "Shut down complete");
}

