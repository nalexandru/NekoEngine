#include <stdio.h>

#include <UI/UI.h>
#include <Math/Math.h>
#include <Render/Render.h>
#include <Render/Device.h>
#include <Render/Texture.h>
#include <System/Memory.h>
#include <Engine/ECSystem.h>
#include <Engine/Resource.h>

#include "Fence.h"
#include "D3D12Render.h"

#define UIRMOD	L"UIRender"

struct UIRenderData
{
	struct mat4 RD_Projection;
};

struct DrawUIContextArgs
{
	ID3D12GraphicsCommandList *cmdList;
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
};

static ID3D12RootSignature *_rootSignature;
static ID3D12PipelineState *_pipelineState;
static ID3D12DescriptorHeap *_srvHeap, *_rtvHeap;

static UINT _rtvIncrement, _srvIncrement;

bool
D3D12_InitUI(void)
{
	D3D12_DESCRIPTOR_HEAP_DESC dhd{};
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	dhd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dhd.NumDescriptors = RE_NUM_BUFFERS;

	if (FAILED(Re_Device.dev->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&_rtvHeap))))
		return false;
	_rtvHeap->SetName(L"UI RTV Descriptor Heap");
	_rtvIncrement = Re_Device.dev->GetDescriptorHandleIncrementSize(dhd.Type);

	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	dhd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	dhd.NumDescriptors = 10 * RE_NUM_BUFFERS;

	if (FAILED(Re_Device.dev->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&_srvHeap))))
		return false;
	_srvHeap->SetName(L"UI SRV Descriptor Heap");
	_srvIncrement = Re_Device.dev->GetDescriptorHandleIncrementSize(dhd.Type);

	{ // Root signature
		HRESULT hr;

		CD3DX12_DESCRIPTOR_RANGE ranges[1]{};
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

		CD3DX12_ROOT_SIGNATURE_DESC rsDesc;
		rsDesc.Init(_countof(params), params, 1, &samplerDesc);
		rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		ID3DBlob *sigBlob;
		ID3DBlob *errBlob;
		hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &sigBlob, &errBlob);
		if (FAILED(hr)) {
			Sys_LogEntry(UIRMOD, LOG_CRITICAL, L"Failed to serialize global root signature: %S [0x%x]", errBlob->GetBufferPointer(), hr);
			errBlob->Release();
			return false;
		}

		hr = Re_Device.dev->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature));
		if (FAILED(hr)) {
			Sys_LogEntry(UIRMOD, LOG_CRITICAL, L"Failed to create global root signature: %S [0x%x]", errBlob->GetBufferPointer(), hr);
			errBlob->Release();
			return false;
		}
		_rootSignature->SetName(L"UI Root Signature");

		sigBlob->Release();
	}

	{ // Pipeline
		D3D12_INPUT_ELEMENT_DESC inputDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		struct Shader *shader = (struct Shader *)Re.GetShader(Rt_HashStringW(L"UI"));

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psDesc = {};
		psDesc.InputLayout = { inputDesc, _countof(inputDesc) };
		psDesc.pRootSignature = _rootSignature;
		psDesc.VS = shader->VS;
		psDesc.PS = shader->PS;
		psDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psDesc.RasterizerState.FrontCounterClockwise = TRUE;
		psDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psDesc.DepthStencilState.DepthEnable = FALSE;
		psDesc.DepthStencilState.StencilEnable = FALSE;
		psDesc.SampleMask = UINT_MAX;
		psDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psDesc.NumRenderTargets = 1;
		psDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psDesc.SampleDesc.Count = 1;

		psDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
		psDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		psDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		psDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

		HRESULT hr = Re_Device.dev->CreateGraphicsPipelineState(&psDesc, IID_PPV_ARGS(&_pipelineState));
		if (FAILED(hr)) {
			return false;
		}
	}

	return true;
}

