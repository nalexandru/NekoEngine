#include <stdio.h>

#include <Render/Render.h>
#include <Render/Device.h>
#include <System/Memory.h>

#include "D3D12Render.h"

#define TEXT_BUFF	1024

struct FontVertex
{
	float x, y, u, v;
	float r, g, b, a;
};

static ID3D12Resource *_texture;
static ID3D12RootSignature *_rootSignature;
static ID3D12PipelineState *_pipelineState;
static ID3D12DescriptorHeap *_heap, *_rtvHeap;

static UINT _rtvIncrement;

static Array _vertices;

void
Re_DrawText(uint16_t x, uint16_t y, const wchar_t *fmt, ...)
{
	wchar_t *buff = (wchar_t *)Sys_Alloc(sizeof(wchar_t), 1024, MH_Transient);
	memset(buff, 0x0, TEXT_BUFF);

	va_list args;
	va_start(args, fmt);
	vswprintf(buff, TEXT_BUFF, fmt, args);
	va_end(args);

	//
}

bool
D3D12_InitTextRender(void)
{
	D3D12_DESCRIPTOR_HEAP_DESC dhd{};
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	dhd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	dhd.NumDescriptors = 1;

	if (FAILED(Re_Device.dev->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&_heap))))
		return false;
	_heap->SetName(L"Text Descriptor Heap");

	dhd.NumDescriptors = RE_NUM_BUFFERS;
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	dhd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	if (FAILED(Re_Device.dev->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&_rtvHeap))))
		return false;
	_rtvHeap->SetName(L"Text RTV Descriptor Heap");

	_rtvIncrement = Re_Device.dev->GetDescriptorHandleIncrementSize(dhd.Type);

	{ // Root signature
		HRESULT hr;

		CD3DX12_ROOT_PARAMETER params[1]{};
		params[0].InitAsShaderResourceView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

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

		ID3DBlob *sigBlob;
		ID3DBlob *errBlob;
		hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &sigBlob, &errBlob);
		if (FAILED(hr)) {
		//	Sys_LogEntry(RTRPMOD, LOG_CRITICAL, L"Failed to serialize global root signature: %S [0x%x]", errBlob->GetBufferPointer(), hr);
			errBlob->Release();
			return false;
		}

		hr = Re_Device.dev->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature));
		if (FAILED(hr)) {
		//	Sys_LogEntry(RTRPMOD, LOG_CRITICAL, L"Failed to create global root signature: %S [0x%x]", errBlob->GetBufferPointer(), hr);
			errBlob->Release();
			return false;
		}
		_rootSignature->SetName(L"Text Root Signature");

		sigBlob->Release();
	}

	{ // Pipeline
		D3D12_INPUT_ELEMENT_DESC inputDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		struct Shader *shader = (struct Shader *)Re_GetShader(Rt_HashStringW(L"Text"));

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psDesc = {};
		psDesc.InputLayout = { inputDesc, _countof(inputDesc) };
		psDesc.pRootSignature = _rootSignature;
		psDesc.VS = shader->VS;
		psDesc.PS = shader->PS;
		psDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psDesc.DepthStencilState.DepthEnable = FALSE;
		psDesc.DepthStencilState.StencilEnable = FALSE;
		psDesc.SampleMask = UINT_MAX;
		psDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psDesc.NumRenderTargets = 1;
		psDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psDesc.SampleDesc.Count = 1;
	}

	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = 1;
		Re_Device.dev->CreateShaderResourceView(_texture, &srvDesc, _heap->GetCPUDescriptorHandleForHeapStart());
	}

	return true;
}

void
D3D12_RenderText(ID3D12Resource *output)
{
	ID3D12GraphicsCommandList *cmdList = Re_MainThreadWorker.cmdList;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += (uintptr_t)Re_Device.frame * _rtvIncrement;

	Re_Device.dev->CreateRenderTargetView(output, NULL, rtvHandle);
	cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, NULL);

	cmdList->SetGraphicsRootSignature(_rootSignature);
	cmdList->SetGraphicsRootDescriptorTable(0, _heap->GetGPUDescriptorHandleForHeapStart());

	cmdList->SetPipelineState(_pipelineState);

	cmdList->DrawInstanced((UINT)_vertices.count, 1, 0, 0);

	Rt_ClearArray(&_vertices, false);
}

void
D3D12_TermTextRender(void)
{
	_heap->Release();
}
