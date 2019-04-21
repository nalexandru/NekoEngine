/* NekoEngine
 *
 * pipeline.c
 * Author: Alexandru Naiman
 *
 * Vulkan Graphics Subsystem Pipeline
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

#include <stdlib.h>
#include <string.h>
#include <engine/io.h>

#include <graphics/vertex.h>

#include <vkgfx.h>
#include <vkutil.h>
#include <pipeline.h>
#include <renderpass.h>

#define PIPE_MODULE	"Pipeline"
#define PIPE_CACHE_FILE	"/pipe.cache"

#define MAX_DYNAMIC_STATE	20

static VkPipelineCache _pipe_cache;
static rt_array _pipelines;
static VkVertexInputAttributeDescription _viad[50];
static VkVertexInputBindingDescription _vibd[VKGFX_VTX_TYPE_COUNT];
static VkPipelineVertexInputStateCreateInfo _visci[VKGFX_VTX_TYPE_COUNT];

int
_pipe_cmp_func(
	const void *item,
	const void *data)
{
	const vkgfx_pipeline *pipe = item;
	const vkgfx_pipeline *test = data;

	if (pipe->flags == test->flags &&
		pipe->shader == test->shader)
		return 0;

	return 1;
}

VkPipelineLayout
_pipe_create_layout(const vkgfx_shader *sh)
{
	VkResult res;
	VkPipelineLayout layout = VK_NULL_HANDLE;
	VkPipelineLayoutCreateInfo ci;
	memset(&ci, 0x0, sizeof(ci));

	ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	ci.pSetLayouts = sh->layouts;
	ci.setLayoutCount = sh->layout_count;
	ci.pushConstantRangeCount = sh->pconst_count;
	ci.pPushConstantRanges = sh->pconst_ranges;

	//for (uint32_t i = 0; i < sh->layout_count; ++i)
	//	log_entry("MATA", LOG_DEBUG, "SUGI PULA: %d

	res = vkCreatePipelineLayout(vkgfx_device, &ci, vkgfx_allocator, &layout);
	if (res != VK_SUCCESS)
		log_entry(PIPE_MODULE, LOG_CRITICAL,
			"Failed to create pipeline layout: %s",
			vku_result_string(res));

	return layout;
}

ne_status
vkgfx_init_pipeline(void)
{
	ne_file *f;
	VkResult res;
	VkPipelineCacheCreateInfo ci;
	int64_t size = 0;
	void *data = NULL;

	memset(&ci, 0x0, sizeof(ci));
	ci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	f = io_open(PIPE_CACHE_FILE, IO_READ);
	if (f) {
		data = io_read_blob(f, &size);
		io_close(f);

		ci.initialDataSize = size;
		ci.pInitialData = data;
	}

	res = vkCreatePipelineCache(vkgfx_device, &ci, vkgfx_allocator, &_pipe_cache);
	if (res != VK_SUCCESS)
		log_entry(PIPE_MODULE, LOG_WARNING,
			"Failed to create pipeline cache: %s",
			vku_result_string(res));

	free(data);

	rt_array_init(&_pipelines, 10, sizeof(vkgfx_pipeline));

	// vertex formats

	memset(&_viad, 0x0, sizeof(_viad));
	memset(&_vibd, 0x0, sizeof(_vibd));
	memset(&_visci, 0x0, sizeof(_visci));
	int next_attr = 0;

	// scene
	_visci[VKGFX_VTX_NORMAL].sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	_visci[VKGFX_VTX_NORMAL].vertexBindingDescriptionCount = 1;
	_visci[VKGFX_VTX_NORMAL].pVertexBindingDescriptions = &_vibd[VKGFX_VTX_NORMAL];
	_visci[VKGFX_VTX_NORMAL].vertexAttributeDescriptionCount = 4;
	_visci[VKGFX_VTX_NORMAL].pVertexAttributeDescriptions = &_viad[next_attr];

	_vibd[VKGFX_VTX_NORMAL].binding = 0;
	_vibd[VKGFX_VTX_NORMAL].stride = sizeof(struct ne_vertex);
	_vibd[VKGFX_VTX_NORMAL].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	_viad[next_attr].binding = 0;
	_viad[next_attr].location = 0;
	_viad[next_attr].format = VK_FORMAT_R32G32B32_SFLOAT;
	_viad[next_attr++].offset = offsetof(struct ne_vertex, pos);

	_viad[next_attr].binding = 0;
	_viad[next_attr].location = 1;
	_viad[next_attr].format = VK_FORMAT_R32G32_SFLOAT;
	_viad[next_attr++].offset = offsetof(struct ne_vertex, uv);

	_viad[next_attr].binding = 0;
	_viad[next_attr].location = 2;
	_viad[next_attr].format = VK_FORMAT_R32G32B32_SFLOAT;
	_viad[next_attr++].offset = offsetof(struct ne_vertex, normal);

	_viad[next_attr].binding = 0;
	_viad[next_attr].location = 3;
	_viad[next_attr].format = VK_FORMAT_R32G32B32_SFLOAT;
	_viad[next_attr++].offset = offsetof(struct ne_vertex, tangent);

	// gui
	_visci[VKGFX_VTX_GUI].sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	_visci[VKGFX_VTX_GUI].vertexBindingDescriptionCount = 1;
	_visci[VKGFX_VTX_GUI].pVertexBindingDescriptions = &_vibd[VKGFX_VTX_GUI];
	_visci[VKGFX_VTX_GUI].vertexAttributeDescriptionCount = 2;
	_visci[VKGFX_VTX_GUI].pVertexAttributeDescriptions = &_viad[next_attr];

	_vibd[VKGFX_VTX_GUI].binding = 0;
	_vibd[VKGFX_VTX_GUI].stride = sizeof(struct ne_gui_vertex);
	_vibd[VKGFX_VTX_GUI].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	_viad[next_attr].binding = 0;
	_viad[next_attr].location = 0;
	_viad[next_attr].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	_viad[next_attr++].offset = offsetof(struct ne_gui_vertex, pos_uv);
	_viad[next_attr].binding = 0;
	_viad[next_attr].location = 1;
	_viad[next_attr].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	_viad[next_attr++].offset = offsetof(struct ne_gui_vertex, color);

	return NE_OK;
}

static inline VkCompareOp
_get_compare_op(uint64_t bits)
{
	switch (bits) {
		case PIPE_COMPARE_OP_NEVER:
			return VK_COMPARE_OP_NEVER;
		case PIPE_COMPARE_OP_LESS:
			return VK_COMPARE_OP_LESS;
		case PIPE_COMPARE_OP_EQUAL:
			return VK_COMPARE_OP_EQUAL;
		case PIPE_COMPARE_OP_LESS_EQUAL:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case PIPE_COMPARE_OP_GREATER:
			return VK_COMPARE_OP_GREATER;
		case PIPE_COMPARE_OP_NOT_EQUAL:
			return VK_COMPARE_OP_NOT_EQUAL;
		case PIPE_COMPARE_OP_GREATER_EQUAL:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case PIPE_COMPARE_OP_ALWAYS:
			return VK_COMPARE_OP_ALWAYS;
	}

	return VK_COMPARE_OP_NEVER;
}

static inline VkStencilOp
_get_stencil_op(uint64_t bits)
{
	switch (bits) {
		case PIPE_STENCIL_OP_KEEP:
			return VK_STENCIL_OP_KEEP;
		case PIPE_STENCIL_OP_ZERO:
			return VK_STENCIL_OP_ZERO;
		case PIPE_STENCIL_OP_REPLACE:
			return VK_STENCIL_OP_REPLACE;
		case PIPE_STENCIL_OP_INC_CLAMP:
			return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		case PIPE_STENCIL_OP_DEC_CLAMP:
			return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		case PIPE_STENCIL_OP_INVERT:
			return VK_STENCIL_OP_INVERT;
		case PIPE_STENCIL_OP_INC_WRAP:
			return VK_STENCIL_OP_INCREMENT_AND_WRAP;
		case PIPE_STENCIL_OP_DEC_WRAP:
			return VK_STENCIL_OP_INCREMENT_AND_WRAP;
	}

	return VK_STENCIL_OP_KEEP;
}

vkgfx_pipeline *
pipe_get_graphics(
	vkgfx_vertex_type vtx_type,
	const vkgfx_shader *shader,
	VkRenderPass render_pass,
	uint32_t subpass,
	uint64_t flags,
	VkPipelineColorBlendAttachmentState *cba_state)
{
	vkgfx_pipeline gfx_pipe;
	gfx_pipe.flags = flags;
	gfx_pipe.shader = shader;
	gfx_pipe.rp = render_pass;
	gfx_pipe.vtx_type = vtx_type;

	vkgfx_pipeline *existing = rt_array_find(&_pipelines, &gfx_pipe,
		_pipe_cmp_func);

	if (existing)
		return existing;

	// not found; create it
	int32_t rp_cba_count = 0;
	VkResult res;
	VkPipeline pipe = VK_NULL_HANDLE;
	VkGraphicsPipelineCreateInfo ci;
	VkSpecializationInfo *sspi = NULL;
	VkSpecializationMapEntry *spme[VKGFX_STAGE_COUNT];
	int32_t *spdata[VKGFX_STAGE_COUNT];
	VkPipelineShaderStageCreateInfo *ssci = NULL;
	VkPipelineInputAssemblyStateCreateInfo iasci;
	VkPipelineTessellationStateCreateInfo tessci;
	VkPipelineViewportStateCreateInfo vpci;
	VkPipelineRasterizationStateCreateInfo rsci;
	VkPipelineMultisampleStateCreateInfo mssci;
	VkPipelineDepthStencilStateCreateInfo dssci;
	VkPipelineColorBlendStateCreateInfo cbci;
	VkPipelineDynamicStateCreateInfo dsci;
	VkPipelineColorBlendAttachmentState cba[MAX_RP_CBA_STATES + 1];
	const VkPipelineColorBlendAttachmentState *rp_cba = NULL;

	memset(&ci, 0x0, sizeof(ci));
	memset(&spme, 0x0, sizeof(spme));
	memset(&spdata, 0x0, sizeof(spdata));
	memset(&iasci, 0x0, sizeof(iasci));
	memset(&tessci, 0x0, sizeof(tessci));
	memset(&vpci, 0x0, sizeof(vpci));
	memset(&rsci, 0x0, sizeof(rsci));
	memset(&mssci, 0x0, sizeof(mssci));
	memset(&dssci, 0x0, sizeof(dssci));
	memset(&cbci, 0x0, sizeof(cbci));
	memset(&dsci, 0x0, sizeof(dsci));
	memset(cba, 0x0, sizeof(cba));

	// shader stage
	ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	ssci = calloc(shader->stage_count, sizeof(*ssci));
	if (!ssci) {
		log_entry(PIPE_MODULE, LOG_CRITICAL,
			"Failed to allocate memory");
		goto error;
	}

	sspi = calloc(shader->stage_count, sizeof(*sspi));
	if (!sspi) {
		log_entry(PIPE_MODULE, LOG_CRITICAL,
			"Failed to allocate memory");
		goto error;
	}

	ci.stageCount = shader->stage_count;
	ci.pStages = ssci;
	for (uint8_t i = 0; i < ci.stageCount; ++i) {
		ssci[i].sType =
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ssci[i].stage = shader->stages[i].stage;
		ssci[i].module = shader->stages[i].sh_mod->module;
		ssci[i].pName = "main";

		if (!shader->stages[i].sc_count)
			continue;

		spme[i] = calloc(shader->stages[i].sc_count,
				sizeof(VkSpecializationMapEntry));
		spdata[i] = calloc(shader->stages[i].sc_count,
				sizeof(int32_t));

		for (uint8_t j = 0; j < shader->stages[i].sc_count; ++j) {
			spme[i][j].constantID = shader->stages[i].sp_const[j].id;
			spme[i][j].offset = sizeof(int32_t) * j;
			spme[i][j].size = sizeof(int32_t);
			spdata[i][j] = shader->stages[i].sp_const[j].value;
		}

		sspi[i].mapEntryCount = shader->stages[i].sc_count;
		sspi[i].pMapEntries = spme[i];
		sspi[i].dataSize = sizeof(int32_t) * shader->stages[i].sc_count;
		sspi[i].pData = spdata[i];
		ssci[i].pSpecializationInfo = &sspi[i];
	}

	// vertex input
	ci.pVertexInputState = &_visci[vtx_type];

	// input assembly
	ci.pInputAssemblyState = &iasci;
	iasci.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	iasci.topology = flags & PIPE_TOPOLOGY_BITS;
	iasci.primitiveRestartEnable = flags & PIPE_PRIMITIVE_RESTART;

	// tessellation
	ci.pTessellationState = (flags & PIPE_TESSELLATION) ? &tessci : NULL;
	tessci.sType =
		VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;

	// viewport
	ci.pViewportState = &vpci;
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

	vpci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vpci.viewportCount = 1;
	vpci.pViewports = &vp;
	vpci.scissorCount = 1;
	vpci.pScissors = &sc;

	// rasterization
	ci.pRasterizationState = &rsci;
	rsci.sType =
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rsci.rasterizerDiscardEnable = flags & PIPE_DISCARD;
	rsci.depthClampEnable = flags & PIPE_DEPTH_CLAMP;
	rsci.depthBiasEnable = flags & PIPE_DEPTH_BIAS;
	rsci.lineWidth = 1.0;

	switch (flags & PIPE_POLYGON_BITS) {
	case PIPE_POLYGON_FILL:
		rsci.polygonMode = VK_POLYGON_MODE_FILL; break;
	case PIPE_POLYGON_LINE:
		rsci.polygonMode = VK_POLYGON_MODE_LINE; break;
	case PIPE_POLYGON_POINT:
		rsci.polygonMode = VK_POLYGON_MODE_POINT; break;
	case PIPE_POLYGON_FILL_RECTANGLE:
		rsci.polygonMode = VK_POLYGON_MODE_FILL_RECTANGLE_NV; break;
	}

	switch (flags & PIPE_CULL_BITS) {
	case PIPE_CULL_NONE:
		rsci.cullMode = VK_CULL_MODE_NONE; break;
	case PIPE_CULL_FRONT:
		rsci.cullMode = VK_CULL_MODE_FRONT_BIT; break;
	case PIPE_CULL_BACK:
		rsci.cullMode = VK_CULL_MODE_BACK_BIT; break;
	case PIPE_CULL_FRONT_AND_BACK:
		rsci.cullMode = VK_CULL_MODE_FRONT_AND_BACK; break;
	}

	rsci.frontFace = (flags & PIPE_CW) ?
		VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;

	// multisample
	ci.pMultisampleState = &mssci;
	mssci.sType =
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	mssci.rasterizationSamples = (flags & PIPE_MULTISAMPLE) ?
		vkgfx_render_target.samples : VK_SAMPLE_COUNT_1_BIT;
	mssci.sampleShadingEnable = false;
	mssci.minSampleShading = 1.f;

	// depth stencil
	ci.pDepthStencilState = &dssci;
	dssci.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

	dssci.depthTestEnable = (flags & PIPE_DEPTH_TEST) == PIPE_DEPTH_TEST;
	dssci.depthWriteEnable = (flags & PIPE_DEPTH_WRITE) == PIPE_DEPTH_WRITE;
	dssci.depthCompareOp =
		_get_compare_op((flags & PIPE_DEPTH_OP_BITS) >> PIPE_DEPTH_OP_OFFSET);
	dssci.depthBoundsTestEnable = (flags & PIPE_DEPTH_BOUNDS) == PIPE_DEPTH_BOUNDS;

	dssci.stencilTestEnable = (flags & PIPE_STENCIL_TEST) == PIPE_STENCIL_TEST;

	dssci.front.failOp =
		_get_stencil_op((flags & PIPE_STENCIL_FRONT_FAIL_BITS)
				>> PIPE_STENCIL_FRONT_FAIL_OFFSET);
	dssci.front.passOp = flags & PIPE_STENCIL_FRONT_PASS_BITS;
		_get_stencil_op((flags & PIPE_STENCIL_FRONT_PASS_BITS)
				>> PIPE_STENCIL_FRONT_PASS_OFFSET);
	dssci.front.depthFailOp = flags & PIPE_STENCIL_FRONT_DFAIL_BITS;
		_get_stencil_op((flags & PIPE_STENCIL_FRONT_DFAIL_BITS)
				>> PIPE_STENCIL_FRONT_DFAIL_OFFSET);
	dssci.front.compareOp =
		_get_compare_op((flags & PIPE_STENCIL_FRONT_OP_BITS)
				>> PIPE_STENCIL_FRONT_OP_OFFSET);
	dssci.front.compareMask = 0;
	dssci.front.writeMask = 1;
	dssci.front.reference = 0;

	dssci.back.failOp = flags & PIPE_STENCIL_BACK_FAIL_BITS;
		_get_stencil_op((flags & PIPE_STENCIL_BACK_FAIL_BITS)
				>> PIPE_STENCIL_BACK_FAIL_OFFSET);
	dssci.back.passOp = flags & PIPE_STENCIL_BACK_PASS_BITS;
		_get_stencil_op((flags & PIPE_STENCIL_BACK_PASS_BITS)
				>> PIPE_STENCIL_BACK_PASS_OFFSET);
	dssci.back.depthFailOp = flags & PIPE_STENCIL_BACK_DFAIL_BITS;
		_get_stencil_op((flags & PIPE_STENCIL_BACK_DFAIL_BITS)
				>> PIPE_STENCIL_BACK_DFAIL_OFFSET);
	dssci.back.compareOp =
		_get_compare_op((flags & PIPE_STENCIL_BACK_OP_BITS)
				>> PIPE_STENCIL_BACK_OP_OFFSET);
	dssci.back.compareMask = 0;
	dssci.back.writeMask = 1;
	dssci.back.reference = 0;

	dssci.minDepthBounds = 0.f;
	dssci.maxDepthBounds = 1.f;

	// color blend
	ci.pColorBlendState = &cbci;
	cbci.sType =
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	rp_cba = rp_get_extra_attachment_states(render_pass, &rp_cba_count);
	memcpy(&cba[1], rp_cba,
		sizeof(VkPipelineColorBlendAttachmentState) * rp_cba_count);

	if (cba_state) {
		memcpy(cba, cba_state, sizeof(VkPipelineColorBlendAttachmentState));
	} else {
		cba[0].blendEnable = VK_FALSE;
		if (flags & PIPE_WRITE_R)
			cba[0].colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
		if (flags & PIPE_WRITE_G)
			cba[0].colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
		if (flags & PIPE_WRITE_B)
			cba[0].colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
		if (flags & PIPE_WRITE_A)
			cba[0].colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
	}

	cbci.attachmentCount = rp_cba_count + 1;
	cbci.pAttachments = cba;
	cbci.logicOpEnable = VK_FALSE;

	// dynamic
	ci.pDynamicState = &dsci;
	dsci.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	VkDynamicState dstates[MAX_DYNAMIC_STATE];
	uint8_t dstate_count = 0;

	dstates[dstate_count++] = VK_DYNAMIC_STATE_VIEWPORT;
	dstates[dstate_count++] = VK_DYNAMIC_STATE_SCISSOR;

	if (flags & PIPE_POLYGON_LINE)
		dstates[dstate_count++] = VK_DYNAMIC_STATE_LINE_WIDTH;

	if (flags & PIPE_DEPTH_BIAS)
		dstates[dstate_count++] = VK_DYNAMIC_STATE_DEPTH_BIAS;

	if (flags & PIPE_BLEND_ENABLE)
		dstates[dstate_count++] = VK_DYNAMIC_STATE_BLEND_CONSTANTS;

	if (flags & PIPE_DEPTH_BOUNDS)
		dstates[dstate_count++] = VK_DYNAMIC_STATE_DEPTH_BOUNDS;

	if (flags & PIPE_STENCIL_TEST) {
		dstates[dstate_count++] = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
		dstates[dstate_count++] = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
		dstates[dstate_count++] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
	}

	dsci.pDynamicStates = dstates;
	dsci.dynamicStateCount = dstate_count;

	ci.layout = _pipe_create_layout(shader);
	if (ci.layout == VK_NULL_HANDLE)
		goto error;

	ci.renderPass = render_pass;
	ci.subpass = subpass;

	res = vkCreateGraphicsPipelines(vkgfx_device, _pipe_cache, 1, &ci,
		vkgfx_allocator, &pipe);
	if (res != VK_SUCCESS) {
		log_entry(PIPE_MODULE, LOG_CRITICAL,
			"Failed to graphics pipeline: %s",
			vku_result_string(res));
		goto error;
	}

	free(ssci);
	free(sspi);
	for (uint8_t i = 0; i < VKGFX_STAGE_COUNT; ++i) {
		free(spme[i]);
		free(spdata[i]);
	}

	gfx_pipe.layout = ci.layout;
	gfx_pipe.pipeline = pipe;

	if (rt_array_add(&_pipelines, &gfx_pipe) != SYS_OK)
		goto error;

	return rt_array_last(&_pipelines);

error:
	if (ci.layout != VK_NULL_HANDLE)
		vkDestroyPipelineLayout(vkgfx_device, ci.layout, vkgfx_allocator);
	free(ssci);
	free(sspi);
	for (uint8_t i = 0; i < VKGFX_STAGE_COUNT; ++i) {
		free(spme[i]);
		free(spdata[i]);
	}
	return VK_NULL_HANDLE;
}

/*
 * shader stage
 * layout
 */
