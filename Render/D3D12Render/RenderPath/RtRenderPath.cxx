#include <System/Log.h>
#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Render/Device.h>

#include "../D3D12Material.h"
#include "RtRenderPath.h"

#define RTRPMOD	L"RtRenderPath"

bool
RtRenderPath::Init()
{
	HRESULT hr;

	if (!Re_Features.rayTracing)
		return false;

	{ // Descriptor heap
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = 2048;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		for (int i = 0; i < RE_NUM_BUFFERS; ++i) {
			hr = Re_Device.dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_descHeap[i]));
			if (FAILED(hr)) {
				Sys_LogEntry(RTRPMOD, LOG_CRITICAL, L"Failed to create descriptor heap: 0x%x", hr);
				return false;
			}

			_descHeap[i]->SetName(L"Raytracing Descriptor Heap");
		}
	}

	{ // Ray gen signature
		D3D12_DESCRIPTOR_RANGE ranges[] = 
		{
			{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, 0 },
			{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 1 }
		};

		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param.DescriptorTable.NumDescriptorRanges = _countof(ranges);
		param.DescriptorTable.pDescriptorRanges = ranges;

		D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
		rsDesc.NumParameters = 1;
		rsDesc.pParameters = &param;
		rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

		ID3DBlob *sigBlob;
		ID3DBlob *errBlob;
		hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &sigBlob, &errBlob);
		if (FAILED(hr)) {
			Sys_LogEntry(RTRPMOD, LOG_CRITICAL, L"Failed to serialize ray gen root signature: 0x%x", hr);
			errBlob->Release();
			return false;
		}

		hr = Re_Device.dev->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&_rayGenSignature));
		if (FAILED(hr)) {
			Sys_LogEntry(RTRPMOD, LOG_CRITICAL, L"Failed to create ray gen root signature: 0x%x", hr);
			errBlob->Release();
			return false;
		}
		_rayGenSignature->SetName(L"RayGen Local Root Signature");

		sigBlob->Release();
	}

	{ // Global root signature
		CD3DX12_DESCRIPTOR_RANGE ranges[3]{};
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4096, 0, 1);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4096, 0, 2);
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4096, 0, 3);

		CD3DX12_ROOT_PARAMETER params[5]{};
		params[0].InitAsDescriptorTable(1, &ranges[0]);
		params[1].InitAsDescriptorTable(1, &ranges[1]);
		params[2].InitAsDescriptorTable(1, &ranges[2]);
		params[3].InitAsShaderResourceView(1, 0);
		params[4].InitAsConstantBufferView(0, 0);

		D3D12_STATIC_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
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
			Sys_LogEntry(RTRPMOD, LOG_CRITICAL, L"Failed to serialize global root signature: %S [0x%x]", errBlob->GetBufferPointer(), hr);
			errBlob->Release();
			return false;
		}

		hr = Re_Device.dev->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&_globalRootSignature));
		if (FAILED(hr)) {
			Sys_LogEntry(RTRPMOD, LOG_CRITICAL, L"Failed to create global root signature: %S [0x%x]", errBlob->GetBufferPointer(), hr);
			errBlob->Release();
			return false;
		}
		_globalRootSignature->SetName(L"Ray Tracing Global Root Signature");

		sigBlob->Release();
	}

	{ // Hit & Miss signature
		D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
		rsDesc.NumParameters = 0;
		rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

		ID3DBlob *sigBlob;
		ID3DBlob *errBlob;
		hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &sigBlob, &errBlob);
		if (FAILED(hr)) {
			Sys_LogEntry(RTRPMOD, LOG_CRITICAL, L"Failed to serialize ray gen root signature: %S [0x%x]", errBlob->GetBufferPointer(), hr);
			errBlob->Release();
			return false;
		}

		hr = Re_Device.dev->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&_hitSignature));
		if (FAILED(hr)) {
			Sys_LogEntry(RTRPMOD, LOG_CRITICAL, L"Failed to create ray gen root signature: %S [0x%x]", errBlob->GetBufferPointer(), hr);
			errBlob->Release();
			return false;
		}
		_hitSignature->SetName(L"Hit Local Root Signature");

		hr = Re_Device.dev->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&_missSignature));
		if (FAILED(hr)) {
			Sys_LogEntry(RTRPMOD, LOG_CRITICAL, L"Failed to create ray gen root signature: %S [0x%x]", errBlob->GetBufferPointer(), hr);
			errBlob->Release();
			return false;
		}
		_missSignature->SetName(L"Miss Local Root Signature");

		sigBlob->Release();
	}

	{ // Pipeline state
		CD3DX12_STATE_OBJECT_DESC desc{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

		CD3DX12_DXIL_LIBRARY_SUBOBJECT *lib = desc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

		struct Shader *s = (struct Shader *)Re_GetShader(Rt_HashStringW(L"PathTracer"));
		lib->SetDXILLibrary(&s->lib);
		lib->DefineExport(L"RayGen");
		lib->DefineExport(L"ClosestHit");
		lib->DefineExport(L"Miss");

		CD3DX12_HIT_GROUP_SUBOBJECT *hitGroup = desc.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		hitGroup->SetClosestHitShaderImport(L"ClosestHit");
		hitGroup->SetHitGroupExport(L"HitGroup");
		hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

		CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT *shaderConfig = desc.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
		shaderConfig->Config(4 * sizeof(float), 2 * sizeof(float));

		CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT *lrs = desc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		lrs->SetRootSignature(_rayGenSignature);
		
		CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT *assoc = desc.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		assoc->SetSubobjectToAssociate(*lrs);
		assoc->AddExport(L"RayGen");

		CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT *grs = desc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		grs->SetRootSignature(_globalRootSignature);

		CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT *dummyLrs = desc.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		dummyLrs->SetRootSignature(_hitSignature);

		CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT *pipelineConfig = desc.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
		pipelineConfig->Config(1);

		D3DCHK(Re_Device.dev->CreateStateObject(desc, IID_PPV_ARGS(&_pso)));
		_pso->QueryInterface(IID_PPV_ARGS(&_psoProps));

		_pso->SetName(L"Raytracing PSO");
	}

	{ // Shader binding table
		for (int i = 0; i < RE_NUM_BUFFERS; ++i) {
			D3D12_InitSBT(&_sbt[i]);

			D3D12_GPU_DESCRIPTOR_HANDLE handle = _descHeap[i]->GetGPUDescriptorHandleForHeapStart();

			D3D12_AddShader(&_sbt[i], SET_RayGen, L"RayGen", (void *)handle.ptr);
			D3D12_AddShader(&_sbt[i], SET_Miss, L"Miss", NULL);
			D3D12_AddShader(&_sbt[i], SET_HitGroup, L"HitGroup", NULL);

			if (!D3D12_BuildSBT(&_sbt[i], _psoProps))
				return false;
		}
	}

	_descIncrement = Re_Device.dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	return true;
}

