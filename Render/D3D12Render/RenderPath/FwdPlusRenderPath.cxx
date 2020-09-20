#include <System/Log.h>
#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Render/Device.h>

#include "FwdPlusRenderPath.h"

#define FWRPMOD	L"FwdPlusRenderPath"

bool
FwdPlusRenderPath::Init()
{
	HRESULT hr;

	{ // Descriptor heap
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = RE_NUM_BUFFERS;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

		hr = Re_Device.dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_rtvHeap));
		if (FAILED(hr)) {
			Sys_LogEntry(FWRPMOD, LOG_CRITICAL, L"Failed to create descriptor heap: 0x%x", hr);
			return false;
		}

		_rtvHeap->SetName(L"RTV Descriptor Heap");

		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		hr = Re_Device.dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_dsvHeap));
		if (FAILED(hr)) {
			Sys_LogEntry(FWRPMOD, LOG_CRITICAL, L"Failed to create descriptor heap: 0x%x", hr);
			return false;
		}

		_dsvHeap->SetName(L"DSV Descriptor Heap");

		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		/*for (int i = 0; i < RE_NUM_BUFFERS; ++i) {
			hr = Re_Device.dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_rtvHeap));
			if (FAILED(hr)) {
				Sys_LogEntry(FWRPMOD, LOG_CRITICAL, L"Failed to create descriptor heap: 0x%x", hr);
				return false;
			}

			_rtvHeap->SetName(L"RTV Descriptor Heap");
		}*/
	}

	{ // Global root signature
		CD3DX12_DESCRIPTOR_RANGE ranges[2]{};
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4096, 0, 1);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

		CD3DX12_ROOT_PARAMETER params[1]{};
		params[0].InitAsDescriptorTable(1, &ranges[0]);
	//	params[1].InitAsDescriptorTable(1, &ranges[1]);

		D3D12_STATIC_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
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
		samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		CD3DX12_ROOT_SIGNATURE_DESC rsDesc;
		rsDesc.Init(_countof(params), params, 1, &samplerDesc);

		ID3DBlob *sigBlob;
		ID3DBlob *errBlob;
		hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &sigBlob, &errBlob);
		if (FAILED(hr)) {
			Sys_LogEntry(FWRPMOD, LOG_CRITICAL, L"Failed to serialize global root signature: %S [0x%x]", errBlob->GetBufferPointer(), hr);
			errBlob->Release();
			return false;
		}

		hr = Re_Device.dev->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&_globalRootSignature));
		if (FAILED(hr)) {
			Sys_LogEntry(FWRPMOD, LOG_CRITICAL, L"Failed to create global root signature: %S [0x%x]", errBlob->GetBufferPointer(), hr);
			errBlob->Release();
			return false;
		}
		_globalRootSignature->SetName(L"Ray Tracing Global Root Signature");

		sigBlob->Release();
	}

	{ // Pipeline state
	/*	CD3DX12_STATE_OBJECT_DESC desc{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

		_library = D3D12_LoadShaderLibrary("/Shaders/HLSL6/PathTracer.hlsl");
		CD3DX12_DXIL_LIBRARY_SUBOBJECT *lib = desc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

		CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT *pipelineConfig = desc.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
		pipelineConfig->Config(1);

		Re_Device.dev->CreateStateObject(desc, IID_PPV_ARGS(&_pso));
		_pso->QueryInterface(IID_PPV_ARGS(&_psoProps));

		_pso->SetName(L"Raytracing PSO");*/
	}

	_srvIncrement = Re_Device.dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	_rtvIncrement = Re_Device.dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	_dsvIncrement = Re_Device.dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	return true;
}

