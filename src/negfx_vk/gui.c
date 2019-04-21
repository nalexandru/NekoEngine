/* NekoEngine
 *
 * gui.c
 * Author: Alexandru Naiman
 *
 * Vulkan Graphics Subsystem GUI
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
#include <gui/font.h>
#include <system/log.h>
#include <system/mutex.h>
#include <ecs/ecsys.h>
#include <engine/math.h>
#include <engine/resource.h>
#include <runtime/runtime.h>

#include <gui.h>
#include <dpool.h>
#include <debug.h>
#include <vkgfx.h>
#include <vkutil.h>
#include <buffer.h>
#include <texture.h>
#include <pipeline.h>
#include <swapchain.h>
#include <renderpass.h>

#define VK_GUI_MODULE	"VulkanGUI"

struct gui_draw_info
{
	VkCommandBuffer draw_cmd_buff, update_cmd_buff;
	VkDescriptorSet ds;
	struct ne_buffer *vtx_buff;
	VkImageView image_view;
	bool rebuild;
};

struct gui_font_info
{
	struct ne_font *font;
	VkDescriptorSet ds;
	struct ne_buffer *buff, *staging_buff;
	uint32_t current;
	uint8_t *map;
};

static bool _dirty = true;
static struct ne_buffer *_gui_buffer, *_gui_index_buffer;
static VkFramebuffer _gui_fb;

static vkgfx_pipeline *_gui_pipe, *_text_pipe;
static const vkgfx_shader *_gui_shader, *_text_shader;
static VkSampler _gui_sampler;

static VkCommandBuffer _gui_cmd_buff;
static rt_array _gui_drawable_cb, _gui_update_cb;
static rt_array _gui_fonts;
static sys_mutex *_gui_cb_mutex;
static int32_t _gui_dp_type;

void gui_update_system(double dt, void **comp);
void gui_draw_system(double dt, void **comp);

int
_gui_font_cmp_func(
	const void *item,
	const void *data)
{
	const struct gui_font_info *info = item;
	return info->font == data ? 0 : 1;
}

static inline ne_status
_gui_init_buffer(void)
{
	uint16_t indices[6] = { 0, 3, 2, 0, 2, 1 };
	_gui_index_buffer = vkgfx_create_buffer_internal(sizeof(indices),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		(uint8_t *)indices,
		VK_NULL_HANDLE,
		0,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1);

	if (!_gui_index_buffer)
		return NE_NO_MEMORY;

	return NE_OK;
}

static inline ne_status
_gui_init_pipeline(void)
{
	_gui_shader = res_load("/shaders/gui.nesh", RES_SHADER);
	if (!_gui_shader)
		return NE_FAIL;

	_text_shader = res_load("/shaders/text.nesh", RES_SHADER);
	if (!_text_shader)
		return NE_FAIL;

	VkPipelineColorBlendAttachmentState gui_cbas;
	memset(&gui_cbas, 0x0, sizeof(gui_cbas));
	gui_cbas.blendEnable = VK_TRUE;
	gui_cbas.colorBlendOp = VK_BLEND_OP_ADD;
	gui_cbas.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	gui_cbas.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	gui_cbas.alphaBlendOp = VK_BLEND_OP_ADD;
	gui_cbas.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	gui_cbas.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	gui_cbas.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
				VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT |
				VK_COLOR_COMPONENT_A_BIT;

	_gui_pipe = pipe_get_graphics(VKGFX_VTX_GUI,
		_gui_shader, rp_get(RP_GUI), 0, PIPE_DEFAULT_TRIANGLES,
		&gui_cbas);
	if (!_gui_pipe)
		return NE_FAIL;

	_text_pipe = pipe_get_graphics(VKGFX_VTX_GUI,
		_text_shader, rp_get(RP_GUI), 0, PIPE_DEFAULT_TRIANGLES,
		&gui_cbas);

	return _text_pipe ? NE_OK : NE_FAIL;
}

static inline VkCommandBuffer
_build_font_update_cb(struct gui_font_info *info)
{
	VkDeviceSize size =
		info->font->vtx_buff_size + info->font->idx_buff_size;
	VkDeviceSize offset = size * info->current;
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

	VK_DBG_MARKER_INSERT(cb, "GUI_FONT_UPDATE", 1.f, .5f, 0.f, 1.f);

	vkgfx_copy_buffer_internal(info->buff, info->staging_buff,
			size, offset, offset, cb, false, VK_NULL_HANDLE,
			VK_NULL_HANDLE);

	assert(vkEndCommandBuffer(cb) == VK_SUCCESS);

	return cb;
}

static inline VkCommandBuffer
_build_font_draw_cb(struct gui_font_info *info)
{
	VkDeviceSize size =
		info->font->vtx_buff_size + info->font->idx_buff_size;
	VkDeviceSize offset = size * info->current;
	VkCommandBuffer cb =
		vkgfx_alloc_gfx_cmd_buff(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

	VkCommandBufferInheritanceInfo cbii;
	memset(&cbii, 0x0, sizeof(cbii));
	cbii.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	cbii.subpass = 0;
	cbii.renderPass = rp_get(RP_GUI);
	cbii.framebuffer = VK_NULL_HANDLE;

	VkCommandBufferBeginInfo cbbi;
	memset(&cbbi, 0x0, sizeof(cbbi));
	cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbbi.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT |
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	cbbi.pInheritanceInfo = &cbii;

	assert(vkBeginCommandBuffer(cb, &cbbi) == VK_SUCCESS);

	VK_DBG_MARKER_INSERT(cb, "GUI_FONT_DRAW", 1.f, .5f, 0.f, 1.f);

	VkDeviceSize vtx_offset = info->buff->offset + offset;
	vkCmdBindVertexBuffers(cb, 0, 1, &info->buff->handle, &vtx_offset);
	vkCmdBindIndexBuffer(cb, info->buff->handle,
		vtx_offset + info->font->vtx_buff_size,
		VK_INDEX_TYPE_UINT16);

	vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS,
		_text_pipe->pipeline);

	vkgfx_set_default_viewport(cb);

	vkCmdBindDescriptorSets(cb,
		VK_PIPELINE_BIND_POINT_GRAPHICS, _text_pipe->layout,
		0, 1, &info->ds, 0, NULL);

	/*VKU_STRUCT(VkBufferMemoryBarrier, bmb,
		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER);
	bmb.buffer = info->buff->handle;
	bmb.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	bmb.dstAccessMask = VK_ACCESS_INDEX_READ_BIT |
		VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	bmb.offset  = offset;
	bmb.size = size;
	//bmb.srcQueueFamilyIndex = vkgfx_
	//bmb.dstQueueFamilyIndex = vkgfx_graphics_queue;

	vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, NULL,
		1, &bmb, 0, NULL);*/

	vkCmdDrawIndexed(cb, info->font->idx_count, 1, 0, 0, 0);

	assert(vkEndCommandBuffer(cb) == VK_SUCCESS);

	return cb;
}

