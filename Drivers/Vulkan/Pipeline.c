#include <Engine/IO.h>
#include <Engine/Engine.h>
#include <System/Log.h>
#include <System/Memory.h>
#include <Render/Pipeline.h>

#include "VulkanDriver.h"

#define VKPMOD	L"VulkanPipeline"

static VkPipelineCache _cache;

static inline VkCompareOp
_NeToVkCompareOp(uint64_t flags)
{
	return VK_COMPARE_OP_LESS;
}

static inline VkBlendOp 
_NeToVkBlendOp(uint64_t flags)
{
	return VK_BLEND_OP_ADD;
}

struct PipelineLayout *
Vk_CreatePipelineLayout(struct RenderDevice *dev, const struct PipelineLayoutDesc *desc)
{
	struct PipelineLayout *l = calloc(1, sizeof(*l));
	if (!l)
		return NULL;

	VkDescriptorSetLayout *layouts = Sys_Alloc(sizeof(*layouts), desc->setLayoutCount, MH_Transient);
	for (uint32_t i = 0; i < desc->setLayoutCount; ++i)
		layouts[i] = desc->setLayouts[i]->layout;
	
	VkPipelineLayoutCreateInfo info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = desc->setLayoutCount,
		.pSetLayouts = layouts
	};

	if (desc->pushConstantSize) {
		VkPushConstantRange range =
		{
			.stageFlags = VK_SHADER_STAGE_ALL,
			.offset = 0,
			.size = desc->pushConstantSize
		};
		info.pushConstantRangeCount = 1;
		info.pPushConstantRanges = &range;
	}

	if (vkCreatePipelineLayout(dev->dev, &info, Vkd_allocCb, &l->layout) != VK_SUCCESS) {
		free(l);
		return NULL;
	}

	return l;
}

void
Vk_DestroyPipelineLayout(struct RenderDevice *dev, struct PipelineLayout *layout)
{
	vkDestroyPipelineLayout(dev->dev, layout->layout, Vkd_allocCb);
	free(layout);
}

struct Pipeline *
Vk_GraphicsPipeline(struct RenderDevice *dev, const struct GraphicsPipelineDesc *desc)
{
	struct Pipeline *p = calloc(1, sizeof(*p));
	if (!p)
		return NULL;

	uint64_t flags = desc->flags;

	VkPipelineVertexInputStateCreateInfo vi =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
	};

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
		.depthClampEnable = flags & RE_DEPTH_CLAMP,
		.rasterizerDiscardEnable = flags & RE_DISCARD,
		.depthBiasEnable = flags & RE_DEPTH_BIAS,
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
		.sampleShadingEnable = flags & RE_SAMPLE_SHADING,
		.alphaToCoverageEnable = flags & RE_ALPHA_TO_COVERAGE,
		.alphaToOneEnable = flags & RE_ALPHA_TO_ONE
	};

	if (flags & RE_MULTISAMPLE) {
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
		.depthTestEnable = flags & RE_DEPTH_TEST,
		.depthWriteEnable = flags & RE_DEPTH_WRITE,
		.depthCompareOp = _NeToVkCompareOp(flags & RE_DEPTH_OP_BITS),
		.depthBoundsTestEnable = flags & RE_DEPTH_BOUNDS,
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
		VK_DYNAMIC_STATE_DEPTH_BOUNDS,
		VK_DYNAMIC_STATE_CULL_MODE_EXT,
		VK_DYNAMIC_STATE_FRONT_FACE_EXT,
		VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT,
		VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT,
		VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT,
		VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT
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
		.layout = desc->layout->layout,
		.renderPass = desc->renderPass ? desc->renderPass->rp : VK_NULL_HANDLE
	};

	info.stageCount = desc->shader->stageCount;
	VkPipelineShaderStageCreateInfo *stages = Sys_Alloc(sizeof(*stages), info.stageCount, MH_Transient);
	info.pStages = stages;

	for (uint32_t i = 0; i < info.stageCount; ++i) {
		stages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[i].stage = desc->shader->stages[i].stage;
		stages[i].module = (VkShaderModule)desc->shader->stages[i].module;
		stages[i].pName = "main";
		stages[i].pSpecializationInfo = NULL;
	}

	if (vkCreateGraphicsPipelines(dev->dev, _cache, 1, &info, Vkd_allocCb, &p->pipeline) != VK_SUCCESS) {
		free(p);
		return NULL;
	}

	return p;
}

