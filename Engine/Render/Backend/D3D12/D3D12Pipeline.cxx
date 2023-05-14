#include <Engine/IO.h>
#include <Engine/Config.h>
#include <Engine/Engine.h>
#include <System/Log.h>
#include <System/Memory.h>

#include "D3D12Backend.h"

#define D3D12PMOD	"D3D12Pipeline"

static ID3D12RootSignature *f_localDummy, *f_globalDummy;

static const char *f_semanticNames[]
{
	"POSITION",
	"BLENDWEIGHT",
	"BLENDINDICES",
	"NORMAL",
	"TEXCOORD0",
	"TEXCOORD1",
	"TANGENT",
	"BINORMAL",
	"TESSFACTOR",
	"COLOR0"
};

/*VkPipelineCache Vkd_pipelineCache;*/

static inline D3D12_COMPARISON_FUNC
NeToD3D12CompareFunc(uint64_t flags)
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

	return D3D12_COMPARISON_FUNC_NEVER;
}

static inline D3D12_BLEND
NeToD3D12BlendFactor(enum NeBlendFactor bf)
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
	case RE_BF_CONSTANT_ALPHA: return D3D12_BLEND_ALPHA_FACTOR;
	case RE_BF_ONE_MINUS_CONSTANT_ALPHA: return D3D12_BLEND_INV_ALPHA_FACTOR;
	case RE_BF_SRC_ALPHA_SATURATE: return D3D12_BLEND_SRC_ALPHA_SAT;
	case RE_BF_SRC1_COLOR: return D3D12_BLEND_SRC1_COLOR;
	case RE_BF_ONE_MINUS_SRC1_COLOR: return D3D12_BLEND_INV_SRC1_COLOR;
	case RE_BF_SRC1_ALPHA: return D3D12_BLEND_SRC1_ALPHA;
	case RE_BF_ONE_MINUS_SRC1_ALPHA: return D3D12_BLEND_INV_SRC1_ALPHA;
	default: return D3D12_BLEND_ZERO;
	}
}

static inline D3D12_BLEND_OP
NeToD3D12BlendOp(enum NeBlendOperation bo)
{
	switch (bo) {
	case RE_BOP_ADD: return D3D12_BLEND_OP_ADD;
	case RE_BOP_SUBTRACT: return D3D12_BLEND_OP_SUBTRACT;
	case RE_BOP_REVERSE_SUBTRACT: return D3D12_BLEND_OP_REV_SUBTRACT;
	case RE_BOP_MIN: return D3D12_BLEND_OP_MIN;
	case RE_BOP_MAX: return D3D12_BLEND_OP_MAX;
	default: return D3D12_BLEND_OP_ADD;
	}
}

static inline ID3D12RootSignature *CreateRootSignature(uint32_t size, D3D12_SHADER_VISIBILITY vis, bool inputAttachments);