ne_status
vkgfx_init_gui(void)
{
	ne_status ret;

	memset(&_gui_drawable_cb, 0x0, sizeof(_gui_drawable_cb));
	memset(&_gui_update_cb, 0x0, sizeof(_gui_update_cb));
	memset(&_gui_fonts, 0x0, sizeof(_gui_fonts));

	if (rt_array_init(&_gui_drawable_cb, 10, sizeof(VkCommandBuffer))
			!= SYS_OK)
		return NE_NO_MEMORY;

	if (rt_array_init(&_gui_update_cb, 10, sizeof(VkCommandBuffer))
		!= SYS_OK)
		return NE_NO_MEMORY;

	if (rt_array_init(&_gui_fonts, 10, sizeof(struct gui_font_info))
		!= SYS_OK)
		return NE_NO_MEMORY;

	_gui_cb_mutex = sys_mutex_create();
	if (!_gui_cb_mutex) {
		ret = NE_NO_MEMORY;
		goto error;
	}

	if ((ret = _gui_init_buffer()) != NE_OK)
		goto error;

	if ((ret = _gui_init_pipeline()) != NE_OK)
		goto error;

	VkSamplerCreateInfo sci;
	memset(&sci, 0x0, sizeof(sci));
	sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sci.minFilter = VK_FILTER_NEAREST;
	sci.magFilter = VK_FILTER_NEAREST;
	sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sci.addressModeU = sci.addressModeV = sci.addressModeW =
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sci.anisotropyEnable = VK_FALSE;
	sci.compareEnable = VK_FALSE;

	vkCreateSampler(vkgfx_device, &sci, vkgfx_allocator, &_gui_sampler);

	comp_type_id comp = comp_get_type_id(GUI_DRAWABLE_COMP_TYPE);
	ret = ecsys_register("gui_sys", ECSYS_GROUP_RENDER, &comp, 1,
				gui_update_system, false, VKGFX_UPDATE_SYS_PRI);
	if (ret != NE_OK)
		return ret;

	ret = ecsys_register("draw_sys", ECSYS_GROUP_RENDER, &comp, 1,
				gui_draw_system, false, VKGFX_DRAW_SYS_PRI);
	if (ret != NE_OK)
		return ret;

	VkDescriptorPoolSize size;
	size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	size.descriptorCount = 1;

	_gui_dp_type = vkgfx_register_dp_type(&size, 1, 50, 1);
	if (_gui_dp_type == -1)
		goto error;

	return ret;

error:
	sys_mutex_destroy(_gui_cb_mutex);
	rt_array_release(&_gui_drawable_cb);
	return ret;
}