void
RtRenderPath::RenderScene(const struct Scene *s, ID3D12Resource *output, D3D12_RESOURCE_STATES outputState)
{
	const struct SceneRenderData *srd = (const struct SceneRenderData *)&s->renderDataStart;
	ID3D12GraphicsCommandList4 *cmdList = Re_MainThreadWorker.rtCmdList;
	ID3D12Resource *rtOutput = NULL;

	{ // Output buffer
		D3D12_RESOURCE_DESC desc = {};
		desc.DepthOrArraySize = 1;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		desc.Width = *E_ScreenWidth;
		desc.Height = *E_ScreenHeight;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;

		rtOutput = D3D12_CreateTransientResource(&desc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		rtOutput->SetName(L"Raytracing Output Buffer");
	}

	{ // Descriptor Heap
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.RaytracingAccelerationStructure.Location = srd->asBuffer->GetGPUVirtualAddress();

		#define INC_HANDLES(x)		\
			cpuHandle.ptr += x;		\
			gpuHandle.ptr += x

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = _descHeap[Re_Device.frame]->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = _descHeap[Re_Device.frame]->GetGPUDescriptorHandleForHeapStart();
	
		Re_Device.dev->CreateUnorderedAccessView(rtOutput, NULL, &uavDesc, cpuHandle);
		INC_HANDLES(_descIncrement);

		Re_Device.dev->CreateShaderResourceView(NULL, &srvDesc, cpuHandle);
		INC_HANDLES(_descIncrement);

		/*srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = srd->materialData.count;
		srvDesc.Buffer.StructureByteStride = sizeof(struct D3D12Material);

		Re_Device.dev->CreateShaderResourceView(srd->materialBuffer, &srvDesc, cpuHandle);
		INC_HANDLES(_descIncrement);*/

		ID3D12DescriptorHeap* heaps[] = { _descHeap[Re_Device.frame] };
		cmdList->SetDescriptorHeaps(_countof(heaps), heaps);

		cmdList->SetComputeRootSignature(_globalRootSignature);

		Re_Device.dev->CopyDescriptorsSimple((UINT)srd->buffers.count, cpuHandle,
			srd->vtxHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		cmdList->SetComputeRootDescriptorTable(0, gpuHandle);
		INC_HANDLES(srd->buffers.count * _descIncrement);

		Re_Device.dev->CopyDescriptorsSimple((UINT)srd->buffers.count, cpuHandle,
			srd->idxHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		cmdList->SetComputeRootDescriptorTable(1, gpuHandle);
		INC_HANDLES(srd->buffers.count * _descIncrement);

		D3D12_UpdateTextureHeap(cpuHandle);
		cmdList->SetComputeRootDescriptorTable(2, gpuHandle);

		cmdList->SetComputeRootShaderResourceView(3, srd->materialBuffer->GetGPUVirtualAddress());
		cmdList->SetComputeRootConstantBufferView(4, srd->dataBuffer->GetGPUVirtualAddress());
	}

	const struct ShaderBindingTable *sbt = &_sbt[Re_Device.frame];

	D3D12_DISPATCH_RAYS_DESC desc = {};
	desc.RayGenerationShaderRecord.StartAddress = sbt->res->GetGPUVirtualAddress();
	desc.RayGenerationShaderRecord.SizeInBytes = sbt->rayGenSectionSize;
	desc.MissShaderTable.StartAddress = sbt->res->GetGPUVirtualAddress() + sbt->rayGenSectionSize;
	desc.MissShaderTable.SizeInBytes = sbt->missSize;
	desc.MissShaderTable.StrideInBytes = sbt->missSize;
	desc.HitGroupTable.StartAddress = sbt->res->GetGPUVirtualAddress() + sbt->rayGenSectionSize + sbt->missSectionSize;
	desc.HitGroupTable.SizeInBytes = sbt->hitGroupSize;
	desc.HitGroupTable.StrideInBytes = sbt->hitGroupSize;

	desc.Width = *E_ScreenWidth;
	desc.Height = *E_ScreenHeight;
	desc.Depth = 1;

	cmdList->SetPipelineState1(_pso);
	cmdList->DispatchRays(&desc);

	D3D12_RESOURCE_BARRIER barriers[] =
	{
		//CD3DX12_RESOURCE_BARRIER::Transition(srd->materialBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
		CD3DX12_RESOURCE_BARRIER::Transition(rtOutput, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
		CD3DX12_RESOURCE_BARRIER::Transition(output, outputState, D3D12_RESOURCE_STATE_COPY_DEST)
	};
	cmdList->ResourceBarrier(_countof(barriers), barriers);

	cmdList->CopyResource(output, rtOutput);

	D3D12_RESOURCE_BARRIER endBarriers[] =
	{
//		CD3DX12_RESOURCE_BARRIER::Transition(srd->materialBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST),
		CD3DX12_RESOURCE_BARRIER::Transition(output, D3D12_RESOURCE_STATE_COPY_DEST, outputState)
	};
	cmdList->ResourceBarrier(_countof(endBarriers), endBarriers);
}

void
RtRenderPath::Term()
{
	_pso->Release();
	_psoProps->Release();

	_hitSignature->Release();
	_missSignature->Release();
	_rayGenSignature->Release();
	_globalRootSignature->Release();

	for (uint32_t i = 0; i < RE_NUM_BUFFERS; ++i) {
		D3D12_TermSBT(&_sbt[i]);
		_descHeap[i]->Release();
	}
}