struct NePipeline *
Re_BkGraphicsPipeline(const struct NeGraphicsPipelineDesc *desc)
{
	NePipeline *p = (NePipeline *)Sys_Alloc(1, sizeof(*p), MH_RenderBackend);
	if (!p)
		return nullptr;

	p->rs = CreateRootSignature(desc->pushConstantSize, D3D12_SHADER_VISIBILITY_ALL, desc->renderPassDesc ? desc->renderPassDesc->inputAttachments > 0 : false);
	if (!p->rs) {
		Sys_Free(p);
		return nullptr;
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psDesc{};
	psDesc.NodeMask = 0;
	psDesc.SampleMask = UINT_MAX;
	psDesc.pRootSignature = p->rs;

	const uint64_t flags = desc->flags;

	D3D12_INPUT_ELEMENT_DESC *inputDesc = (D3D12_INPUT_ELEMENT_DESC *)Sys_Alloc(sizeof(*inputDesc), desc->vertexDesc.attributeCount, MH_Transient);
	for (uint32_t i = 0; i < desc->vertexDesc.attributeCount; ++i) {
		const struct NeVertexBinding *b = &desc->vertexDesc.bindings[i];
		const struct NeVertexAttribute *at = &desc->vertexDesc.attributes[i];

		inputDesc[i].SemanticName = f_semanticNames[at->semantic];
		inputDesc[i].SemanticIndex = 0;
		inputDesc[i].Format = NeVertexToDXGIFormat(at->format);
		inputDesc[i].InputSlot = at->binding;
		inputDesc[i].AlignedByteOffset = at->offset;
		inputDesc[i].InputSlotClass = (D3D12_INPUT_CLASSIFICATION)b->inputRate;
		inputDesc[i].InstanceDataStepRate = 0;

		p->vertexBufferStride[b->binding] = b->stride;
	}

	psDesc.InputLayout = { inputDesc, desc->vertexDesc.attributeCount };

	switch (flags & RE_TOPOLOGY_BITS) {
		case RE_TOPOLOGY_TRIANGLES:
			psDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			p->topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		break;
		case RE_TOPOLOGY_POINTS:
			psDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
			p->topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		break;
		case RE_TOPOLOGY_LINES:
			psDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
			p->topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		break;
	}

	switch (flags & RE_POLYGON_BITS) {
		case RE_POLYGON_FILL: psDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; break;
		case RE_POLYGON_LINE: psDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME; break;
	}

	switch (flags & RE_CULL_BITS) {
		case RE_CULL_BACK: psDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK; break;
		case RE_CULL_FRONT: psDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT; break;
		case RE_CULL_NONE: psDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; break;
	}

	switch (flags & RE_FRONT_FACE_BITS) {
		case RE_FRONT_FACE_CCW: psDesc.RasterizerState.FrontCounterClockwise = true; break;
		case RE_FRONT_FACE_CW: psDesc.RasterizerState.FrontCounterClockwise = false; break;
	}

	psDesc.RasterizerState.DepthBias = flags & RE_DEPTH_BIAS;
	psDesc.RasterizerState.DepthClipEnable = flags & RE_DEPTH_CLAMP;

	psDesc.RasterizerState.MultisampleEnable = (flags & RE_MULTISAMPLE) == RE_MULTISAMPLE;
	if (psDesc.RasterizerState.MultisampleEnable) {
		switch (flags & RE_SAMPLES_BITS) {
			case RE_MS_2_SAMPLES: psDesc.SampleDesc.Count = 2; break;
			case RE_MS_4_SAMPLES: psDesc.SampleDesc.Count = 4; break;
			case RE_MS_8_SAMPLES: psDesc.SampleDesc.Count = 8; break;
			case RE_MS_16_SAMPLES: psDesc.SampleDesc.Count = 16; break;
		}
		psDesc.SampleDesc.Quality = DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN;
	} else {
		psDesc.SampleDesc.Count = 1;
	}

	psDesc.DepthStencilState.DepthEnable = (flags & RE_DEPTH_TEST) == RE_DEPTH_TEST;
	psDesc.DepthStencilState.DepthWriteMask = (flags & RE_DEPTH_WRITE) == RE_DEPTH_WRITE ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	psDesc.DepthStencilState.DepthFunc = NeToD3D12CompareFunc(flags & RE_DEPTH_OP_BITS);
	psDesc.DepthStencilState.StencilEnable = FALSE; // stencil not supported in v1 of the render API
	// TODO: depth bounds - (flags & RE_DEPTH_BOUNDS) == RE_DEPTH_BOUNDS,

	psDesc.BlendState.AlphaToCoverageEnable = (flags & RE_ALPHA_TO_COVERAGE) == RE_ALPHA_TO_COVERAGE;
	psDesc.BlendState.IndependentBlendEnable = TRUE;

	for (uint32_t i = 0; i < desc->attachmentCount; ++i) {
		psDesc.RTVFormats[i] = desc->renderPassDesc->rtvFormats[i];

		// Blend
		psDesc.BlendState.RenderTarget[i].BlendEnable = desc->attachments[i].enableBlend;

		psDesc.BlendState.RenderTarget[i].LogicOpEnable = FALSE;	// not supported in v1
		psDesc.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_SET;

		psDesc.BlendState.RenderTarget[i].SrcBlend = NeToD3D12BlendFactor(desc->attachments[i].srcColor);
		psDesc.BlendState.RenderTarget[i].DestBlend = NeToD3D12BlendFactor(desc->attachments[i].dstColor);
		psDesc.BlendState.RenderTarget[i].BlendOp = NeToD3D12BlendOp(desc->attachments[i].colorOp);

		psDesc.BlendState.RenderTarget[i].SrcBlendAlpha = NeToD3D12BlendFactor(desc->attachments[i].srcAlpha);
		psDesc.BlendState.RenderTarget[i].DestBlendAlpha = NeToD3D12BlendFactor(desc->attachments[i].dstAlpha);
		psDesc.BlendState.RenderTarget[i].BlendOpAlpha = NeToD3D12BlendOp(desc->attachments[i].alphaOp);

		psDesc.BlendState.RenderTarget[i].RenderTargetWriteMask = desc->attachments[i].writeMask;
	}

	psDesc.NumRenderTargets = desc->attachmentCount;
	psDesc.DSVFormat = desc->renderPassDesc->depthFormat;

	for (uint32_t i = 0; i < desc->stageInfo->stageCount; ++i) {
		switch (desc->stageInfo->stages[i].stage) {
			case SS_VERTEX: psDesc.VS = *(D3D12_SHADER_BYTECODE *)desc->stageInfo->stages[i].module; break;
			case SS_FRAGMENT: psDesc.PS = *(D3D12_SHADER_BYTECODE *)desc->stageInfo->stages[i].module; break;
			case SS_GEOMETRY: psDesc.GS = *(D3D12_SHADER_BYTECODE *)desc->stageInfo->stages[i].module; break;
			//case SS_MESH: psDesc.MS = *(D3D12_SHADER_BYTECODE *)desc->stageInfo->stages[i].module; break;
			//case SS_TASK: psDesc.AS = *(D3D12_SHADER_BYTECODE *)desc->stageInfo->stages[i].module; break;
			case SS_TESS_CTRL: psDesc.HS = *(D3D12_SHADER_BYTECODE *)desc->stageInfo->stages[i].module; break;
			case SS_TESS_EVAL: psDesc.DS = *(D3D12_SHADER_BYTECODE *)desc->stageInfo->stages[i].module; break;
		}
	}

	if (FAILED(Re_device->dev->CreateGraphicsPipelineState(&psDesc, IID_PPV_ARGS(&p->ps)))) {
		p->rs->Release();
		Sys_Free(p);
		return nullptr;
	}

	// TODO:
	// .rasterizerDiscardEnable = flags & RE_DISCARD,
	// .depthBiasEnable = flags & RE_DEPTH_BIAS,
	// .sampleShadingEnable = flags & RE_SAMPLE_SHADING,
	// .alphaToOneEnable = flags & RE_ALPHA_TO_ONE
	// //rs.DepthBiasClamp = 0.f;
	// psDesc.CachedPSO

#ifdef _DEBUG
	if (desc->name) {
		p->ps->SetName(NeWin32_UTF8toUCS2(desc->name));
		p->rs->SetName(NeWin32_UTF8toUCS2(desc->name));
	}
#endif

	return p;
}

struct NePipeline *
Re_BkComputePipeline(const struct NeComputePipelineDesc *desc)
{
	NePipeline *p = (NePipeline *)Sys_Alloc(sizeof(*p), 1, MH_RenderBackend);
	if (!p)
		return nullptr;

	p->rs = CreateRootSignature(desc->pushConstantSize, D3D12_SHADER_VISIBILITY_ALL, false);
	if (!p->rs) {
		Sys_Free(p);
		return nullptr;
	}

	D3D12_COMPUTE_PIPELINE_STATE_DESC psDesc{};
	psDesc.NodeMask = 0;
	psDesc.pRootSignature = p->rs;

	for (uint32_t i = 0; i < desc->stageInfo->stageCount; ++i) {
		if (desc->stageInfo->stages[i].stage != SS_COMPUTE)
			continue;

		psDesc.CS = *(D3D12_SHADER_BYTECODE *)desc->stageInfo->stages[i].module;
		break;
	}

	if (!psDesc.CS.pShaderBytecode || (FAILED(Re_device->dev->CreateComputePipelineState(&psDesc, IID_PPV_ARGS(&p->ps))))) {
		p->rs->Release();
		Sys_Free(p);
		return NULL;
	}

#ifdef _DEBUG
	if (desc->name) {
		p->ps->SetName(NeWin32_UTF8toUCS2(desc->name));
		p->rs->SetName(NeWin32_UTF8toUCS2(desc->name));
	}
#endif

	return p;
}

struct NePipeline *
Re_BkRayTracingPipeline(const struct NeRayTracingPipelineDesc *desc)
{
	NePipeline *p = (NePipeline *)Sys_Alloc(1, sizeof(*p), MH_RenderBackend);
	if (!p)
		return nullptr;

	CD3DX12_STATE_OBJECT_DESC soDesc{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };
	CD3DX12_DXIL_LIBRARY_SUBOBJECT *lib = soDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

/*	struct Shader *s = (struct Shader *)Re.GetShader(Rt_HashStringW(L"PathTracer"));
	lib->SetDXILLibrary(&s->lib);*/

	/*CD3DX12_HIT_GROUP_SUBOBJECT *hitGroup = soDesc.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
	hitGroup->SetClosestHitShaderImport(L"ClosestHit");
	hitGroup->SetHitGroupExport(L"HitGroup");
	hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

	CD3DX12_HIT_GROUP_SUBOBJECT *shadowHitGroup = soDesc.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
	shadowHitGroup->SetClosestHitShaderImport(L"ShadowHit");
	shadowHitGroup->SetHitGroupExport(L"ShadowHitGroup");
	shadowHitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

	CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT *shaderConfig = soDesc.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
	shaderConfig->Config(4 * sizeof(float), 2 * sizeof(float));

	CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT *lrs = soDesc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
	lrs->SetRootSignature(_rayGenSignature);

	CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT *assoc = soDesc.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
	assoc->SetSubobjectToAssociate(*lrs);

	CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT *grs = soDesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	grs->SetRootSignature(_globalRootSignature);

	CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT *dummyLrs = soDesc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
	dummyLrs->SetRootSignature(_hitSignature);

	CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT *pipelineConfig = soDesc.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
	pipelineConfig->Config(1);

	D3DCHK(Re_Device.dev->CreateStateObject(desc, IID_PPV_ARGS(&_pso)));
	_pso->QueryInterface(IID_PPV_ARGS(&_psoProps));

	_pso->SetName(L"Raytracing PSO");*/

/*	{ // Shader binding table
		for (int i = 0; i < RE_NUM_BUFFERS; ++i) {
			D3D12_InitSBT(&_sbt[i]);

			D3D12_GPU_DESCRIPTOR_HANDLE handle = _descHeap[i]->GetGPUDescriptorHandleForHeapStart();

			D3D12_AddShader(&_sbt[i], SET_RayGen, L"RayGen", (void *)handle.ptr);
			D3D12_AddShader(&_sbt[i], SET_Miss, L"Miss", NULL);
			D3D12_AddShader(&_sbt[i], SET_HitGroup, L"HitGroup", NULL);
			D3D12_AddShader(&_sbt[i], SET_Miss, L"ShadowMiss", NULL);
			D3D12_AddShader(&_sbt[i], SET_HitGroup, L"ShadowHitGroup", NULL);

			if (!D3D12_BuildSBT(&_sbt[i], _psoProps))
				return false;
		}
	}*/

	/*VkPipelineDynamicStateCreateInfo dyn =
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
					break;*
		}

		//info.stage.module = desc->stageInfo->stages[i].module;
		break;
	}

	if (vkCreateRayTracingPipelinesKHR(Re_device->dev, VK_NULL_HANDLE, Vkd_pipelineCache, 1, &info, Vkd_allocCb, &p->pipeline) != VK_SUCCESS) {
		Sys_Free(p);
		return NULL;
	}*/

	return p;
}

void
Re_LoadPipelineCache(void)
{
	/*int64_t dataSize = 0;
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

	Sys_Free(data);*/
}

void
Re_SavePipelineCache(void)
{
	/*size_t dataSize = 0;
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

	Sys_Free(data);*/
}

void
Re_BkDestroyPipeline(struct NePipeline *pipeline)
{
	pipeline->ps->Release();
	pipeline->rs->Release();
	Sys_Free(pipeline);
}

static inline ID3D12RootSignature *
CreateRootSignature(uint32_t size, D3D12_SHADER_VISIBILITY vis, bool inputAttachments)
{
	/*
	 * 		CD3DX12_DESCRIPTOR_RANGE ranges[1]{};
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 10, 0, 0);

		CD3DX12_ROOT_PARAMETER params[3]{};
		params[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		params[1].InitAsConstants(1, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
		params[2].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

		D3D12_STATIC_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.MipLODBias = 0;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		samplerDesc.MinLOD = 0.f;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc.ShaderRegister = 0;
		samplerDesc.RegisterSpace = 0;
		samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	*/

	/*
	 * VkPipelineLayoutCreateInfo info =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = inputAttachments ? 2 : 1,
			.pSetLayouts = layouts,
			.pushConstantRangeCount = size > 0,
			.pPushConstantRanges = &range
		};
	 */

	CD3DX12_DESCRIPTOR_RANGE ranges[3]{};
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, UINT16_MAX, 0, 1);

	CD3DX12_ROOT_PARAMETER params[3]{};
	params[0].InitAsDescriptorTable(1, &ranges[0]);			// Textures
	params[1].InitAsShaderResourceView(1, 0, vis);			// Materials

	if (size)
		params[2].InitAsConstantBufferView(0, 0, vis);

	CD3DX12_ROOT_SIGNATURE_DESC rsDesc{};
	rsDesc.Init(size ? _countof(params) : _countof(params) - 1, params, D3D12Bk_staticSamplerCount, D3D12Bk_staticSamplers);

	ID3DBlob *sig, *err;
	if (HRESULT hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &sig, &err); FAILED(hr)) {
		Sys_LogEntry(D3D12PMOD, LOG_CRITICAL, "Failed to serialize global root signature: %s [0x%x]", err->GetBufferPointer(), hr);
		err->Release();
		return nullptr;
	}

	ID3D12RootSignature *rs;
	if (HRESULT hr = Re_device->dev->CreateRootSignature(0, sig->GetBufferPointer(), sig->GetBufferSize(), IID_PPV_ARGS(&rs)); FAILED(hr)) {
		Sys_LogEntry(D3D12PMOD, LOG_CRITICAL, "Failed to create global root signature: %s [0x%x]", err->GetBufferPointer(), hr);
		err->Release();
		return nullptr;
	}

	sig->Release();
	return rs;
}

/* NekoEngine
 *
 * D3D12Pipeline.cxx
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