vkgfx_pipeline *
pipe_get_compute(const vkgfx_shader *shader)
{
	VkResult res;
	VkPipeline pipe = VK_NULL_HANDLE;
	VkPipelineLayout layout = VK_NULL_HANDLE;
	VkComputePipelineCreateInfo ci;
	VkPipelineShaderStageCreateInfo stage;
	vkgfx_pipeline gfx_pipe;
	gfx_pipe.flags = 0;
	gfx_pipe.shader = shader;
	gfx_pipe.rp = 0;
	gfx_pipe.vtx_type = 0;

	vkgfx_pipeline *existing = rt_array_find(&_pipelines, &gfx_pipe,
		_pipe_cmp_func);

	if (existing)
		return existing;

	// not found; create it
	memset(&ci, 0x0, sizeof(ci));
	ci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;

	ci.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	ci.stage.stage = shader->stages[0].stage;
	ci.stage.module = shader->stages[0].sh_mod->module;
	ci.stage.pName = "main";

	ci.layout = _pipe_create_layout(shader);
	if (ci.layout == VK_NULL_HANDLE)
		goto error;

	res = vkCreateComputePipelines(vkgfx_device, _pipe_cache, 1, &ci,
		vkgfx_allocator, &pipe);
	if (res != VK_SUCCESS) {
		log_entry(PIPE_MODULE, LOG_CRITICAL,
			"Failed to graphics pipeline: %s",
			vku_result_string(res));
		return VK_NULL_HANDLE;
	}

	gfx_pipe.layout = ci.layout;
	gfx_pipe.pipeline = pipe;

	if (rt_array_add(&_pipelines, &gfx_pipe) != SYS_OK)
		goto error;

	return rt_array_last(&_pipelines);

error:
	if (ci.layout != VK_NULL_HANDLE)
		vkDestroyPipelineLayout(vkgfx_device, ci.layout, vkgfx_allocator);
//	free(ssci);
//	free(sspi);
//	for (uint8_t i = 0; i < VKGFX_STAGE_COUNT; ++i) {
//		free(spme[i]);
//		free(spdata[i]);
//	}
	return VK_NULL_HANDLE;
}