void
D3D12_RenderUI(struct Scene *s, ID3D12Resource *output, D3D12_RESOURCE_STATES startState, D3D12_RESOURCE_STATES endState)
{
	ID3D12GraphicsCommandList *cmdList = Re_MainThreadWorker.cmdList;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += (uintptr_t)Re_Device.frame * _rtvIncrement;

	Re_Device.dev->CreateRenderTargetView(output, NULL, rtvHandle);
	cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, NULL);

	struct UIRenderData urd{};
	m4_copy(&urd.RD_Projection, &UI_Projection);

	D3D12_RESOURCE_DESC rd = CD3DX12_RESOURCE_DESC::Buffer(ROUND_UP(sizeof(urd), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT));
	ID3D12Resource *cb = D3D12_CreateTransientResource(&rd, D3D12_RESOURCE_STATE_COPY_DEST, NULL);
	cb->SetName(L"UI Constant Buffer");

	D3D12_StageUpload(cb, sizeof(urd), &urd);

	cmdList->SetGraphicsRootSignature(_rootSignature);
	cmdList->SetGraphicsRootConstantBufferView(0, cb->GetGPUVirtualAddress());

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->SetPipelineState(_pipelineState);

	D3D12_VIEWPORT vp{ 0, 0, (float)*E_ScreenWidth, (float)*E_ScreenHeight, 0.f, 1.f };
	cmdList->RSSetViewports(1, &vp);

	D3D12_RECT scissor{ 0, 0, (LONG)*E_ScreenWidth, (LONG)*E_ScreenHeight };
	cmdList->RSSetScissorRects(1, &scissor);

	D3D12_RESOURCE_BARRIER barriers[] =
	{
		CD3DX12_RESOURCE_BARRIER::Transition(cb, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
		CD3DX12_RESOURCE_BARRIER::Transition(output, startState, D3D12_RESOURCE_STATE_RENDER_TARGET)
	};
	cmdList->ResourceBarrier(startState != D3D12_RESOURCE_STATE_RENDER_TARGET ? 2 : 1, barriers);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = _srvHeap->GetCPUDescriptorHandleForHeapStart();
	cpuHandle.ptr += (uintptr_t)Re_Device.frame * 10 * _srvIncrement;

	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = _srvHeap->GetGPUDescriptorHandleForHeapStart();
	gpuHandle.ptr += (uintptr_t)Re_Device.frame * 10 * _srvIncrement;

	cmdList->SetDescriptorHeaps(1, &_srvHeap);
	cmdList->SetGraphicsRootDescriptorTable(2, gpuHandle);

	struct DrawUIContextArgs drawArgs = { cmdList, cpuHandle };
	E_ExecuteSystemS(s, DRAW_UI_CONTEXT, &drawArgs);

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(output, D3D12_RESOURCE_STATE_RENDER_TARGET, endState);
	cmdList->ResourceBarrier(1, &barrier);

	SignalFence(&Re_UploadFence, Re_Device.transferQueue);
}

void
D3D12_TermUI(void)
{
	_pipelineState->Release();
	_rootSignature->Release();
	_rtvHeap->Release();
	_srvHeap->Release();
}

void
D3D12_DrawUIContext(void **comp, struct DrawUIContextArgs *args)
{
	struct UIContext *ctx = (struct UIContext *)comp[0];

	const UINT64 vtxSize = sizeof(UIVertex) * ctx->vertices.count;
	const UINT64 idxSize = sizeof(uint16_t) * ctx->indices.count;

	CD3DX12_RESOURCE_DESC rd = CD3DX12_RESOURCE_DESC::Buffer(vtxSize);
	ID3D12Resource *vtxBuffer = D3D12_CreateTransientResource(&rd, D3D12_RESOURCE_STATE_COPY_DEST, NULL);
	vtxBuffer->SetName(L"UI Vertex Buffer");

	rd = CD3DX12_RESOURCE_DESC::Buffer(idxSize);
	ID3D12Resource *idxBuffer = D3D12_CreateTransientResource(&rd, D3D12_RESOURCE_STATE_COPY_DEST, NULL);
	idxBuffer->SetName(L"UI Index Buffer");

	D3D12_StageUpload(vtxBuffer, vtxSize, ctx->vertices.data);
	D3D12_StageUpload(idxBuffer, idxSize, ctx->indices.data);
	
	D3D12_RESOURCE_BARRIER barriers[] =
	{
		CD3DX12_RESOURCE_BARRIER::Transition(vtxBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
		CD3DX12_RESOURCE_BARRIER::Transition(idxBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER)
	};
	args->cmdList->ResourceBarrier(_countof(barriers), barriers);

	D3D12_VERTEX_BUFFER_VIEW vbView{ vtxBuffer->GetGPUVirtualAddress(), (UINT)vtxSize, sizeof(UIVertex) };
	args->cmdList->IASetVertexBuffers(0, 1, &vbView);

	D3D12_INDEX_BUFFER_VIEW ibView{ idxBuffer->GetGPUVirtualAddress(), (UINT)idxSize, DXGI_FORMAT_R16_UINT };
	args->cmdList->IASetIndexBuffer(&ibView);

	for (size_t i = 0; i < ctx->draws.count; ++i) {
		const struct UIDrawCall *drawCall = (struct UIDrawCall *)Rt_ArrayGet(&ctx->draws, i);

		struct TextureRenderData *trd = (struct TextureRenderData *)&((struct Texture *)E_ResourcePtr(drawCall->texture))->renderDataStart;
		D3D12_CopyTextureDescriptor(trd->id, args->srvHandle);
		args->srvHandle.ptr += _srvIncrement;

		args->cmdList->SetGraphicsRoot32BitConstant(1, (UINT)i, 0);
		args->cmdList->DrawIndexedInstanced(drawCall->idxCount, 1, drawCall->idxOffset, 0, 0);
	}
}

