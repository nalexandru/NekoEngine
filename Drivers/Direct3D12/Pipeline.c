#include <Engine/IO.h>
#include <Engine/Engine.h>
#include <System/Log.h>
#include <System/Memory.h>

#include "D3D12Driver.h"

static inline D3D12_COMPARISON_FUNC
_NeToD3DCompareOp(uint64_t flags)
{
	switch (flags & RE_DEPTH_OP_BITS) {
	case RE_DEPTH_OP_LESS: return D3D12_COMPARISON_FUNC_LESS;
	case RE_DEPTH_OP_EQUAL: return D3D12_COMPARISON_FUNC_EQUAL;
	case RE_DEPTH_OP_LESS_EQUAL: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case RE_DEPTH_OP_GREATER: return D3D12_COMPARISON_FUNC_GREATER;
	case RE_DEPTH_OP_NOT_EQUAL: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
	case RE_DEPTH_OP_GREATER_EQUAL: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	case RE_DEPTH_OP_ALWAYS: return D3D12_COMPARISON_FUNC_ALWAYS;
	}

	return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
}

static inline D3D12_BLEND
_NeToD3DBlendFactor(enum NeBlendFactor bf)
{
	switch (bf) {
	case RE_BF_ZERO: return D3D12_BLEND_ZERO;
	case RE_BF_ONE: return D3D12_BLEND_ONE;
	case RE_BF_SRC_COLOR: return D3D12_BLEND_SRC_COLOR;
	case RE_BF_ONE_MINUS_SRC_COLOR: return D3D12_BLEND_INV_SRC_COLOR;
	case RE_BF_DST_COLOR: return D3D12_BLEND_DEST_COLOR;
	case RE_BF_ONE_MINUS_DST_COLOR: return D3D12_BLEND_INV_DEST_COLOR;
	case RE_BF_SRC_ALPHA: return D3D12_BLEND_SRC_ALPHA;
	case RE_BF_ONE_MINUS_SRC_ALPHA: return D3D12_BLEND_INV_SRC_ALPHA;
	case RE_BF_DST_ALPHA: return D3D12_BLEND_DEST_ALPHA;
	case RE_BF_ONE_MINUS_DST_ALPHA: return D3D12_BLEND_INV_DEST_ALPHA;
	case RE_BF_CONSTANT_COLOR: return D3D12_BLEND_BLEND_FACTOR;
	case RE_BF_ONE_MINUS_CONSTANT_COLOR: return D3D12_BLEND_INV_BLEND_FACTOR;
	case RE_BF_CONSTANT_ALPHA: return D3D12_BLEND_BLEND_FACTOR;
	case RE_BF_ONE_MINUS_CONSTANT_ALPHA: return D3D12_BLEND_INV_BLEND_FACTOR;
	case RE_BF_SRC_ALPHA_SATURATE: return D3D12_BLEND_SRC_ALPHA_SAT;
	case RE_BF_SRC1_COLOR: return D3D12_BLEND_SRC1_COLOR;
	case RE_BF_ONE_MINUS_SRC1_COLOR: return D3D12_BLEND_INV_SRC1_COLOR;
	case RE_BF_SRC1_ALPHA: return D3D12_BLEND_SRC1_ALPHA;
	case RE_BF_ONE_MINUS_SRC1_ALPHA: return D3D12_BLEND_INV_SRC_ALPHA;
	}

	return RE_BF_ZERO;
}

static inline D3D12_BLEND_OP
_NeToD3DBlendOp(enum NeBlendOperation bop)
{
	switch (bop) {
	case RE_BOP_ADD: return D3D12_BLEND_OP_ADD;
	case RE_BOP_SUBTRACT: return D3D12_BLEND_OP_SUBTRACT;
	case RE_BOP_REVERSE_SUBTRACT: return D3D12_BLEND_OP_REV_SUBTRACT;
	case RE_BOP_MIN: return D3D12_BLEND_OP_MIN;
	case RE_BOP_MAX: return D3D12_BLEND_OP_MAX;
	}

	return RE_BOP_ADD;
}