VkPipeline
pipe_get_ray_tracing(vkgfx_shader *shader)
{
	VkResult res;
	VkPipeline pipe = VK_NULL_HANDLE;
	/*VkRayTracingPipelineCreateInfoNV ci;
	memset(&ci, 0x0, sizeof(ci));
	ci.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;

	res = vkCreateRayTracingPipelinesNV(vkgfx_device, _pipe_cache, 1, &ci,
		vkgfx_allocator, &pipe);
	if (res != VK_SUCCESS) {
		log_entry(PIPE_MODULE, LOG_CRITICAL,
			"Failed to create ray tracing pipeline: %s",
			vku_result_string(res));
		return VK_NULL_HANDLE;
	}*/

	return pipe;
}

void
vkgfx_release_pipeline(void)
{
	ne_file *f = NULL;
	size_t size = 0;
	void *data = NULL;
	VkResult res;

	for (size_t i = 0; i < _pipelines.count; ++i) {
		vkgfx_pipeline *pipe = rt_array_get(&_pipelines, i);
		vkDestroyPipelineLayout(vkgfx_device, pipe->layout, vkgfx_allocator);
		vkDestroyPipeline(vkgfx_device, pipe->pipeline, vkgfx_allocator);
	}
	rt_array_release(&_pipelines);

	if (_pipe_cache == VK_NULL_HANDLE)
		return;

	res = vkGetPipelineCacheData(vkgfx_device, _pipe_cache, &size, data);
	if (res != VK_SUCCESS) {
		log_entry(PIPE_MODULE, LOG_WARNING,
			"Failed to get pipeline cache: %s",
			vku_result_string(res));
		goto exit;
	}

	if (!size)
		goto exit;

	data = malloc(size);
	if (!data)
		goto exit;

	res = vkGetPipelineCacheData(vkgfx_device, _pipe_cache, &size, data);
	if (res != VK_SUCCESS) {
		log_entry(PIPE_MODULE, LOG_WARNING,
			"Failed to get pipeline cache: %s",
			vku_result_string(res));
		goto exit;
	}

	f = io_open("/pipe.cache", IO_WRITE);
	if (!f)
		goto exit;

	io_write(f, data, size);
	io_close(f);

exit:
	free(data);
	vkDestroyPipelineCache(vkgfx_device, _pipe_cache, vkgfx_allocator);
	_pipe_cache = VK_NULL_HANDLE;
}

