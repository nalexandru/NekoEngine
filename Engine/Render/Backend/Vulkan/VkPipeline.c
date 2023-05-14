#include <Engine/IO.h>
#include <Engine/Engine.h>
#include <System/Log.h>
#include <System/Memory.h>

#include "VulkanBackend.h"

#define VKPMOD	"VulkanPipeline"

VkPipelineCache Vkd_pipelineCache;

static inline VkCompareOp
NeToVkCompareOp(uint64_t flags)
{
	switch (flags & RE_DEPTH_OP_BITS) {
	case RE_DEPTH_OP_LESS: return VK_COMPARE_OP_LESS;
	case RE_DEPTH_OP_EQUAL: return VK_COMPARE_OP_EQUAL;
	case RE_DEPTH_OP_LESS_EQUAL: return VK_COMPARE_OP_LESS_OR_EQUAL;
	case RE_DEPTH_OP_GREATER: return VK_COMPARE_OP_GREATER;
	case RE_DEPTH_OP_NOT_EQUAL: return VK_COMPARE_OP_NOT_EQUAL;
	case RE_DEPTH_OP_GREATER_EQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
	case RE_DEPTH_OP_ALWAYS: return VK_COMPARE_OP_ALWAYS;
	}
	
	return VK_COMPARE_OP_GREATER_OR_EQUAL;
}

static inline VkPipelineLayout CreateLayout(uint32_t size, VkShaderStageFlags pcFlags, bool inputAttachments);

