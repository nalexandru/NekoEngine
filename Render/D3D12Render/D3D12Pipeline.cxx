#include <System/Log.h>
#include <Render/Render.h>
#include <Render/Shader.h>
#include <Render/Device.h>
#include <Runtime/Runtime.h>

#include "D3D12Render.h"

#define PSOMOD	L"Pipeline"

static ID3D12RootSignature *_dummyGlobal, *_dummyLocal;

bool
D3D12_InitPipelines(void)
{
	HRESULT hr;
	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	ID3DBlob *sigBlob, *errBlob;

	hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errBlob);
	if (FAILED(hr)) {
		Sys_LogEntry(PSOMOD, LOG_CRITICAL, L"Failed to serialize dummy global root signature: %S", errBlob->GetBufferPointer());
		goto error;
	}

	hr = Re_Device.dev->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&_dummyGlobal));
	if (FAILED(hr)) {
		Sys_LogEntry(PSOMOD, LOG_CRITICAL, L"Failed to create dummy global root signature: %S", errBlob->GetBufferPointer());
		goto error;
	}

	sigBlob->Release(); sigBlob = NULL;

	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
	hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errBlob);
	if (FAILED(hr)) {
		Sys_LogEntry(PSOMOD, LOG_CRITICAL, L"Failed to serialize dummy local root signature: %S", errBlob->GetBufferPointer());
		goto error;
	}

	hr = Re_Device.dev->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&_dummyGlobal));
	if (FAILED(hr)) {
		Sys_LogEntry(PSOMOD, LOG_CRITICAL, L"Failed to create dummy local root signature: %S", errBlob->GetBufferPointer());
		goto error;
	}

	sigBlob->Release(); sigBlob = NULL;

error:
	if (sigBlob)
		sigBlob->Release();

	if (errBlob)
		errBlob->Release();

	return false;
}

struct Pipeline *
Re_GetGraphicsPipeline(struct Shader *shader, uint64_t flags)
{
	/*D3D12_GRAPHICS_PIPELINE_STATE_DESC psd = { 0 };
	
	for (uint32_t i = 0; i < shader->modules.count; ++i) {
		struct ShaderModule *mod = (struct ShaderModule *)Rt_ArrayGet(&shader->modules, i);
		D3D12_SHADER_BYTECODE *dst;
		switch (mod->type) {
		case ST_Vertex: dst = &psd.VS; break;
		case ST_Pixel: dst = &psd.PS; break;
		case ST_Domain: dst = &psd.DS; break;
		case ST_Hull: dst = &psd.HS; break;
		case ST_Geometry: dst = &psd.GS; break;
		default: continue;
		}
	
		dst->BytecodeLength = mod->length;
		dst->pShaderBytecode = mod->bytecode;
	}

	psd.InputLayout = shader->inputLayout;

	switch (flags & RE_TOPOLOGY_BITS) {
	case RE_TOPOLOGY_TRIANGLE: psd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; break;
	case RE_TOPOLOGY_LINE: psd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE; break;
	case RE_TOPOLOGY_PATCH: psd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH; break;
	case RE_TOPOLOGY_POINT: psd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT; break;
	}

	psd.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	switch (flags & RE_FILL_BITS) {
	case RE_FILL_SOLID: psd.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	case RE_FILL_WIREFRAME: psd.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	}

	switch (flags & RE_CULL_BITS) {
	case RE_CULL_BACK: psd.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	case RE_CULL_NONE: psd.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	case RE_CULL_FRONT: psd.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	}

	psd.RasterizerState.FrontCounterClockwise = flags & RE_CCW;

    /*INT DepthBias;
    FLOAT DepthBiasClamp;
    FLOAT SlopeScaledDepthBias;
    BOOL DepthClipEnable;
    BOOL MultisampleEnable;
    BOOL AntialiasedLineEnable;
    UINT ForcedSampleCount;
    D3D12_CONSERVATIVE_RASTERIZATION_MODE ConservativeRaster;*

	psd.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psd.DepthStencilState.DepthEnable = flags & RE_DEPTH_TEST;
	psd.DepthStencilState.DepthWriteMask = flags & RE_DEPTH_WRITE ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;

	psd.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psd.NumRenderTargets = 1;

	ID3D12PipelineState *ps;
	Re_Device.dev->CreateGraphicsPipelineState(&psd, IID_PPV_ARGS(&ps));*/

	return NULL;
}

/*ID3D12StateObject *
D3D12_RaytracingPipeline(struct ShaderLibrary *library, UINT maxRecursionDepth, UINT maxPayloadSize, UINT maxAttributeSize)
{
	size_t subObjectCount = 6, i = 0;
	D3D12_STATE_SUBOBJECT *subObjects = (D3D12_STATE_SUBOBJECT *)calloc(subObjectCount, sizeof(*subObjects));

	subObjects[i].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
	subObjects[i++].pDesc = &library->desc;

	// hit group
	D3D12_HIT_GROUP_DESC hitDesc = {};
	hitDesc.AnyHitShaderImport = NULL;
	hitDesc.ClosestHitShaderImport = L"ClosestHit";
	hitDesc.IntersectionShaderImport = NULL;
	hitDesc.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
	hitDesc.HitGroupExport = L"HitGroup";

	subObjects[i].Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
	subObjects[i++].pDesc = &hitDesc;

	// shader config
	D3D12_RAYTRACING_SHADER_CONFIG rtShaderConfig = {};
	rtShaderConfig.MaxPayloadSizeInBytes = maxPayloadSize;
	rtShaderConfig.MaxAttributeSizeInBytes = maxAttributeSize;
	
	subObjects[i].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
	subObjects[i++].pDesc = &rtShaderConfig;

	// exports
	D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION exportAssoc = {};
	exportAssoc.NumExports = library->desc.NumExports;
	exportAssoc.pExports = library->symbols;
	exportAssoc.pSubobjectToAssociate = &subObjects[i - 1];

	subObjects[i].Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
	subObjects[i++].pDesc = &exportAssoc;

	// root signatures

	// there must be an empty local & global root signature
	subObjects[i].Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
	subObjects[i++].pDesc = &_dummyGlobal;

	subObjects[i].Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
	subObjects[i++].pDesc = &_dummyLocal;

	// pipeline config
	D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = { maxRecursionDepth };
	subObjects[i].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
	subObjects[i++].pDesc = &pipelineConfig;

	D3D12_STATE_OBJECT_DESC pipelineDesc = {};
	pipelineDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
	pipelineDesc.NumSubobjects = (UINT)subObjectCount;
	pipelineDesc.pSubobjects = subObjects;
	
	ID3D12StateObject *pso = NULL;
	HRESULT hr = Re_Device.dev->CreateStateObject(&pipelineDesc, IID_PPV_ARGS(&pso));
	if (FAILED(hr))
		Sys_LogEntry(PSOMOD, LOG_CRITICAL, L"Failed to create pipeline: 0x%x", hr);

	return pso;
}*/

void
Re_LoadPipelineCache(void)
{

}

void
Re_SavePipelineCache(void)
{

}