struct Pipeline *
Vk_ComputePipeline(struct RenderDevice *dev, struct Shader *sh)
{
	struct Pipeline *p = calloc(1, sizeof(*p));
	if (!p)
		return NULL;

	VkComputePipelineCreateInfo info =
	{
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.stage = { 0 },
		.layout = VK_NULL_HANDLE
	};

	if (vkCreateComputePipelines(dev->dev, _cache, 1, &info, Vkd_allocCb, &p->pipeline) != VK_SUCCESS) {
		free(p);
		return NULL;
	}

	return p;
}

struct Pipeline *
Vk_RayTracingPipeline(struct RenderDevice *dev, struct ShaderBindingTable *sbt, uint32_t maxDepth)
{
	struct Pipeline *p = calloc(1, sizeof(*p));
	if (!p)
		return NULL;

	VkPipelineDynamicStateCreateInfo dyn =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
	};

	VkRayTracingPipelineCreateInfoKHR info =
	{
		.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
		.pDynamicState = &dyn,
		.maxPipelineRayRecursionDepth = maxDepth,
		.layout = VK_NULL_HANDLE
	};

	info.stageCount = 0;
	info.pStages = NULL;
	info.groupCount = 0;
	info.pGroups = 0;

	if (vkCreateRayTracingPipelinesKHR(dev->dev, VK_NULL_HANDLE, _cache, 1, &info, Vkd_allocCb, &p->pipeline) != VK_SUCCESS) {
		free(p);
		return NULL;
	}

	return p;
}

void
Vk_LoadPipelineCache(struct RenderDevice *dev)
{
	int64_t dataSize = 0;
	void *data = NULL;

	char *path = Sys_Alloc(sizeof(*path), 4096, MH_Transient);
	snprintf(path, 4096, "/Config/VulkanPipelineCache/%d_%d_%d.bin", dev->physDevProps.vendorID, dev->physDevProps.deviceID, dev->physDevProps.driverVersion);

	File f = E_OpenFile(path, IO_READ);
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
	VkResult rc = vkCreatePipelineCache(dev->dev, &info, Vkd_allocCb, &_cache);
	if (rc != VK_SUCCESS)
		Sys_LogEntry(VKPMOD, LOG_WARNING, L"Failed to create pipeline cache: 0x%x", rc);

	Sys_Free(data);
}

void
Vk_SavePipelineCache(struct RenderDevice *dev)
{
	size_t dataSize = 0;
	void *data = NULL;
	vkGetPipelineCacheData(dev->dev, _cache, &dataSize, data);

	data = malloc(dataSize);
	vkGetPipelineCacheData(dev->dev, _cache, &dataSize, data);

	E_EnableWrite(WD_Config);

	E_CreateDirectory("VulkanPipelineCache");

	char *path = Sys_Alloc(sizeof(*path), 4096, MH_Transient);
	snprintf(path, 4096, "/VulkanPipelineCache/%d_%d_%d.bin", dev->physDevProps.vendorID, dev->physDevProps.deviceID, dev->physDevProps.driverVersion);

	File f = E_OpenFile(path, IO_WRITE);
	E_WriteFile(f, data, dataSize);
	E_CloseFile(f);

	E_DisableWrite();

	vkDestroyPipelineCache(dev->dev, _cache, Vkd_allocCb);

	free(data);
}