struct NePipeline *
D3D12_GraphicsPipeline(struct NeRenderDevice *dev, const struct NeGraphicsPipelineDesc *desc)
{
	struct NePipeline *p = Sys_Alloc(1, sizeof(*p), MH_RenderDriver);
	if (!p)
		return NULL;

	const uint64_t flags = desc->flags;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pDesc = { 0 };

	switch (flags & RE_TOPOLOGY_BITS) {
	case RE_TOPOLOGY_TRIANGLES: pDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; break;
	case RE_TOPOLOGY_POINTS: pDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT; break;
	case RE_TOPOLOGY_LINES: pDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE; break;
	}

	switch (flags & RE_POLYGON_BITS) {
	case RE_POLYGON_FILL: pDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; break;
	case RE_POLYGON_LINE: pDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME; break;
	case RE_POLYGON_POINT: pDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME; break;
	}

	switch (flags & RE_CULL_BITS) {
	case RE_CULL_BACK: pDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK; break;
	case RE_CULL_FRONT: pDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT; break;
	case RE_CULL_NONE: pDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; break;
	case RE_CULL_FRONT_AND_BACK: pDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; break;
	}

	switch (flags & RE_FRONT_FACE_BITS) {
	case RE_FRONT_FACE_CCW: pDesc.RasterizerState.FrontCounterClockwise = TRUE; break;
	case RE_FRONT_FACE_CW: pDesc.RasterizerState.FrontCounterClockwise = FALSE; break;
	}

	pDesc.RasterizerState.MultisampleEnable = (flags & RE_MULTISAMPLE) == RE_MULTISAMPLE;
	if (pDesc.RasterizerState.MultisampleEnable) {
		switch (flags & RE_SAMPLES_BITS) {
		case RE_MS_2_SAMPLES: pDesc.SampleDesc.Count = 2; break;
		case RE_MS_4_SAMPLES: pDesc.SampleDesc.Count = 4; break;
		case RE_MS_8_SAMPLES: pDesc.SampleDesc.Count = 8; break;
		case RE_MS_16_SAMPLES: pDesc.SampleDesc.Count = 16; break;
		}
		pDesc.SampleDesc.Quality = 1;
		//pDesc.SampleMask
	}

//		.sampleShadingEnable = flags & RE_SAMPLE_SHADING,
//		.alphaToCoverageEnable = flags & RE_ALPHA_TO_COVERAGE,
//		.alphaToOneEnable = flags & RE_ALPHA_TO_ONE

	pDesc.RasterizerState.DepthClipEnable = flags & RE_DEPTH_CLAMP;
	pDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	pDesc.DepthStencilState.DepthEnable = (flags & RE_DEPTH_TEST) == RE_DEPTH_TEST;
	pDesc.DepthStencilState.DepthWriteMask = (flags & RE_DEPTH_WRITE) == RE_DEPTH_WRITE ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	pDesc.DepthStencilState.DepthFunc = _NeToD3DCompareOp(flags & RE_DEPTH_OP_BITS);
	pDesc.DepthStencilState.StencilEnable = FALSE; // stencil not supported by the render api (yet)

	pDesc.BlendState.IndependentBlendEnable = TRUE;
	pDesc.BlendState.AlphaToCoverageEnable = FALSE;

	pDesc.NumRenderTargets = desc->attachmentCount;
	pDesc.DSVFormat = NeToDXGITextureFormat(desc->depthFormat);

	for (uint32_t i = 0; i < desc->attachmentCount; ++i) {
		pDesc.BlendState.RenderTarget[i].BlendEnable = desc->attachments[i].enableBlend;

		pDesc.BlendState.RenderTarget[i].SrcBlend = _NeToD3DBlendFactor(desc->attachments[i].srcColor);
		pDesc.BlendState.RenderTarget[i].DestBlend = _NeToD3DBlendFactor(desc->attachments[i].dstColor);
		pDesc.BlendState.RenderTarget[i].BlendOp = _NeToD3DBlendOp(desc->attachments[i].colorOp);

		pDesc.BlendState.RenderTarget[i].SrcBlendAlpha = _NeToD3DBlendFactor(desc->attachments[i].srcAlpha);
		pDesc.BlendState.RenderTarget[i].DestBlendAlpha = _NeToD3DBlendFactor(desc->attachments[i].dstAlpha);
		pDesc.BlendState.RenderTarget[i].BlendOpAlpha = _NeToD3DBlendOp(desc->attachments[i].alphaOp);

		pDesc.BlendState.RenderTarget[i].LogicOpEnable = FALSE;

		pDesc.BlendState.RenderTarget[i].RenderTargetWriteMask = desc->attachments[i].writeMask;

		pDesc.RTVFormats[i] = desc->renderPassDesc->rtvFormats[i];
	}

    //ID3D12RootSignature *pRootSignature;
	for (uint32_t i = 0; i < desc->stageInfo->stageCount; ++i) {
		const struct NeShaderStageDesc *sDesc = &desc->stageInfo->stages[i];
		const struct D3D12DShaderModule *sm = sDesc->module;
		struct D3D12_SHADER_BYTECODE *dst = NULL;

		switch (sDesc->stage) {
		case SS_VERTEX: dst = &pDesc.VS; break;
		case SS_FRAGMENT: dst = &pDesc.PS; break;
		case SS_TESS_CTRL: dst = &pDesc.DS; break;
		case SS_TESS_EVAL: dst = &pDesc.HS; break;
		case SS_GEOMETRY: dst = &pDesc.GS; break;
		}

		if (!dst)
			continue;

		dst->pShaderBytecode = sm->bytecode;
		dst->BytecodeLength = sm->len;
	}

    //D3D12_STREAM_OUTPUT_DESC StreamOutput;

	pDesc.NodeMask = 0;
	//pDesc.CachedPSO

	HRESULT hr = ID3D12Device5_CreateGraphicsPipelineState(dev->dev, &pDesc, &IID_ID3D12PipelineState, &p->ps);

	if (FAILED(hr)) {
		Sys_Free(p);
		return NULL;
	}

	return p;
}