struct NePipeline *
Re_BkGraphicsPipeline(const struct NeGraphicsPipelineDesc *desc)
{
	struct NePipeline *p = Sys_Alloc(1, sizeof(*p), MH_RenderBackend);
	if (!p)
		return NULL;

	VkPipelineLayout layout = CreateLayout(desc->pushConstantSize, VK_SHADER_STAGE_ALL, desc->renderPassDesc ? desc->renderPassDesc->inputAttachments > 0 : false);
	if (!layout) {
		Sys_Free(p);
		return NULL;
	}

	const uint64_t flags = desc->flags;

	VkPipelineVertexInputStateCreateInfo vi =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
	};

	if (desc->vertexDesc.attributeCount) {
		VkVertexInputAttributeDescription *attribs = Sys_Alloc(sizeof(VkVertexInputAttributeDescription), desc->vertexDesc.attributeCount, MH_Transient);
		vi.pVertexAttributeDescriptions = attribs;
		vi.vertexAttributeDescriptionCount = desc->vertexDesc.attributeCount;

		for (uint32_t i = 0; i < desc->vertexDesc.attributeCount; ++i) {
			const struct NeVertexAttribute *at = &desc->vertexDesc.attributes[i];

			attribs[i].location = at->location;
			attribs[i].binding = at->binding;
			attribs[i].format = NeToVkVertexFormat(at->format);
			attribs[i].offset = at->offset;
		}
	}

	if (desc->vertexDesc.bindingCount) {
		VkVertexInputBindingDescription *bindings = Sys_Alloc(sizeof(VkVertexInputBindingDescription), desc->vertexDesc.bindingCount, MH_Transient);
		vi.pVertexBindingDescriptions = bindings;
		vi.vertexBindingDescriptionCount = desc->vertexDesc.bindingCount;

		for (uint32_t i = 0; i < desc->vertexDesc.attributeCount; ++i) {
			const struct NeVertexBinding *b = &desc->vertexDesc.bindings[i];

			bindings[i].binding = b->binding;
			bindings[i].stride = b->stride;
			bindings[i].inputRate = b->inputRate;
		}
	}

	VkPipelineInputAssemblyStateCreateInfo ia =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.primitiveRestartEnable = VK_FALSE
	};

	switch (flags & RE_TOPOLOGY_BITS) {
	case RE_TOPOLOGY_TRIANGLES: ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
	case RE_TOPOLOGY_POINTS: ia.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST; break;
	case RE_TOPOLOGY_LINES: ia.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; break;
	}

	VkPipelineRasterizationStateCreateInfo rs =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = (flags & RE_DEPTH_CLAMP) == RE_DEPTH_CLAMP,
		.rasterizerDiscardEnable = (flags & RE_DISCARD) == RE_DISCARD,
		.depthBiasEnable = (flags & RE_DEPTH_BIAS) == RE_DEPTH_BIAS,
		.lineWidth = 1.f
	};

	switch (flags & RE_POLYGON_BITS) {
	case RE_POLYGON_FILL: rs.polygonMode = VK_POLYGON_MODE_FILL; break;
	case RE_POLYGON_LINE: rs.polygonMode = VK_POLYGON_MODE_LINE; break;
	case RE_POLYGON_POINT: rs.polygonMode = VK_POLYGON_MODE_POINT; break;
	}

	switch (flags & RE_CULL_BITS) {
	case RE_CULL_BACK: rs.cullMode = VK_CULL_MODE_BACK_BIT; break;
	case RE_CULL_FRONT: rs.cullMode = VK_CULL_MODE_FRONT_BIT; break;
	case RE_CULL_NONE: rs.cullMode = VK_CULL_MODE_NONE; break;
	case RE_CULL_FRONT_AND_BACK: rs.cullMode = VK_CULL_MODE_FRONT_AND_BACK; break;
	}

	switch (flags & RE_FRONT_FACE_BITS) {
	case RE_FRONT_FACE_CCW: rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; break;
	case RE_FRONT_FACE_CW: rs.frontFace = VK_FRONT_FACE_CLOCKWISE; break;
	}

	VkPipelineMultisampleStateCreateInfo ms =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.sampleShadingEnable = (flags & RE_SAMPLE_SHADING) == RE_SAMPLE_SHADING,
		.alphaToCoverageEnable = (flags & RE_ALPHA_TO_COVERAGE) == RE_ALPHA_TO_COVERAGE,
		.alphaToOneEnable = (flags & RE_ALPHA_TO_ONE) == RE_ALPHA_TO_ONE
	};

	if ((flags & RE_MULTISAMPLE) == RE_MULTISAMPLE) {
		switch (flags & RE_SAMPLES_BITS) {
		case RE_MS_2_SAMPLES: ms.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT; break;
		case RE_MS_4_SAMPLES: ms.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT; break;
		case RE_MS_8_SAMPLES: ms.rasterizationSamples = VK_SAMPLE_COUNT_8_BIT; break;
		case RE_MS_16_SAMPLES: ms.rasterizationSamples = VK_SAMPLE_COUNT_16_BIT; break;
		}
	} else {
		ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	}

	VkPipelineDepthStencilStateCreateInfo ds =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = (flags & RE_DEPTH_TEST) == RE_DEPTH_TEST,
		.depthWriteEnable = (flags & RE_DEPTH_WRITE) == RE_DEPTH_WRITE,
		.depthCompareOp = NeToVkCompareOp(flags & RE_DEPTH_OP_BITS),
		.depthBoundsTestEnable = (flags & RE_DEPTH_BOUNDS) == RE_DEPTH_BOUNDS,
		.stencilTestEnable = VK_FALSE, // stencil not supported in v1 of the render API
		.minDepthBounds = 0.f,
		.maxDepthBounds = 1.f
	};

	VkPipelineColorBlendAttachmentState *cbAttachments = Sys_Alloc(sizeof(*cbAttachments), desc->attachmentCount, MH_Transient);

	for (uint32_t i = 0; i < desc->attachmentCount; ++i) {
		cbAttachments[i].blendEnable = desc->attachments[i].enableBlend;

		cbAttachments[i].srcColorBlendFactor = desc->attachments[i].srcColor;
		cbAttachments[i].dstColorBlendFactor = desc->attachments[i].dstColor;
		cbAttachments[i].colorBlendOp = desc->attachments[i].colorOp;

		cbAttachments[i].srcAlphaBlendFactor = desc->attachments[i].srcAlpha;
		cbAttachments[i].dstAlphaBlendFactor = desc->attachments[i].dstAlpha;
		cbAttachments[i].alphaBlendOp = desc->attachments[i].alphaOp;

		cbAttachments[i].colorWriteMask = desc->attachments[i].writeMask;
	}

	VkPipelineColorBlendStateCreateInfo cb =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_SET,
		.attachmentCount = desc->attachmentCount,
		.pAttachments = cbAttachments,
		.blendConstants = { 0.f, 0.f, 0.f, 0.f }
	};

	VkDynamicState dynState[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH,
		VK_DYNAMIC_STATE_DEPTH_BIAS,
		VK_DYNAMIC_STATE_BLEND_CONSTANTS,
		VK_DYNAMIC_STATE_DEPTH_BOUNDS
	};
	VkPipelineDynamicStateCreateInfo dyn =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = sizeof(dynState) / sizeof(dynState[0]),
		.pDynamicStates = dynState
	};

	VkViewport vp = { 0, (float)*E_screenHeight, (float)*E_screenWidth, -(float)*E_screenHeight, 0.f, 1.f };
	VkRect2D scissor = { { 0, 0 }, { *E_screenWidth, *E_screenHeight } };

	VkPipelineViewportStateCreateInfo vpState =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &vp,
		.scissorCount = 1,
		.pScissors = &scissor
	};

	VkGraphicsPipelineCreateInfo info =
	{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pVertexInputState = &vi,
		.pInputAssemblyState = &ia,
		.pTessellationState = NULL,
		.pViewportState = &vpState,
		.pRasterizationState = &rs,
		.pMultisampleState = &ms,
		.pDepthStencilState = &ds,
		.pColorBlendState = &cb,
		.pDynamicState = &dyn,
		.layout = layout,
		.renderPass = desc->renderPassDesc ? desc->renderPassDesc->rp : VK_NULL_HANDLE
	};

	info.stageCount = desc->stageInfo->stageCount;
	VkPipelineShaderStageCreateInfo *stages = Sys_Alloc(sizeof(*stages), info.stageCount, MH_Transient);
	info.pStages = stages;

	for (uint32_t i = 0; i < info.stageCount; ++i) {
		stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[i].stage = desc->stageInfo->stages[i].stage;
		stages[i].module = desc->stageInfo->stages[i].module;
		stages[i].pName = "main";
		stages[i].pSpecializationInfo = NULL;
	}

	if (vkCreateGraphicsPipelines(Re_device->dev, Vkd_pipelineCache, 1, &info, Vkd_allocCb, &p->pipeline) != VK_SUCCESS) {
		vkDestroyPipelineLayout(Re_device->dev, layout, Vkd_allocCb);
		Sys_Free(p);
		return NULL;
	}

	p->layout = layout;