void
FwdPlusRenderPath::RenderScene(const struct Scene *s, ID3D12Resource *output, D3D12_RESOURCE_STATES outputState)
{
	const struct SceneRenderData *srd = (const struct SceneRenderData *)&s->renderDataStart;
	ID3D12GraphicsCommandList *cmdList = Re_MainThreadWorker.cmdList;
	ID3D12Resource *color = NULL, *depth = NULL;

	{ // Outputs
		D3D12_RESOURCE_DESC desc = {};
		D3D12_CLEAR_VALUE clear{};

		desc.DepthOrArraySize = 1;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		desc.Width = *E_ScreenWidth;
		desc.Height = *E_ScreenHeight;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;

		clear.Format = desc.Format;
		clear.Color[0] = .8f; clear.Color[1] = .2f; clear.Color[2] = .5f; clear.Color[3] = 1.f;

		color = D3D12_CreateTransientResource(&desc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clear);
		color->SetName(L"Color Buffer");

		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		clear.Format = desc.Format;
		clear.DepthStencil.Depth = 0.f; clear.DepthStencil.Stencil = 0;

		depth = D3D12_CreateTransientResource(&desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear);
		depth->SetName(L"Depth Buffer");
	}

	{ // Descriptor Heap
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += (uintptr_t)Re_Device.frame * _rtvIncrement;

		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
		dsvHandle.ptr += (uintptr_t)Re_Device.frame * _dsvIncrement;

		Re_Device.dev->CreateRenderTargetView(color, NULL, rtvHandle);
		Re_Device.dev->CreateDepthStencilView(depth, NULL, dsvHandle);

		cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

		const float clearColor[] = { .8f, .2f, .5f, 1.f };
		cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, NULL);
		cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 0.f, 0, 0, NULL);

	/*	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.RaytracingAccelerationStructure.Location = srd->asBuffer->GetGPUVirtualAddress();
	
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = srd->materialBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = (UINT)srd->materialBufferSize;

		D3D12_CPU_DESCRIPTOR_HANDLE handle = _descHeap[Re_Device.frame]->GetCPUDescriptorHandleForHeapStart();
	
		Re_Device.dev->CreateUnorderedAccessView(rtOutput, NULL, &uavDesc, handle);
		handle.ptr += _descIncrement;

		Re_Device.dev->CreateShaderResourceView(NULL, &srvDesc, handle);
		handle.ptr += _descIncrement;

		Re_Device.dev->CreateConstantBufferView(&cbvDesc, handle);
		handle.ptr += _descIncrement;

		D3D12_UpdateTextureHeap(handle);*/
	}


	/*ID3D12DescriptorHeap *heaps[] = { _descHeap[Re_Device.frame] };
	cmdList->SetDescriptorHeaps(_countof(heaps), heaps);

	D3D12_GPU_DESCRIPTOR_HANDLE table = _descHeap[Re_Device.frame]->GetGPUDescriptorHandleForHeapStart();
	table.ptr += 3ULL * _descIncrement;

	cmdList->SetComputeRootSignature(_globalRootSignature);
	cmdList->SetComputeRootDescriptorTable(0, table);*/

	D3D12_RESOURCE_BARRIER barriers[] =
	{
		//CD3DX12_RESOURCE_BARRIER::Transition(srd->materialBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
		CD3DX12_RESOURCE_BARRIER::Transition(color, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE),
		CD3DX12_RESOURCE_BARRIER::Transition(output, outputState, D3D12_RESOURCE_STATE_COPY_DEST)
	};
	cmdList->ResourceBarrier(_countof(barriers), barriers);

	cmdList->CopyResource(output, color);

	D3D12_RESOURCE_BARRIER endBarriers[] =
	{
//		CD3DX12_RESOURCE_BARRIER::Transition(srd->materialBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST),
		CD3DX12_RESOURCE_BARRIER::Transition(output, D3D12_RESOURCE_STATE_COPY_DEST, outputState)
	};
	cmdList->ResourceBarrier(_countof(endBarriers), endBarriers);
}

void
FwdPlusRenderPath::Term()
{
//	_pso->Release();
//	_psoProps->Release();

	_globalRootSignature->Release();

	_rtvHeap->Release();
	_dsvHeap->Release();

	for (uint32_t i = 0; i < RE_NUM_BUFFERS; ++i) {
		//_descHeap[i]->Release();
	}
}
