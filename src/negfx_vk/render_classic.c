/* NekoEngine
 *
 * render_classic.c
 * Author: Alexandru Naiman
 *
 * Vulkan Graphics Subsystem Forward+ renderer
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

#include <ecs/ecsys.h>
#include <system/log.h>
#include <scene/camera.h>
#include <graphics/mesh.h>
#include <engine/math.h>
#include <engine/task.h>
#include <engine/resource.h>

#include <debug.h>
#include <vkgfx.h>
#include <dpool.h>
#include <render.h>
#include <vkutil.h>
#include <buffer.h>
#include <texture.h>
#include <pipeline.h>
#include <swapchain.h>
#include <renderpass.h>

#define CLASSIC_MAX_LIGHTS		1024
#define CLASSIC_RENDER_MODULE		"VulkanClassicRender"
#define CLASSIC_RESET_MESH_COMP_SYS	"vkgfx_classic_reset_mesh_comp_sys"

struct scene_data
{
	kmMat4 view;
	kmMat4 proj;
	kmVec4 ambient;
	kmVec4 camera_pos;
	kmVec2 screen_size;
	int32_t light_count;
	int32_t x_tile_count;
	int32_t samples;
	float gamma;

	// padding
	kmVec2 __p0;
	kmMat4 __p1;
};

struct draw_info
{
	VkDescriptorSet ds;
	struct vkgfx_pipeline *pipeline;
	uint8_t buffer_id;
	bool update_staged;
	VkDeviceSize offset;
};

struct material_info
{
	VkDescriptorSet ds;
	struct vkgfx_pipeline *pipeline;
};

static int32_t _mesh_dp_type, _mat_dp_type;

// Data
static struct scene_data _scene_data;
static struct ne_light _lights[CLASSIC_MAX_LIGHTS];
static VkDeviceSize _light_data_size = 0, _light_indices_size = 0;
static uint32_t _num_tiles = 16;
static struct ne_buffer *_scene_data_buffer, *_light_buffer;
static VkDescriptorPool _scene_dp;
static VkDescriptorSet _scene_data_ds, _light_ds;

// FIXME
static uint32_t _next_light_id = 0;

// Material samplers
static VkSampler _normal_sampler;

// Command Buffers
rt_array *depth_cb, *lighting_cb;

// Shaders
static const vkgfx_shader *_depth_shader;
static const vkgfx_shader *_lighting_shader;
static const vkgfx_shader *_light_culling_shader;

ne_status classic_init_material(struct ne_material *);
void classic_release_material(struct ne_material *);
ne_status classic_init_drawable_mesh(struct ne_drawable_mesh_comp *);
void classic_release_drawable_mesh(struct ne_drawable_mesh_comp *);
void classic_add_mesh(struct ne_drawable_mesh_comp *, size_t);
struct ne_light *classic_create_light(void);
void classic_destroy_light(struct ne_light *);

// Systems
void
classic_reset_mesh_comp_sys(double dt,
	void **comp)
{
	struct ne_drawable_mesh_comp *c = comp[0];
	struct draw_info *di = (struct draw_info *)&c->private;

	di->update_staged = false;
}

// Render interface

ne_status
vkgfx_init_classic_render(void)
{
	VkResult res;

	vkgfx_module.init_material = classic_init_material;
	vkgfx_module.release_material = classic_release_material;
	vkgfx_module.init_drawable_mesh = classic_init_drawable_mesh;
	vkgfx_module.release_drawable_mesh = classic_release_drawable_mesh;
	vkgfx_module.add_mesh = classic_add_mesh;
	vkgfx_module.create_light = classic_create_light;
	vkgfx_module.destroy_light = classic_destroy_light;
	vkgfx_module.max_lights = CLASSIC_MAX_LIGHTS;

	_scene_data_buffer = vkgfx_create_buffer_internal(
				sizeof(struct scene_data),
				VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				NULL, VK_NULL_HANDLE, 0,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 3);
	if (!_scene_data_buffer)
		return NE_NO_MEMORY;

	_light_data_size = vkgfx_min_buffer_size(sizeof(_lights),
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	_light_indices_size = vkgfx_min_buffer_size(CLASSIC_MAX_LIGHTS *
		_num_tiles * sizeof(int32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

	assert(_light_data_size >= sizeof(_lights));

	_light_buffer = vkgfx_create_buffer_internal(
				_light_data_size + _light_indices_size,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				NULL, VK_NULL_HANDLE, 0,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 3);
	if (!_light_buffer)
		return NE_NO_MEMORY;

	_depth_shader = res_load("/shaders/depth.nesh", RES_SHADER);
	if (!_depth_shader)
		return NE_FAIL;

	_lighting_shader = res_load("/shaders/lighting.nesh", RES_SHADER);
	if (!_lighting_shader)
		return NE_FAIL;

	_light_culling_shader = res_load("/shaders/light_culling.nesh", RES_SHADER);
	if (!_light_culling_shader)
		return NE_FAIL;

	VkDescriptorPoolSize sizes[3];
	memset(sizes, 0x0, sizeof(sizes));
	sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	sizes[0].descriptorCount = 2;
	sizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	sizes[1].descriptorCount = 2;
	sizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sizes[2].descriptorCount = 1;

	VKU_STRUCT(VkDescriptorPoolCreateInfo, dpci,
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
	dpci.poolSizeCount = 3;
	dpci.pPoolSizes = sizes;
	dpci.maxSets = 2;

	if ((res = vkCreateDescriptorPool(vkgfx_device, &dpci, vkgfx_allocator,
					&_scene_dp)) != VK_SUCCESS) {
		log_entry(CLASSIC_RENDER_MODULE, LOG_CRITICAL,
				"Failed to create descriptor pool: %s",
				vku_result_string(res));
		return NE_FAIL;
	}

	VKU_STRUCT(VkDescriptorSetAllocateInfo, dsai,
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
	dsai.descriptorPool = _scene_dp;
	dsai.descriptorSetCount = 1;
	dsai.pSetLayouts = _depth_shader->layouts;

	if ((res = vkAllocateDescriptorSets(vkgfx_device, &dsai, &_scene_data_ds))
			!= VK_SUCCESS) {
		log_entry(CLASSIC_RENDER_MODULE, LOG_CRITICAL,
				"Failed to allocate scene data descriptor set: %s",
				vku_result_string(res));
		return NE_FAIL;
	}

	dsai.pSetLayouts = &_lighting_shader->layouts[1];

	if ((res = vkAllocateDescriptorSets(vkgfx_device, &dsai, &_light_ds))
			!= VK_SUCCESS) {
		log_entry(CLASSIC_RENDER_MODULE, LOG_CRITICAL,
				"Failed to allocate light descriptor set: %s",
				vku_result_string(res));
		return NE_FAIL;
	}

	VkDescriptorBufferInfo dbi;
	memset(&dbi, 0x0, sizeof(dbi));
	dbi.buffer = _scene_data_buffer->handle;
	dbi.offset = 0;
	dbi.range = sizeof(struct scene_data);

	VKU_STRUCT(VkWriteDescriptorSet, wds,
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
	wds.dstSet = _scene_data_ds;
	wds.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	wds.descriptorCount = 1;
	wds.pBufferInfo = &dbi;

	vkUpdateDescriptorSets(vkgfx_device, 1, &wds, 0, NULL);

	dbi.buffer = _light_buffer->handle;
	dbi.offset = 0;
	dbi.range = _light_data_size;

	wds.dstSet = _light_ds;
	wds.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;

	vkUpdateDescriptorSets(vkgfx_device, 1, &wds, 0, NULL);

	wds.dstBinding = 1;
	dbi.offset = _light_data_size;
	dbi.range = _light_indices_size;

	vkUpdateDescriptorSets(vkgfx_device, 1, &wds, 0, NULL);

	VkDescriptorImageInfo dii;
	memset(&dii, 0x0, sizeof(dii));
	dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	dii.imageView = vkgfx_render_target.normal.image_view;
	//dii.sampler = _normal_sampler;

	wds.dstBinding = 2;
	wds.pBufferInfo = NULL;
	wds.pImageInfo = &dii;
	wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	//vkUpdateDescriptorSets(vkgfx_device, 1, &wds, 0, NULL);

	depth_cb = calloc((size_t)task_num_workers() + 1, sizeof(rt_array));
	lighting_cb = calloc((size_t)task_num_workers() + 1, sizeof(rt_array));

	if (!depth_cb || !lighting_cb)
		return NE_NO_MEMORY;

	for (uint32_t i = 0; i < task_num_workers() + 1; ++i) {
		rt_array_init_ptr(&depth_cb[i], 100);
		rt_array_init_ptr(&lighting_cb[i], 100);
	}

	VkDescriptorPoolSize size;
	size.descriptorCount = 1;

	size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	_mesh_dp_type = vkgfx_register_dp_type(&size, 1, 50, 1);
	if (_mesh_dp_type == -1)
		return NE_FAIL;

	size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	_mat_dp_type = vkgfx_register_dp_type(&size, 1, 50, 1);
	if (_mat_dp_type == -1)
		return NE_FAIL;

	VKU_STRUCT(VkSamplerCreateInfo, sci,
			VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
	sci.minFilter = VK_FILTER_LINEAR;
	sci.magFilter = VK_FILTER_LINEAR;
	sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sci.addressModeU = sci.addressModeV = sci.addressModeW =
		VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sci.anisotropyEnable = VK_FALSE;
	sci.compareEnable = VK_FALSE;

	vkCreateSampler(vkgfx_device, &sci, vkgfx_allocator, &_normal_sampler);

	comp_type_id comp_type = comp_get_type_id(DRAWABLE_MESH_COMP_TYPE);
	ecsys_register(CLASSIC_RESET_MESH_COMP_SYS, ECSYS_GROUP_MANUAL,
		&comp_type, 1, classic_reset_mesh_comp_sys, true, 0);

	dii.sampler = _normal_sampler;
	vkUpdateDescriptorSets(vkgfx_device, 1, &wds, 0, NULL);

	pipe_get_compute(_light_culling_shader);

	return NE_OK;
}

void
vkgfx_classic_render_build_cb(
	VkCommandBuffer *depth,
	VkCommandBuffer *lighting)
{
	*depth = vkgfx_alloc_gfx_cmd_buff(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	*lighting = vkgfx_alloc_gfx_cmd_buff(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkCommandBufferBeginInfo bi;
	memset(&bi, 0x0, sizeof(bi));
	bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	bi.pInheritanceInfo = NULL;

	VkClearValue clear[2];
	memset(clear, 0x0, sizeof(clear));
	clear[0].color.float32[0] = 0.f;
	clear[0].color.float32[1] = 0.f;
	clear[0].color.float32[2] = 0.f;
	clear[0].color.float32[3] = 0.f;
	clear[1].depthStencil.depth = 1.f;
	clear[1].depthStencil.stencil = 0;

	// Update
	memset(&_scene_data, 0x0, sizeof(_scene_data));

	_scene_data.light_count = _next_light_id;
	_scene_data.screen_size.x = vkgfx_render_target.width;
	_scene_data.screen_size.y = vkgfx_render_target.height;
	_scene_data.samples = vkgfx_render_target.samples;
	_scene_data.gamma = 2.2f;

	memcpy(&_scene_data.camera_pos, &ne_main_camera->pos, sizeof(kmVec3));
	memcpy(&_scene_data.proj, &ne_main_camera->proj, sizeof(kmMat4));
	memcpy(&_scene_data.view, &ne_main_camera->view, sizeof(kmMat4));

	vkgfx_stage_update(&_scene_data, _scene_data_buffer,
			sizeof(struct scene_data),
			vkgfx_current_offset(_scene_data_buffer));

	vkgfx_stage_update(_lights, _light_buffer, sizeof(_lights),
			vkgfx_current_offset(_light_buffer));

	// Z-Prepass
	assert(vkBeginCommandBuffer(*depth, &bi) == VK_SUCCESS);

	VkRenderPassBeginInfo rpbi;
	memset(&rpbi, 0x0, sizeof(rpbi));
	rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpbi.renderPass = rp_get(RP_DEPTH);
	rpbi.framebuffer = vkgfx_render_target.depth_fb;
	rpbi.renderArea.offset.x = 0;
	rpbi.renderArea.offset.y = 0;
	rpbi.renderArea.extent.width = vkgfx_render_target.width;
	rpbi.renderArea.extent.height = vkgfx_render_target.height;
	rpbi.clearValueCount = 2;
	rpbi.pClearValues = clear;

	VK_DBG_MARKER_BEGIN(*depth, "Z-Prepass", .66f, .2f, .82f, 1.f);
	vkCmdBeginRenderPass(*depth, &rpbi,
		VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	for (uint32_t i = 0; i < task_num_workers() + 1; ++i) {
		if (!depth_cb[i].count)
			continue;

		vkCmdExecuteCommands(*depth,
				(uint32_t)depth_cb[i].count,
				(const VkCommandBuffer *)depth_cb[i].data);
		depth_cb[i].count = 0;
	}

	vkCmdEndRenderPass(*depth);
	VK_DBG_MARKER_END(*depth);

	assert(vkEndCommandBuffer(*depth) == VK_SUCCESS);

	// Lighting
	assert(vkBeginCommandBuffer(*lighting, &bi) == VK_SUCCESS);

	rpbi.renderPass = rp_get(RP_LIGHTING);
	rpbi.framebuffer = vkgfx_render_target.lighting_fb;
	rpbi.clearValueCount = 1;
	rpbi.pClearValues = clear;

	VK_DBG_MARKER_BEGIN(*lighting, "Lighting", .66f, .2f, .82f, 1.f);
	vkCmdBeginRenderPass(*lighting, &rpbi,
				VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	for (uint32_t i = 0; i < task_num_workers() + 1; ++i) {
		if (!lighting_cb[i].count)
			continue;

		vkCmdExecuteCommands(*lighting,
				(uint32_t)lighting_cb[i].count,
				(const VkCommandBuffer *)lighting_cb[i].data);
		lighting_cb[i].count = 0;
	}

	vkCmdEndRenderPass(*lighting);
	VK_DBG_MARKER_END(*lighting);

	assert(vkEndCommandBuffer(*lighting) == VK_SUCCESS);

	vkgfx_next_buffer(_scene_data_buffer);
	vkgfx_next_buffer(_light_buffer);

	ecsys_update_single(CLASSIC_RESET_MESH_COMP_SYS);
}

void
vkgfx_classic_render_submit(
	VkSemaphore wait,
	VkSemaphore signal)
{
}

void
vkgfx_classic_render_screen_resized(void)
{
	VkDescriptorImageInfo dii;
	memset(&dii, 0x0, sizeof(dii));
	dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	dii.imageView = vkgfx_render_target.normal.image_view;
	dii.sampler = _normal_sampler;

	VKU_STRUCT(VkWriteDescriptorSet, wds,
		VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
	wds.dstSet = _light_ds;
	wds.dstBinding = 2;
	wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	wds.descriptorCount = 1;
	wds.pImageInfo = &dii;

	vkUpdateDescriptorSets(vkgfx_device, 1, &wds, 0, NULL);
}

void
vkgfx_release_classic_render(void)
{
	vkgfx_destroy_buffer(_scene_data_buffer);
	vkgfx_destroy_buffer(_light_buffer);

	vkDestroySampler(vkgfx_device, _normal_sampler, vkgfx_allocator);
	vkDestroyDescriptorPool(vkgfx_device, _scene_dp, vkgfx_allocator);

	for (uint32_t i = 0; i < task_num_workers() + 1; ++i) {
		rt_array_release(&depth_cb[i]);
		rt_array_release(&lighting_cb[i]);
	}

	free(depth_cb);
	free(lighting_cb);
}

// Classic rendering functions

ne_status
classic_init_material(struct ne_material *mat)
{
	ne_status ret;
	struct material_info *mi = (struct material_info *)&mat->private;

	if (vkgfx_ds_alloc(_mat_dp_type, &mi->ds,
				_lighting_shader->layouts[3]) != NE_OK) {
		ret = NE_NO_MEMORY;
		goto error;
	}

	// write ds
	VkDescriptorImageInfo dii[5];
	memset(&dii, 0x0, sizeof(dii));

	dii[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	dii[0].imageView = mat->diffuse_map->image_view;
	dii[0].sampler = _normal_sampler;

	dii[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	dii[1].imageView = mat->normal_map->image_view;
	dii[1].sampler = _normal_sampler;

	dii[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	dii[2].imageView = mat->metallic_map->image_view;
	dii[2].sampler = _normal_sampler;

	dii[3].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	dii[3].imageView = mat->roughness_map->image_view;
	dii[3].sampler = _normal_sampler;

	dii[4].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	dii[4].imageView = mat->ao_map->image_view;
	dii[4].sampler = _normal_sampler;

	VkWriteDescriptorSet wds[5];
	memset(wds, 0x0, sizeof(wds));

	wds[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds[0].dstSet = mi->ds;
	wds[0].dstBinding = 0;
	wds[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	wds[0].descriptorCount = 1;
	wds[0].pImageInfo = &dii[0];

	wds[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds[1].dstSet = mi->ds;
	wds[1].dstBinding = 1;
	wds[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	wds[1].descriptorCount = 1;
	wds[1].pImageInfo = &dii[1];

	wds[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds[2].dstSet = mi->ds;
	wds[2].dstBinding = 2;
	wds[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	wds[2].descriptorCount = 1;
	wds[2].pImageInfo = &dii[2];

	wds[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds[3].dstSet = mi->ds;
	wds[3].dstBinding = 3;
	wds[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	wds[3].descriptorCount = 1;
	wds[4].dstSet = mi->ds;
	wds[3].pImageInfo = &dii[3];

	wds[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wds[4].dstBinding = 4;
	wds[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	wds[4].descriptorCount = 1;
	wds[4].pImageInfo = &dii[4];

	vkUpdateDescriptorSets(vkgfx_device, 5, wds, 0, NULL);

	return NE_OK;

error:
	if (mi->ds)
		vkgfx_ds_free(_mesh_dp_type, mi->ds);

	return ret;
}

void
classic_release_material(struct ne_material *mat)
{
	struct material_info *mi = (struct material_info *)&mat->private;
	vkgfx_ds_free(_mesh_dp_type, mi->ds);
}

ne_status
classic_init_drawable_mesh(struct ne_drawable_mesh_comp *comp)
{
	ne_status ret;
	struct draw_info *di = (struct draw_info *)&comp->private;

	if (vkgfx_ds_alloc(_mesh_dp_type, &di->ds,
				_depth_shader->layouts[1]) != NE_OK) {
		ret = NE_NO_MEMORY;
		goto error;
	}

	comp->data_buffer = vkgfx_create_buffer_internal(
				sizeof(struct ne_mesh_data),
				VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				&comp->data, VK_NULL_HANDLE, 0,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 3);
	if (!comp->data_buffer)
		return NE_NO_MEMORY;

	// write ds
	VkDescriptorBufferInfo dbi;
	memset(&dbi, 0x0, sizeof(dbi));
	dbi.buffer = comp->data_buffer->handle;
	dbi.offset = 0;
	dbi.range = sizeof(struct ne_mesh_data);

	VKU_STRUCT(VkWriteDescriptorSet, wds,
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
	wds.dstSet = di->ds;
	wds.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	wds.descriptorCount = 1;
	wds.pBufferInfo = &dbi;

	vkUpdateDescriptorSets(vkgfx_device, 1, &wds, 0, NULL);

	return NE_OK;
error:
	if (di->ds)
		vkgfx_ds_free(_mesh_dp_type, di->ds);

	return ret;
}

void
classic_release_drawable_mesh(struct ne_drawable_mesh_comp *comp)
{
	struct draw_info *di = (struct draw_info *)&comp->private;

	vkgfx_destroy_buffer(comp->data_buffer);
	vkgfx_ds_free(_mesh_dp_type, di->ds);
}

void
classic_add_mesh(
	struct ne_drawable_mesh_comp *comp,
	size_t group)
{
	VkCommandBuffer depth, lighting;
	vkgfx_pipeline *depth_pipe, *lighting_pipe;
	struct ne_mesh_group *grp = rt_array_get(&comp->mesh->groups, group);
	struct draw_info *di = (struct draw_info *)&comp->private;
	struct ne_material *mat = rt_array_get_ptr(&comp->materials, group);
	struct material_info *mi = (struct material_info *)&mat->private;

	uint32_t depth_offsets[2] =
	{
		vkgfx_current_offset(_scene_data_buffer),
		di->offset
	};
	VkDescriptorSet depth_sets[4] =
		{ _scene_data_ds, di->ds, mi->ds };

	uint32_t lighting_offsets[4] =
	{
		vkgfx_current_offset(_scene_data_buffer),
		vkgfx_current_offset(_light_buffer),
		vkgfx_current_offset(_light_buffer),
		di->offset
	};
	VkDescriptorSet lighting_sets[4] =
		{ _scene_data_ds, _light_ds, di->ds, mi->ds };

	depth_pipe = pipe_get_graphics(VKGFX_VTX_NORMAL,
					_depth_shader, rp_get(RP_DEPTH), 0,
					PIPE_DEFAULT_TRIANGLES |
					PIPE_DEFAULT_DEPTH_WRITE |
					PIPE_MULTISAMPLE |
					PIPE_WRITE_ALL,
					NULL);

	lighting_pipe = pipe_get_graphics(VKGFX_VTX_NORMAL,
					_lighting_shader, rp_get(RP_LIGHTING), 0,
					PIPE_DEFAULT_TRIANGLES |
					PIPE_DEFAULT_DEPTH_READ |
					PIPE_MULTISAMPLE |
					PIPE_WRITE_ALL,
					NULL);

	uint32_t worker_id = task_worker_id();
	if (worker_id == ENGINE_MAIN_THREAD)
		worker_id = task_num_workers();

	if (!di->update_staged) {
		kmMat4Identity(&comp->data.mvp);
		kmMat4Multiply(&comp->data.mvp, &ne_main_camera->proj, &ne_main_camera->view);
		kmMat4Multiply(&comp->data.mvp, &comp->data.mvp, &comp->data.model);

		di->offset = vkgfx_current_offset(comp->data_buffer);
		depth_offsets[1] = di->offset;
		lighting_offsets[3] = di->offset;

		vkgfx_stage_update(&comp->data,
			comp->data_buffer,
			sizeof(struct ne_mesh_data),
			lighting_offsets[3]);

		di->update_staged = true;
		vkgfx_next_buffer(comp->data_buffer);
	}

	VkCommandBufferInheritanceInfo cbii;
	memset(&cbii, 0x0, sizeof(cbii));
	cbii.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	cbii.subpass = 0;
	cbii.framebuffer = VK_NULL_HANDLE;

	VkCommandBufferBeginInfo cbbi;
	memset(&cbbi, 0x0, sizeof(cbbi));
	cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbbi.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT |
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	cbbi.pInheritanceInfo = &cbii;

	depth = vkgfx_alloc_gfx_cmd_buff(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

	cbii.renderPass = rp_get(RP_DEPTH);
	assert(vkBeginCommandBuffer(depth, &cbbi) == VK_SUCCESS);

	VK_DBG_MARKER_INSERT(depth, "Z Prepass Mesh", 1.f, .5f, 0.f, 1.f);

	vkCmdBindVertexBuffers(depth, 0, 1, &comp->mesh->vtx_buffer->handle,
				&comp->mesh->vtx_buffer->offset);
	vkCmdBindIndexBuffer(depth, comp->mesh->idx_buffer->handle,
				comp->mesh->idx_buffer->offset, VK_INDEX_TYPE_UINT32);

	vkCmdBindPipeline(depth, VK_PIPELINE_BIND_POINT_GRAPHICS,
				depth_pipe->pipeline);

	vkgfx_set_default_viewport(depth);

	vkCmdBindDescriptorSets(depth, VK_PIPELINE_BIND_POINT_GRAPHICS,
				depth_pipe->layout, 0,
				3, depth_sets, 2, depth_offsets);

	vkCmdDrawIndexed(depth, grp->idx_count, 1, grp->idx_offset,
							grp->vtx_offset, 0);

	assert(vkEndCommandBuffer(depth) == VK_SUCCESS);

	// lighting

	lighting = vkgfx_alloc_gfx_cmd_buff(VK_COMMAND_BUFFER_LEVEL_SECONDARY);

	cbii.renderPass = rp_get(RP_LIGHTING);
	assert(vkBeginCommandBuffer(lighting, &cbbi) == VK_SUCCESS);

	VK_DBG_MARKER_INSERT(lighting, "Lighting Mesh", 1.f, .5f, 0.f, 1.f);

	vkCmdBindVertexBuffers(lighting, 0, 1, &comp->mesh->vtx_buffer->handle,
				&comp->mesh->vtx_buffer->offset);
	vkCmdBindIndexBuffer(lighting, comp->mesh->idx_buffer->handle,
				comp->mesh->idx_buffer->offset, VK_INDEX_TYPE_UINT32);

	vkCmdBindPipeline(lighting, VK_PIPELINE_BIND_POINT_GRAPHICS,
				lighting_pipe->pipeline);

	vkgfx_set_default_viewport(lighting);

	vkCmdBindDescriptorSets(lighting, VK_PIPELINE_BIND_POINT_GRAPHICS,
				lighting_pipe->layout, 0,
				4, lighting_sets, 4, lighting_offsets);

	vkCmdPushConstants(lighting, lighting_pipe->layout, VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(mat->data), &mat->data);

	vkCmdDrawIndexed(lighting, grp->idx_count, 1, grp->idx_offset,
							grp->vtx_offset, 0);

	assert(vkEndCommandBuffer(lighting) == VK_SUCCESS);

	rt_array_add_ptr(&depth_cb[worker_id], depth);
	rt_array_add_ptr(&lighting_cb[worker_id], lighting);
}

// Light
struct ne_light *
classic_create_light(void)
{
	return &_lights[_next_light_id++];
}

void
classic_destroy_light(struct ne_light *l)
{
	// FIXME
}