#ifdef _DEBUG
	if (desc->name) {
		VkBk_SetObjectName(Re_device->dev, p->pipeline, VK_OBJECT_TYPE_PIPELINE, desc->name);
		VkBk_SetObjectName(Re_device->dev, p->layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, desc->name);
	}
#endif

	return p;
}

struct NePipeline *
Re_BkComputePipeline(const struct NeComputePipelineDesc *desc)
{
	struct NePipeline *p = Sys_Alloc(sizeof(*p), 1, MH_RenderBackend);
	if (!p)
		return NULL;

	VkPipelineLayout layout = CreateLayout(desc->pushConstantSize, VK_SHADER_STAGE_COMPUTE_BIT, false);
	if (!layout) {
		Sys_Free(p);
		return NULL;
	}

	VkSpecializationMapEntry specMap[] =
	{
		{ 0, 0, sizeof(uint32_t) },
		{ 1, sizeof(uint32_t), sizeof(uint32_t) },
		{ 2, sizeof(uint32_t) * 2, sizeof(uint32_t) }
	};
	uint32_t specData[] = { desc->threadsPerThreadgroup.x, desc->threadsPerThreadgroup.y, desc->threadsPerThreadgroup.z };
	VkSpecializationInfo si =
	{
		.mapEntryCount = sizeof(specMap) / sizeof(specMap[0]),
		.pMapEntries = specMap,
		.dataSize = sizeof(specData),
		.pData = specData
	};
	VkComputePipelineCreateInfo info =
	{
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.stage = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_COMPUTE_BIT,
			.pSpecializationInfo = &si,
			.pName = "main"
		},
		.layout = layout
	};

	for (uint32_t i = 0; i < desc->stageInfo->stageCount; ++i) {
		if (desc->stageInfo->stages[i].stage != SS_COMPUTE)
			continue;

		info.stage.module = desc->stageInfo->stages[i].module;
		break;
	}

	if (!info.stage.module || (vkCreateComputePipelines(Re_device->dev, Vkd_pipelineCache, 1, &info, Vkd_allocCb, &p->pipeline) != VK_SUCCESS)) {
		Sys_Free(p);
		return NULL;
	}

	p->bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
	p->layout = layout;

#ifdef _DEBUG
	if (desc->name) {
		VkBk_SetObjectName(Re_device->dev, p->pipeline, VK_OBJECT_TYPE_PIPELINE, desc->name);
		VkBk_SetObjectName(Re_device->dev, p->layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, desc->name);
	}
#endif

	return p;
}