struct NePipeline *
D3D12_ComputePipeline(struct NeRenderDevice *dev, const struct NeComputePipelineDesc *desc)
{
	struct NePipeline *p = Sys_Alloc(sizeof(*p), 1, MH_RenderDriver);
	if (!p)
		return NULL;

	D3D12_COMPUTE_PIPELINE_STATE_DESC pDesc = { 0 };

	//pDesc.CachedPSO = 

    //ID3D12RootSignature *pRootSignature;
	for (uint32_t i = 0; i < desc->stageInfo->stageCount; ++i) {
		if (desc->stageInfo->stages[i].stage != SS_COMPUTE)
			continue;

		const struct D3D12DShaderModule *sm = desc->stageInfo->stages[i].module;

		pDesc.CS.pShaderBytecode = sm->bytecode;
		pDesc.CS.BytecodeLength = sm->len;

		break;
	}

	HRESULT hr = ID3D12Device5_CreateComputePipelineState(dev->dev, &pDesc, &IID_ID3D12PipelineState, &p->ps);

	if (FAILED(hr)) {
		Sys_Free(p);
		return NULL;
	}

	return p;
}

struct NePipeline *
D3D12_RayTracingPipeline(struct NeRenderDevice *dev, struct NeShaderBindingTable *sbt, uint32_t maxDepth)
{
	struct NePipeline *p = Sys_Alloc(1, sizeof(*p), MH_RenderDriver);
	if (!p)
		return NULL;

/*	VkPipelineDynamicStateCreateInfo dyn =
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
		Sys_Free(p);
		return NULL;
	}*/

//	D3D12_STATE

/*	HRESULT hr = ID3D12Device5_CreatePipelineState(dev->dev, &pDesc, &IID_ID3D12PipelineState, &p->ps);

	if (FAILED(hr)) {
		Sys_Free(p);
		return NULL;
	}*/

	return p;
}

void
D3D12_LoadPipelineCache(struct NeRenderDevice *dev)
{
/*	int64_t dataSize = 0;
	void *data = NULL;

	char *path = Sys_Alloc(sizeof(*path), 4096, MH_Transient);
	snprintf(path, 4096, "/Config/VulkanPipelineCache/%d_%d_%d.bin", dev->physDevProps.vendorID, dev->physDevProps.deviceID, dev->physDevProps.driverVersion);

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
	VkResult rc = vkCreatePipelineCache(dev->dev, &info, Vkd_allocCb, &_cache);
	if (rc != VK_SUCCESS)
		Sys_LogEntry(VKPMOD, LOG_WARNING, L"Failed to create pipeline cache: 0x%x", rc);

	Sys_Free(data);*/
}

void
D3D12_SavePipelineCache(struct NeRenderDevice *dev)
{
/*	size_t dataSize = 0;
	void *data = NULL;
	vkGetPipelineCacheData(dev->dev, _cache, &dataSize, data);

	data = Sys_Alloc(1, dataSize, MH_RenderDriver);
	vkGetPipelineCacheData(dev->dev, _cache, &dataSize, data);

	E_EnableWrite(WD_Config);

	E_CreateDirectory("VulkanPipelineCache");

	char *path = Sys_Alloc(sizeof(*path), 4096, MH_Transient);
	snprintf(path, 4096, "/VulkanPipelineCache/%d_%d_%d.bin", dev->physDevProps.vendorID, dev->physDevProps.deviceID, dev->physDevProps.driverVersion);

	NeFile f = E_OpenFile(path, IO_WRITE);
	E_WriteFile(f, data, dataSize);
	E_CloseFile(f);

	E_DisableWrite();

	vkDestroyPipelineCache(dev->dev, _cache, Vkd_allocCb);

	Sys_Free(data);*/
}

void
D3D12_DestroyPipeline(struct NeRenderDevice *dev, struct NePipeline *pipeline)
{
	ID3D12PipelineState_Release(pipeline->ps);
	Sys_Free(pipeline);
}

/*static inline VkPipelineLayout
_CreateLayout(struct NeRenderDevice *dev, uint32_t size)
{
	VkPushConstantRange range =
	{
		.stageFlags = VK_SHADER_STAGE_ALL,
		.offset = 0,
		.size = size
	};
	VkPipelineLayoutCreateInfo info =
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &dev->setLayout,
		.pushConstantRangeCount = size > 0,
		.pPushConstantRanges = &range
	};
	VkPipelineLayout layout;
	if (vkCreatePipelineLayout(dev->dev, &info, Vkd_allocCb, &layout) != VK_SUCCESS)
		return VK_NULL_HANDLE;
	else
		return layout;
}*/
