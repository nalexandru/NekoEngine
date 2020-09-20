#include <System/Log.h>
#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Render/Render.h>
#include <Render/Device.h>
#include <System/Memory.h>

#include "../VKRender.h"

//#include "RtRenderPath.h"

#define FWRPMOD	L"RtRenderPath"

static VkPipeline _rtPipeline;
static VkPipelineLayout _rtPipelineLayout;

static bool
_Init(void)
{
	if (!Re_Features.rayTracing)
		return false;

	{ // Descriptor heap
		/*D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = 3 * RE_NUM_BUFFERS;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		hr = Re_Device.dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_descHeap));
		if (FAILED(hr)) {
			Sys_LogEntry(RTRPMOD, LOG_CRITICAL, L"Failed to create descriptor heap: 0x%x", hr);
			return false;
		}

		_descHeap->SetName(L"Raytracing Descriptor Heap");*/
	}

	{ // Ray gen signature
		/*D3D12_DESCRIPTOR_RANGE ranges[] = 
		{
			{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, 0 },
			{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 1 },
			{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 2 }
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

		sigBlob->Release();*/
	}

	{ // Global root signature
		/*CD3DX12_DESCRIPTOR_RANGE ranges[2];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1024, 0, 1);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

		CD3DX12_ROOT_PARAMETER params[2];
		params[0].InitAsDescriptorTable(1, &ranges[0]);
		params[1].InitAsDescriptorTable(1, &ranges[1]);

		CD3DX12_ROOT_SIGNATURE_DESC rsDesc;
		rsDesc.Init(_countof(params), params);

		ID3DBlob *sigBlob;
		ID3DBlob *errBlob;
		hr = D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &sigBlob, &errBlob);
		if (FAILED(hr)) {
			Sys_LogEntry(RTRPMOD, LOG_CRITICAL, L"Failed to serialize global root signature: %s [0x%x]", errBlob->GetBufferPointer(), hr);
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

		sigBlob->Release();*/
	}

	{ // Hit & Miss signature
		/*D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
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

		sigBlob->Release();*/
	}

	{ // Pipeline state
		/*CD3DX12_STATE_OBJECT_DESC desc{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

		_library = D3D12_LoadShaderLibrary("/Shaders/HLSL6/PathTracer.hlsl");
		CD3DX12_DXIL_LIBRARY_SUBOBJECT *lib = desc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

		lib->SetDXILLibrary(&_library->desc.DXILLibrary);
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

		Re_Device.dev->CreateStateObject(desc, IID_PPV_ARGS(&_pso));
		_pso->QueryInterface(IID_PPV_ARGS(&_psoProps));

		_pso->SetName(L"Raytracing PSO");*/
	}

	{ // Shader binding table
		/*D3D12_InitSBT(&_sbt);

		D3D12_GPU_DESCRIPTOR_HANDLE handle = _descHeap->GetGPUDescriptorHandleForHeapStart();

		D3D12_AddShader(&_sbt, SET_RayGen, L"RayGen", (void *)handle.ptr);
		D3D12_AddShader(&_sbt, SET_Miss, L"Miss", NULL);
		D3D12_AddShader(&_sbt, SET_HitGroup, L"HitGroup", NULL);

		if (!D3D12_BuildSBT(&_sbt, _psoProps))
			return false;*/
	}

	return true;
}

static void
_RenderScene(const struct Scene *s, VkImage output, VkImageView outputView, uint32_t outputState)
{
	const struct SceneRenderData *srd = (const struct SceneRenderData *)&s->renderDataStart;
	VkCommandBuffer cmdBuffer = VK_GraphicsCommandBuffer(0, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	VkImage rtOutput;
	VkImageView rtOutputView;
	VkWriteDescriptorSet *wds;

	{ // Output Buffer
		VkImageCreateInfo ici =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_UNORM,
			.extent = { *E_ScreenWidth, *E_ScreenHeight, 1 },
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};
		rtOutput = VK_CreateTransientImage(&ici);

		VkImageViewCreateInfo ivci =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = rtOutput,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_R8G8B8A8_UNORM,
			.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
			.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
		};
		vkCreateImageView(Re_Device.dev, &ivci, VK_CPUAllocator, &rtOutputView);
	}

	{ // Descriptor Set
		wds = Sys_Alloc(sizeof(*wds), 2, MH_Transient);
		memset(wds, 0x0, sizeof(*wds) * 2);

		VkDescriptorImageInfo *rtOutputII = Sys_Alloc(sizeof(*rtOutputII), 1, MH_Transient);
		rtOutputII->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		rtOutputII->imageView = rtOutputView;
		rtOutputII->sampler = VK_NULL_HANDLE;

		wds[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds[0].dstSet = 0;
		wds[0].dstBinding = 0;
		wds[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		wds[0].descriptorCount = 1;
		wds[0].pImageInfo = rtOutputII;

		VkWriteDescriptorSetAccelerationStructureKHR *writeAS = Sys_Alloc(sizeof(*writeAS), 1, MH_Transient);
		writeAS->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		writeAS->pNext = NULL;
		writeAS->accelerationStructureCount = 1;
		writeAS->pAccelerationStructures = &srd->topLevelAS;

		wds[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wds[1].pNext = writeAS;
		wds[1].dstSet = 0;
		wds[1].dstBinding = 1;
		wds[1].descriptorCount = 1;
	}

//	VkDescriptorSet sets[] = { NULL };
//	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _rtPipelineLayout, 0, _countof(sets), sets, 0, NULL);

	// transition

	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _rtPipeline);
	vkCmdPushDescriptorSetKHR(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _rtPipelineLayout, 0, 2, wds);

	vkCmdTraceRaysKHR(cmdBuffer, NULL, NULL, NULL, NULL, *E_ScreenWidth, *E_ScreenHeight, 1);

	// transition barrier

	// copy to output
	VkImageCopy copy =
	{
		.srcSubresource = 0,
		.srcOffset = { 0, 0, 0 },
		.dstSubresource = 0,
		.dstOffset = { 0, 0, 0 },
		.extent = { *E_ScreenWidth, *E_ScreenHeight, 1 }
	};
	vkCmdCopyImage(cmdBuffer, rtOutput, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, output, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

	// transition output

}

void
_Term()
{
	/*D3D12_TermSBT(&_sbt);

	_hitSignature->Release();
	_missSignature->Release();
	_rayGenSignature->Release();

	_descHeap->Release();*/
}
