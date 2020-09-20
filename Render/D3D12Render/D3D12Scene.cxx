#include <assert.h>

#include <Engine/Job.h>
#include <Engine/ECSystem.h>
#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <Scene/Transform.h>
#include <Render/Render.h>
#include <Render/Device.h>
#include <Render/Material.h>
#include <Render/ModelRender.h>

#include <Engine/Resource.h>
#include <Engine/Component.h>

#include <DirectXMath.h>

#include "D3D12Render.h"
#include "D3D12Material.h"

using namespace DirectX;

struct SceneModelBuffers
{
	ID3D12Resource *vtxBuffer, *idxBuffer;
	UINT firstVtx, vtxCount, firstIdx, idxCount;
};

const size_t Re_SceneRenderDataSize = sizeof(struct SceneRenderData);

static UINT _descIncrement;
static inline void _CreateHeaps(struct SceneRenderData *srd);

bool
Re_InitScene(struct Scene *scene)
{
	if (!Re_Features.rayTracing)
		return true;

	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;

	srd->heapSize = 100;
	Rt_InitArray(&srd->instanceData, srd->heapSize, sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
	Rt_InitArray(&srd->materialData, srd->heapSize, sizeof(struct D3D12Material));
	Rt_InitArray(&srd->buffers, srd->heapSize, sizeof(struct SceneModelBuffers));

	_CreateHeaps(srd);

	_descIncrement = Re_Device.dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	return true;
}

void
Re_TermScene(struct Scene *scene)
{
	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;

	srd->vtxHeap->Release();
	srd->idxHeap->Release();

	Rt_TermArray(&srd->instanceData);
	Rt_TermArray(&srd->materialData);
	Rt_TermArray(&srd->buffers);
}

void
D3D12_UpdateSceneData(struct Scene *scene)
{
	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;

	m4_inverse(&srd->shaderData.inverseView, &Scn_ActiveCamera->viewMatrix);
	m4_inverse(&srd->shaderData.inverseProjection, &Scn_ActiveCamera->projMatrix);

/*	m4_mul(&srd->shaderData.viewProjection, &Scn_ActiveCamera->projMatrix, &Scn_ActiveCamera->viewMatrix);
	m4_inverse(&srd->shaderData.inverseViewProjection, &srd->shaderData.viewProjection);*/

	srd->shaderData.aspect = (float)*E_ScreenWidth / (float)*E_ScreenHeight;
	srd->shaderData.numSamples = 4;

	srd->dataBuffer = D3D12_CreateTransientResource(&CD3DX12_RESOURCE_DESC::Buffer(sizeof(srd->shaderData)), D3D12_RESOURCE_STATE_COPY_DEST);
	srd->dataBuffer->SetName(L"Scene Data Buffer");
	D3D12_StageUpload(srd->dataBuffer, sizeof(srd->shaderData), &srd->shaderData, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
}

void
D3D12_BuildTLAS(ID3D12GraphicsCommandList4 *cmdList, struct Scene *scene)
{
	ID3D12Resource *scratch = NULL, *tlas = NULL;
	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;

	Rt_ClearArray(&srd->instanceData, false);
	Rt_ClearArray(&srd->materialData, false);
	Rt_ClearArray(&srd->buffers, false);
	E_ExecuteSystemS(scene, PREPARE_SCENE_DATA_SYS, srd);

	if (srd->buffers.size > srd->heapSize) {
		srd->heapSize = srd->buffers.size;
		_CreateHeaps(srd);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE vtxHandle = srd->vtxHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE idxHandle = srd->idxHeap->GetCPUDescriptorHandleForHeapStart();

	for (size_t i = 0; i < srd->buffers.count; ++i) {
		struct SceneModelBuffers *smb = (struct SceneModelBuffers *)Rt_ArrayGet(&srd->buffers, i);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		srvDesc.Format = DXGI_FORMAT_UNKNOWN; 
		srvDesc.Buffer.FirstElement = smb->firstVtx;
		srvDesc.Buffer.NumElements = smb->vtxCount;
		srvDesc.Buffer.StructureByteStride = sizeof(struct Vertex);
		Re_Device.dev->CreateShaderResourceView(smb->vtxBuffer, &srvDesc, vtxHandle);
		vtxHandle.ptr += _descIncrement;

		srvDesc.Buffer.FirstElement = smb->firstIdx;
		srvDesc.Buffer.NumElements = smb->idxCount;
		srvDesc.Buffer.StructureByteStride = sizeof(uint32_t);
		Re_Device.dev->CreateShaderResourceView(smb->idxBuffer, &srvDesc, idxHandle);
		idxHandle.ptr += _descIncrement;
	}

	const UINT64 materialSize = srd->materialData.elem_size * srd->materialData.count;
	srd->materialBufferSize = ROUND_UP(materialSize, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
	srd->materialBuffer = D3D12_CreateTransientResource(&CD3DX12_RESOURCE_DESC::Buffer(
		max(srd->materialBufferSize, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_COPY_DEST);
	srd->materialBuffer->SetName(L"Scene Material Buffer");

	D3D12_StageUpload(srd->materialBuffer, materialSize, srd->materialData.data, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc = { 0 };
	desc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	desc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	desc.Inputs.NumDescs = (UINT)srd->instanceData.count;
	desc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
	
	Re_Device.dev->GetRaytracingAccelerationStructurePrebuildInfo(&desc.Inputs, &info);
	
	info.ResultDataMaxSizeInBytes = ROUND_UP(info.ResultDataMaxSizeInBytes, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
	info.ScratchDataSizeInBytes = ROUND_UP(info.ScratchDataSizeInBytes, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
	UINT64 instDescSize = ROUND_UP(Rt_ArrayByteSize(&srd->instanceData), D3D12_RAYTRACING_INSTANCE_DESCS_BYTE_ALIGNMENT);

	scratch = D3D12_CreateTransientResource(&CD3DX12_RESOURCE_DESC::Buffer(info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	scratch->SetName(L"Scene Scratch Buffer");

	tlas = D3D12_CreateTransientResource(&CD3DX12_RESOURCE_DESC::Buffer(info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
	tlas->SetName(L"Scene TLAS Buffer");

	D3D12_GPU_VIRTUAL_ADDRESS instPtr;
	D3D12_RAYTRACING_INSTANCE_DESC *inst =
		(D3D12_RAYTRACING_INSTANCE_DESC *)D3D12_AllocStagingArea(sizeof(*inst) * srd->instanceData.count, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT, &instPtr);
	
	memcpy(inst, srd->instanceData.data, sizeof(*inst) * srd->instanceData.count);

	desc.Inputs.InstanceDescs = instPtr;
	desc.DestAccelerationStructureData = tlas->GetGPUVirtualAddress();
	desc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();

	cmdList->BuildRaytracingAccelerationStructure(&desc, 0, NULL);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(tlas));

	srd->asBuffer = tlas;
}

void
D3D12_PrepareSceneData(void **comp, struct SceneRenderData *args)
{
	struct Transform *xform = (struct Transform *)comp[0];
	struct ModelRender *modelRender = (struct ModelRender *)comp[1];

	struct Model *model = (struct Model *)E_ResourcePtr(modelRender->model);
	struct ModelRenderData *mrd = (struct ModelRenderData *)&model->renderDataStart;

	if (!model)
		return;

	struct mat4 mat;
	m4_transpose(&mat, &xform->mat);

	for (size_t i = 0; i < model->numMeshes; ++i) {
		struct Mesh *mesh = &model->meshes[i];
		struct SceneModelBuffers *smb = (struct SceneModelBuffers *)Rt_ArrayAllocate(&args->buffers);

		smb->vtxBuffer = mrd->vtxBuffer;
		smb->firstVtx = mesh->firstVertex;
		smb->vtxCount = mesh->vertexCount;

		smb->idxBuffer = mrd->idxBuffer;
		smb->firstIdx = mesh->firstIndex;
		smb->idxCount = mesh->indexCount;

		D3D12_RAYTRACING_INSTANCE_DESC *desc = (D3D12_RAYTRACING_INSTANCE_DESC *)Rt_ArrayAllocate(&args->instanceData);
		memcpy(&desc->Transform, &mat, sizeof(desc->Transform));
		desc->InstanceContributionToHitGroupIndex = 0;
		desc->InstanceMask = 0xFF;
		desc->InstanceID = i;
		desc->AccelerationStructure = mrd->structures[i].asBuffer->GetGPUVirtualAddress();
		desc->Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;

		struct D3D12Material *mtl = (struct D3D12Material *)Rt_ArrayAllocate(&args->materialData);

		memcpy(&mtl->diffuseColor[0], &model->materialInstances[i].props.color.a, sizeof(float) * 7);
		mtl->metallic = model->materialInstances[i].props.metallic;
		mtl->roughness = model->materialInstances[i].props.roughness;
		
		for (size_t j = 0; j < RE_MAX_TEXTURES; ++j) {
			if (model->materialInstances[i].textures[j] == E_INVALID_HANDLE)
				break;

			mtl->textures[j] = E_ResHandleToGPU(model->materialInstances[i].textures[j]);
		}
	}
}

void
_CreateHeaps(struct SceneRenderData *srd)
{
	if (srd->vtxHeap)
		srd->vtxHeap->Release();

	if (srd->idxHeap)
		srd->idxHeap->Release();

	D3D12_DESCRIPTOR_HEAP_DESC dhd{};
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	dhd.NumDescriptors = (UINT)srd->heapSize;

	D3DCHK(Re_Device.dev->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&srd->vtxHeap)));
	D3DCHK(Re_Device.dev->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&srd->idxHeap)));
}

