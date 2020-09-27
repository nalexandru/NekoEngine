#include <Windows.h>

#include <Engine/Job.h>
#include <Render/Device.h>
#include <System/Memory.h>

#include "D3D12Render.h"

const size_t Re_ModelRenderDataSize = sizeof(struct ModelRenderData);

bool
Re_InitModel(const char *name, struct Model *m)
{
	struct ModelRenderData *mrd = (struct ModelRenderData *)&m->renderDataStart;
	wchar_t *buff = (wchar_t *)Sys_Alloc(sizeof(wchar_t), 512, MH_Transient);
	UINT64 vertexSize = sizeof(struct Vertex) * m->numVertices;
	UINT64 indexSize = sizeof(uint32_t) * m->numIndices;
	UINT64 scratchSize = 0, resultSize = 0;
	D3D12_RAYTRACING_GEOMETRY_DESC geomDesc = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS buildInputs = {};

	if (Re_Features.rayTracing) {
		geomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geomDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(struct Vertex);
		geomDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geomDesc.Triangles.VertexCount = m->numVertices;
		geomDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
		geomDesc.Triangles.IndexCount = m->numIndices;
		geomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
	
		buildInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		buildInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		buildInputs.NumDescs = 1;
		buildInputs.pGeometryDescs = &geomDesc;
		buildInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
		Re_Device.dev->GetRaytracingAccelerationStructurePrebuildInfo(&buildInputs, &info);
	
		scratchSize = ROUND_UP(info.ScratchDataSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		resultSize = ROUND_UP(info.ResultDataMaxSizeInBytes, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT);
	}

	{ // Create resources
		D3D12_HEAP_PROPERTIES hp{ D3D12_HEAP_TYPE_DEFAULT };

		if (HRESULT hr = Re_Device.dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexSize),
				D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_PPV_ARGS(&mrd->vtxBuffer)); FAILED(hr))
			return false;

		swprintf(buff, 512, L"%hs Vertex Buffer", name);
		mrd->vtxBuffer->SetName(buff);

		if (HRESULT hr = Re_Device.dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(indexSize),
				D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_PPV_ARGS(&mrd->idxBuffer)); FAILED(hr))
			return false;

		swprintf(buff, 512, L"%hs Index Buffer", name);
		mrd->idxBuffer->SetName(buff);

		if (Re_Features.rayTracing) {
			mrd->structures = (struct AccelerationStructure *)calloc(m->numMeshes, sizeof(*mrd->structures));
			if (!mrd->structures)
				return false;

			for (uint32_t i = 0; i < m->numMeshes; ++i) {
				if (HRESULT hr = Re_Device.dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE,
						&CD3DX12_RESOURCE_DESC::Buffer(scratchSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
						D3D12_RESOURCE_STATE_UNORDERED_ACCESS, NULL, IID_PPV_ARGS(&mrd->structures[i].scratchBuffer)); FAILED(hr))
					return false;

				swprintf(buff, 512, L"%hs Scratch Buffer", name);
				mrd->structures[i].scratchBuffer->SetName(buff);

				if (HRESULT hr = Re_Device.dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE,
						&CD3DX12_RESOURCE_DESC::Buffer(resultSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
						D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL, IID_PPV_ARGS(&mrd->structures[i].asBuffer)); FAILED(hr))
					return false;

				swprintf(buff, 512, L"%hs BLAS Buffer", name);
				mrd->structures[i].asBuffer->SetName(buff);
			}
		}
	}

	mrd->vtxBufferView.BufferLocation = mrd->vtxBuffer->GetGPUVirtualAddress();
	mrd->vtxBufferView.StrideInBytes = sizeof(struct Vertex);
	mrd->vtxBufferView.SizeInBytes = (UINT)vertexSize;

	mrd->idxBufferView.BufferLocation = mrd->idxBuffer->GetGPUVirtualAddress();
	mrd->idxBufferView.Format = DXGI_FORMAT_R32_UINT;
	mrd->idxBufferView.SizeInBytes = (UINT)indexSize;

	D3D12_StageUpload(mrd->vtxBuffer, vertexSize, m->vertices);
	D3D12_StageUpload(mrd->idxBuffer, indexSize, m->indices);

	if (Re_Features.rayTracing)
		D3D12_StageBLASBuild(m, buildInputs.Flags);

	return true;
}

void
Re_TermModel(struct Model *m)
{
	struct ModelRenderData *mrd = (struct ModelRenderData *)&m->renderDataStart;

	D3D12_DestroyResource(mrd->vtxBuffer);
	D3D12_DestroyResource(mrd->idxBuffer);

	if (Re_Features.rayTracing) {
		for (uint32_t i = 0; i < m->numMeshes; ++i) {
			D3D12_DestroyResource(mrd->structures[i].scratchBuffer);
			D3D12_DestroyResource(mrd->structures[i].asBuffer);
		}
		free(mrd->structures);
	}
}