void
vkgfx_gui_update(void)
{
	for (size_t i = 0; i < _gui_fonts.count; ++i) {
		struct gui_font_info *info = rt_array_get(&_gui_fonts, i);
		if (info->font->idx_count)
			vkgfx_register_update_cb(_build_font_update_cb(info));
	}
}

VkCommandBuffer
vkgfx_gui_build_cb(uint32_t id)
{
	_gui_cmd_buff = vkgfx_alloc_gfx_cmd_buff(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkCommandBufferBeginInfo bi;
	memset(&bi, 0x0, sizeof(bi));
	bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	bi.pInheritanceInfo = NULL;

	assert(vkBeginCommandBuffer(_gui_cmd_buff, &bi) == VK_SUCCESS);

	for (size_t i = 0; i < _gui_fonts.count; ++i) {
		struct gui_font_info *info = rt_array_get(&_gui_fonts, i);

		if (!info->font->idx_count)
			continue;

		rt_array_add_ptr(&_gui_drawable_cb, _build_font_draw_cb(info));
		info->current = (info->current + 1) % VKGFX_MAX_SWAPCHAIN_IMAGES;

		uint8_t *base = info->map +
			((info->font->vtx_buff_size + info->font->idx_buff_size) * info->current);

		info->font->vertices = base;
		info->font->indices = (uint16_t *)(((uint8_t *)info->font->vertices)
			+ info->font->vtx_buff_size);
	}

	VkImageBlit blit;
	blit.dstSubresource.aspectMask =
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.dstSubresource.baseArrayLayer =
		blit.srcSubresource.baseArrayLayer = 0;
	blit.dstSubresource.layerCount =
		blit.srcSubresource.layerCount = 1;
	blit.dstSubresource.mipLevel =
		blit.srcSubresource.mipLevel = 0;

	blit.dstOffsets[0].x = blit.srcOffsets[0].x = 0;
	blit.dstOffsets[0].y = blit.srcOffsets[0].y = 0;
	blit.dstOffsets[0].z = blit.srcOffsets[0].z = 0;
	blit.dstOffsets[1].x = blit.srcOffsets[1].x = vkgfx_render_target.width;
	blit.dstOffsets[1].y = blit.srcOffsets[1].y = vkgfx_render_target.height;
	blit.dstOffsets[1].z = blit.srcOffsets[1].z = 1;

	vku_transition_image_layout(
			sw_image(id),
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT,
			_gui_cmd_buff,
			false,
			VK_NULL_HANDLE,
			VK_NULL_HANDLE);

	vkCmdBlitImage(_gui_cmd_buff, vkgfx_render_target.resolve.image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, sw_image(id),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
			VK_FILTER_NEAREST);

	VK_DBG_MARKER_BEGIN(_gui_cmd_buff, "GUI", .66f, .2f, .82f, 1.f);
	if (_gui_drawable_cb.count) {
		VkRenderPassBeginInfo rpbi;
		memset(&rpbi, 0x0, sizeof(rpbi));
		rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpbi.renderPass = rp_get(RP_GUI);
		rpbi.framebuffer = sw_framebuffer(id);
		rpbi.renderArea.offset.x = 0;
		rpbi.renderArea.offset.y = 0;
		rpbi.renderArea.extent.width = vkgfx_render_target.width;
		rpbi.renderArea.extent.height = vkgfx_render_target.height;
		rpbi.clearValueCount = 0;
		rpbi.pClearValues = NULL;

		vkCmdBeginRenderPass(_gui_cmd_buff, &rpbi,
			VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		vkCmdExecuteCommands(_gui_cmd_buff,
			(uint32_t)_gui_drawable_cb.count,
			(const VkCommandBuffer *)_gui_drawable_cb.data);
		vkCmdEndRenderPass(_gui_cmd_buff);
	}
	VK_DBG_MARKER_END(_gui_cmd_buff);

	assert(vkEndCommandBuffer(_gui_cmd_buff) == VK_SUCCESS);

	_gui_drawable_cb.count = 0;

	return _gui_cmd_buff;
}

ne_status
vkgfx_register_font(struct ne_font *font)
{
	struct gui_font_info info;
	memset(&info, 0x0, sizeof(info));

	info.font = font;
	if (vkgfx_ds_alloc(_gui_dp_type, &info.ds, _gui_shader->layouts[0]) != NE_OK)
		return NE_NO_MEMORY;

	info.buff = vkgfx_create_buffer_internal(
		(font->vtx_buff_size + font->idx_buff_size),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		NULL, VK_NULL_HANDLE, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VKGFX_MAX_SWAPCHAIN_IMAGES);
	if (!info.buff) {
		vkgfx_ds_free(_gui_dp_type, info.ds);
		return NE_NO_MEMORY;
	}

	info.staging_buff = vkgfx_create_buffer_internal(
		(font->vtx_buff_size + font->idx_buff_size),
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, NULL, VK_NULL_HANDLE, 0,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VKGFX_MAX_SWAPCHAIN_IMAGES);
	if (!info.buff) {
		vkgfx_destroy_buffer(info.buff);
		vkgfx_ds_free(_gui_dp_type, info.ds);
		return NE_NO_MEMORY;
	}

	info.map = NULL;
	if (vkgfx_map_buffer(info.staging_buff, 0,
				VK_WHOLE_SIZE, &info.map) != NE_OK) {
		vkgfx_destroy_buffer(info.buff);
		vkgfx_destroy_buffer(info.staging_buff);
		vkgfx_ds_free(_gui_dp_type, info.ds);
		return NE_NO_MEMORY;
	}
	info.font->vertices = info.map;
	info.font->indices = (uint16_t *)(((uint8_t *)font->vertices)
						+ font->vtx_buff_size);

	VkDescriptorImageInfo dii;
	memset(&dii, 0x0, sizeof(dii));
	dii.sampler = _gui_sampler;
	dii.imageView = info.font->texture->image_view;
	dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet wds;
	memset(&wds, 0x0, sizeof(wds));
	wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds.dstSet = info.ds;
	wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	wds.descriptorCount = 1;
	wds.pImageInfo = &dii;

	vkUpdateDescriptorSets(vkgfx_device, 1, &wds, 0, NULL);

	rt_array_add(&_gui_fonts, &info);

	return NE_OK;
}

void
vkgfx_unregister_font(struct ne_font *font)
{
	size_t id = 0;
	struct gui_font_info *info = NULL;

	id = rt_array_find_id(&_gui_fonts, font, _gui_font_cmp_func);
	if (id == RT_NOT_FOUND)
		return;

	info = rt_array_get(&_gui_fonts, id);
	if (!info)
		return;

	vkgfx_unmap_buffer(info->staging_buff);

	vkgfx_ds_free(_gui_dp_type, info->ds);
	vkgfx_destroy_buffer(info->buff);
	vkgfx_destroy_buffer(info->staging_buff);

	rt_array_remove(&_gui_fonts, id);
}

// GUI drawable

ne_status
vkgfx_init_gui_drawable(struct ne_gui_drawable_comp *draw)
{
	struct gui_draw_info *info = (struct gui_draw_info *)&draw->private;

	// FIXME: suballocate
	info->vtx_buff = vkgfx_create_buffer_internal(sizeof(draw->vertices),
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		(uint8_t *)draw->vertices, VK_NULL_HANDLE, 0,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1);
	if (!info->vtx_buff)
		return NE_NO_MEMORY;

	if (vkgfx_ds_alloc(_gui_dp_type, &info->ds, _gui_shader->layouts[0]) != NE_OK)
		return NE_NO_MEMORY;

	info->rebuild = true;
	draw->dirty = true;

	return NE_OK;
}

void
vkgfx_free_gui_drawable(struct ne_gui_drawable_comp *draw)
{
	struct gui_draw_info *info = (struct gui_draw_info *)&draw->private;

	vkgfx_ds_free(_gui_dp_type, info->ds);
	vkgfx_destroy_buffer(info->vtx_buff);
}

void
vkgfx_release_gui(void)
{
	sys_mutex_destroy(_gui_cb_mutex);
	_gui_cb_mutex = NULL;

	rt_array_release(&_gui_drawable_cb);
	rt_array_release(&_gui_update_cb);
	rt_array_release(&_gui_fonts);

	if (_gui_sampler != VK_NULL_HANDLE)
		vkDestroySampler(vkgfx_device, _gui_sampler, vkgfx_allocator);

	vkgfx_destroy_buffer(_gui_index_buffer);

	res_unload(_gui_shader, RES_SHADER);
	res_unload(_text_shader, RES_SHADER);
}

// GUI systems

void
gui_update_system(
	double dt,
	void **comp)
{
	struct ne_gui_drawable_comp *draw = comp[0];
	struct gui_draw_info *info = (struct gui_draw_info *)&draw->private;

	if (!draw->dirty)
		return;

	info->update_cmd_buff =
		vkgfx_alloc_xfer_cmd_buff(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

	VkCommandBufferInheritanceInfo cbii;
	memset(&cbii, 0x0, sizeof(cbii));
	cbii.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

	VkCommandBufferBeginInfo tbi;
	memset(&tbi, 0x0, sizeof(tbi));
	tbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	tbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	tbi.pInheritanceInfo = &cbii;

	assert(vkBeginCommandBuffer(info->update_cmd_buff, &tbi) == VK_SUCCESS);

	VK_DBG_MARKER_INSERT(info->update_cmd_buff, "GUI_DRAWABLE_UPDATE",
		1.f, .5f, 0.f, 1.f);

	vkCmdUpdateBuffer(info->update_cmd_buff, info->vtx_buff->handle,
		0, sizeof(draw->vertices), draw->vertices);

	assert(vkEndCommandBuffer(info->update_cmd_buff) == VK_SUCCESS);

	VkImageView write_iv = VK_NULL_HANDLE;
	if (info->image_view != draw->texture->image_view)
		write_iv = draw->texture->image_view;

	if (write_iv != VK_NULL_HANDLE) {
		VkDescriptorImageInfo dii;
		memset(&dii, 0x0, sizeof(dii));
		dii.sampler = _gui_sampler;
		dii.imageView = write_iv;
		dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet wds;
		memset(&wds, 0x0, sizeof(wds));
		wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds.dstSet = info->ds;
		wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		wds.descriptorCount = 1;
		wds.pImageInfo = &dii;

		vkUpdateDescriptorSets(vkgfx_device, 1, &wds, 0, NULL);
		info->image_view = write_iv;
	}

	draw->dirty = false;

	vkgfx_register_update_cb(info->update_cmd_buff);
}

void
gui_draw_system(
	double dt,
	void **comp)
{
	struct ne_gui_drawable_comp *draw = comp[0];
	struct gui_draw_info *info = (struct gui_draw_info *)&draw->private;

	info->draw_cmd_buff =
		vkgfx_alloc_gfx_cmd_buff(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

	VkCommandBufferInheritanceInfo cbii;
	memset(&cbii, 0x0, sizeof(cbii));
	cbii.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	cbii.subpass = 0;
	cbii.renderPass = rp_get(RP_GUI);
	cbii.framebuffer = VK_NULL_HANDLE;

	VkCommandBufferBeginInfo cbbi;
	memset(&cbbi, 0x0, sizeof(cbbi));
	cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	cbbi.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT |
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	cbbi.pInheritanceInfo = &cbii;

	assert(vkBeginCommandBuffer(info->draw_cmd_buff, &cbbi) == VK_SUCCESS);

	VK_DBG_MARKER_INSERT(info->draw_cmd_buff, "GUI_DRAWABLE_DRAW",
		1.f, .5f, 0.f, 1.f);

	vkCmdBindVertexBuffers(info->draw_cmd_buff, 0, 1, &info->vtx_buff->handle,
		&info->vtx_buff->offset);
	vkCmdBindIndexBuffer(info->draw_cmd_buff, _gui_index_buffer->handle,
		_gui_index_buffer->offset, VK_INDEX_TYPE_UINT16);

	vkCmdBindPipeline(info->draw_cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS,
		_gui_pipe->pipeline);

	vkgfx_set_default_viewport(info->draw_cmd_buff);

	vkCmdBindDescriptorSets(info->draw_cmd_buff,
		VK_PIPELINE_BIND_POINT_GRAPHICS, _gui_pipe->layout,
		0, 1, &info->ds, 0, NULL);

	vkCmdDrawIndexed(info->draw_cmd_buff, 6, 1, 0, 0, 0);

	assert(vkEndCommandBuffer(info->draw_cmd_buff) == VK_SUCCESS);

//	sys_mutex_lock(_gui_cb_mutex);
	rt_array_add_ptr(&_gui_drawable_cb, info->draw_cmd_buff);
//	sys_mutex_unlock(_gui_cb_mutex);
}