struct NePipeline *
Re_BkRayTracingPipeline(const struct NeRayTracingPipelineDesc *desc)
{
	struct NePipeline *p = Sys_Alloc(1, sizeof(*p), MH_RenderBackend);
	if (!p)
		return NULL;

	VkPipelineDynamicStateCreateInfo dyn =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
	};

	VkRayTracingShaderGroupCreateInfoKHR groups[] =
	{
		{
			.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
			.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR	// ray gen
		},
		{
			.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
			.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR	// miss
		},
		{
			.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
			.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR	// hit
		},
		{
			.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
			.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR	// proc hit
		},
	};

	VkRayTracingPipelineCreateInfoKHR info =
	{
		.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
		.pDynamicState = &dyn,
		.maxPipelineRayRecursionDepth = desc->maxDepth,
		.stageCount = desc->stageInfo->stageCount,
		.layout = VK_NULL_HANDLE
	};

	VkPipelineShaderStageCreateInfo *stages = Sys_Alloc(sizeof(*stages), info.stageCount, MH_Transient);
	//VkRayTracingShaderGroupCreateInfoKHR *groups = Sys_Alloc(sizeof(*groups), 6, MH_Transient);
	
	info.pStages = stages;

	for (uint32_t i = 0; i < info.stageCount; ++i) {
		stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[i].stage = desc->stageInfo->stages[i].stage;
		stages[i].module = desc->stageInfo->stages[i].module;
		stages[i].pName = "main";
		stages[i].pSpecializationInfo = NULL;
	}

	for (uint32_t i = 0; i < desc->stageInfo->stageCount; ++i) {
		switch (desc->stageInfo->stages[i].stage) {
		case SS_RAYGEN:
			groups[0].generalShader = i;
		break;
		case SS_MISS:
			groups[1].generalShader = i;
		break;
		case SS_CLOSEST_HIT:
			groups[2].closestHitShader = i;
		break;
		case SS_ANY_HIT:
			groups[2].anyHitShader = i;
		break;
	/*	case SS_INTERSECTION:
			groups[3].intersectionShader = i;
		break;
		case SS_CALLABLE:
		break;*/
		}

		//info.stage.module = desc->stageInfo->stages[i].module;
		break;
	}

	if (vkCreateRayTracingPipelinesKHR(Re_device->dev, VK_NULL_HANDLE, Vkd_pipelineCache, 1, &info, Vkd_allocCb, &p->pipeline) != VK_SUCCESS) {
		Sys_Free(p);
		return NULL;
	}

	return p;
}

void
Re_LoadPipelineCache(void)
{
	int64_t dataSize = 0;
	void *data = NULL;

	char *path = Sys_Alloc(sizeof(*path), 4096, MH_Transient);
	snprintf(path, 4096, "/Config/VulkanPipelineCache/%u_%u_%u.bin", Re_device->physDevProps.vendorID,
		Re_device->physDevProps.deviceID, Re_device->physDevProps.driverVersion);

	NeFile f = E_OpenFile(path, IO_READ);
	if (f) {
		data = E_ReadFileBlob(f, &dataSize, false);
		E_CloseFile(f);
	}

	VkPipelineCacheCreateInfo info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		.initialDataSize = (size_t)dataSize,
		.pInitialData = data
	};
	VkResult rc = vkCreatePipelineCache(Re_device->dev, &info, Vkd_allocCb, &Vkd_pipelineCache);
	if (rc != VK_SUCCESS)
		Sys_LogEntry(VKPMOD, LOG_WARNING, "Failed to create pipeline cache: 0x%x", rc);

	Sys_Free(data);
}

void
Re_SavePipelineCache(void)
{
	size_t dataSize = 0;
	void *data = NULL;
	vkGetPipelineCacheData(Re_device->dev, Vkd_pipelineCache, &dataSize, data);

	data = Sys_Alloc(1, dataSize, MH_RenderBackend);
	vkGetPipelineCacheData(Re_device->dev, Vkd_pipelineCache, &dataSize, data);

	E_EnableWrite(WD_Config);

	E_CreateDirectory("VulkanPipelineCache");

	char *path = Sys_Alloc(sizeof(*path), 4096, MH_Transient);
	snprintf(path, 4096, "/VulkanPipelineCache/%u_%u_%u.bin", Re_device->physDevProps.vendorID,
		Re_device->physDevProps.deviceID, Re_device->physDevProps.driverVersion);

	NeFile f = E_OpenFile(path, IO_WRITE);
	E_WriteFile(f, data, dataSize);
	E_CloseFile(f);

	E_DisableWrite();

	vkDestroyPipelineCache(Re_device->dev, Vkd_pipelineCache, Vkd_allocCb);

	Sys_Free(data);
}

void
Re_BkDestroyPipeline(struct NePipeline *pipeline)
{
	vkDestroyPipeline(Re_device->dev, pipeline->pipeline, Vkd_allocCb);
	vkDestroyPipelineLayout(Re_device->dev, pipeline->layout, Vkd_allocCb);
	Sys_Free(pipeline);
}

static inline VkPipelineLayout
CreateLayout(uint32_t size, VkShaderStageFlags pcFlags, bool inputAttachments)
{
	VkDescriptorSetLayout layouts[] = { Re_device->setLayout, Re_device->iaSetLayout };
	VkPushConstantRange range =
	{
		.stageFlags = pcFlags,
		.offset = 0,
		.size = size
	};
	VkPipelineLayoutCreateInfo info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = inputAttachments ? 2 : 1,
		.pSetLayouts = layouts,
		.pushConstantRangeCount = size > 0,
		.pPushConstantRanges = &range
	};
	VkPipelineLayout layout;
	if (vkCreatePipelineLayout(Re_device->dev, &info, Vkd_allocCb, &layout) != VK_SUCCESS)
		return VK_NULL_HANDLE;
	else
		return layout;
}

/* NekoEngine
 *
 * VkPipeline.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
