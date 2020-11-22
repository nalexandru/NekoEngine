#include <assert.h>

#include <System/Log.h>
#include <Engine/Config.h>
#include <Render/Device.h>
#include <Render/Material.h>

#include "Fence.h"
#include "Buffer.h"
#include "D3D12Render.h"

#define D3D12STMOD	L"D3D12Staging"

struct Fence Re_UploadFence, Re_ASFence;

struct ASBuildInfo
{
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc;
	D3D12_RAYTRACING_GEOMETRY_DESC geomDesc;
	ID3D12Resource *as;
};

static ID3D12Resource *_stagingBuffer;
static Array _blasBuildInfo, _blasBarriers;
static uint8_t *_bufferPtr;
static uint64_t *_size, _offset = 0, _peakSize = 0;

static inline void _CheckFlush(UINT64 size, UINT64 alignment);

bool
D3D12_InitStaging(void)
{
	_size = &E_GetCVarU64(L"D3D12_StagingBufferSize", 64ULL * 1024ULL * 1024ULL)->u64;

	InitFence(&Re_UploadFence);
	InitFence(&Re_ASFence);

	D3D12_HEAP_PROPERTIES hp{};
	hp.Type = D3D12_HEAP_TYPE_UPLOAD;

	CD3DX12_RESOURCE_DESC rd = CD3DX12_RESOURCE_DESC::Buffer(*_size);
	D3DCHK(Re_Device.dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&_stagingBuffer)));
	_stagingBuffer->SetName(L"Staging Buffer");

	_stagingBuffer->Map(0, NULL, (void **)&_bufferPtr);

	if (Re.features.rayTracing) {
		Rt_InitArray(&_blasBuildInfo, 10, sizeof(struct ASBuildInfo));
		Rt_InitArray(&_blasBarriers, 10, sizeof(D3D12_RESOURCE_BARRIER));
	}

	return true;
}

void
D3D12_StageUpload(ID3D12Resource *dest, UINT64 size, const void *data, UINT64 alignment, UINT64 rowPitch, UINT64 slicePitch)
{
	UINT64 offset = 0;
	ID3D12Resource *staging = _stagingBuffer;
	D3D12_SUBRESOURCE_DATA srd = { data, (LONG_PTR)size, (LONG_PTR)size };

	if (rowPitch)
		srd.RowPitch = rowPitch;
	
	if (slicePitch)
		srd.SlicePitch = slicePitch;

	D3D12_RESOURCE_DESC rd = dest->GetDesc();
	D3D12_RESOURCE_ALLOCATION_INFO ai = Re_Device.dev->GetResourceAllocationInfo(0, 1, &rd);

	if (!size)
		size = ai.SizeInBytes;

	if (!alignment)
		alignment = ai.Alignment;

	if (size > * _size) {
		D3D12_HEAP_PROPERTIES hp = { D3D12_HEAP_TYPE_UPLOAD };
		rd = CD3DX12_RESOURCE_DESC::Buffer(size, D3D12_RESOURCE_FLAG_NONE, alignment);
		D3DCHK(Re_Device.dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd,
			D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_PPV_ARGS(&staging)));
	} else {
		_offset = ROUND_UP(_offset, alignment);
		_CheckFlush(size, alignment);

		offset = _offset;
		_offset += size;
	}

	struct RenderWorker *w = D3D12_CurrentThreadWorker();
	w->copyList->Reset(w->copyAllocators[Re_Device.frame], NULL);
	UpdateSubresources(w->copyList, dest, staging, offset, 0, 1, &srd);
	w->copyList->Close();

	Re_Device.transferQueue->ExecuteCommandLists(1, (ID3D12CommandList **)&w->copyList);

	if (staging != _stagingBuffer)
		D3D12_DestroyResource(staging);
}

void *
D3D12_AllocStagingArea(UINT64 size, UINT64 alignment, D3D12_GPU_VIRTUAL_ADDRESS *gpuHandle)
{
	_offset = ROUND_UP(_offset, alignment);
	_CheckFlush(size, alignment);

	void *ret = _bufferPtr + _offset;

	if (gpuHandle) {
		*gpuHandle = _stagingBuffer->GetGPUVirtualAddress();
		*gpuHandle += _offset;
	}

	_offset += size;

	return ret;
}

void
D3D12_StageBLASBuild(struct Model *model, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags)
{
	struct ModelRenderData *mrd = (struct ModelRenderData *)&model->renderDataStart;

	for (uint32_t i = 0; i < model->numMeshes; ++i) {
		struct Mesh m = model->meshes[i];

		struct ASBuildInfo *bi = (struct ASBuildInfo *)Rt_ArrayAllocate(&_blasBuildInfo);
		bi->buildDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		bi->buildDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		bi->buildDesc.Inputs.NumDescs = 1;
		bi->buildDesc.Inputs.pGeometryDescs = NULL;
		bi->buildDesc.Inputs.Flags = buildFlags;
		bi->buildDesc.DestAccelerationStructureData = mrd->structures[i].asBuffer->GetGPUVirtualAddress();
		bi->buildDesc.ScratchAccelerationStructureData = mrd->structures[i].scratchBuffer->GetGPUVirtualAddress();
	
		bi->geomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		bi->geomDesc.Flags = model->materialInstances[i].props.alphaMode == ALPHA_MODE_OPAQUE ? D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE : D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
		bi->geomDesc.Triangles.VertexCount = m.vertexCount;
		bi->geomDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		bi->geomDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(struct Vertex);
		bi->geomDesc.Triangles.VertexBuffer.StartAddress = mrd->vtxBuffer->GetGPUVirtualAddress() + (sizeof(struct Vertex) * m.firstVertex);
		bi->geomDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
		bi->geomDesc.Triangles.IndexBuffer = mrd->idxBuffer->GetGPUVirtualAddress() + (sizeof(uint32_t) * m.firstIndex);
		bi->geomDesc.Triangles.IndexCount = m.indexCount;

		bi->as = mrd->structures[i].asBuffer;
	}
}

void
D3D12_ResetUploadHeap(void)
{
	_peakSize = max(_peakSize, _offset);
	_offset = 0;
}

void
D3D12_BuildBLAS(ID3D12GraphicsCommandList4 *cmdList)
{
	if (!_blasBuildInfo.count)
		return;

	for (size_t i = 0; i < _blasBuildInfo.count; ++i) {
		struct ASBuildInfo *bi = (ASBuildInfo *)Rt_ArrayGet(&_blasBuildInfo, i);
		bi->buildDesc.Inputs.pGeometryDescs = &bi->geomDesc;
		cmdList->BuildRaytracingAccelerationStructure(&bi->buildDesc, 0, NULL);
		D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::UAV(bi->as);
		Rt_ArrayAdd(&_blasBarriers, &barrier);
	}

	// https://www.gdcvault.com/play/1026721/RTX-Ray-Tracing-Best-Practices
	// Suggests that barriers are not needed for individual BLAS builds, only one for the TLAS. ~3:20.
	cmdList->ResourceBarrier((UINT)_blasBarriers.count, (const D3D12_RESOURCE_BARRIER *)_blasBarriers.data);

	Rt_ClearArray(&_blasBarriers, false);
	Rt_ClearArray(&_blasBuildInfo, false);
}

void
D3D12_TermStaging(void)
{
	Sys_LogEntry(D3D12STMOD, LOG_INFORMATION, L"Peak usage: %u/%u B (%.02f%%)", _peakSize, *_size, ((double)_peakSize / (double)*_size) * 100.0);

	TermFence(&Re_ASFence);
	TermFence(&Re_UploadFence);

	if (Re.features.rayTracing) {
		Rt_TermArray(&_blasBarriers);
		Rt_TermArray(&_blasBuildInfo);
	}

	_stagingBuffer->Unmap(0, NULL);
	_stagingBuffer->Release();
}

void
_CheckFlush(UINT64 size, UINT64 alignment)
{
	if (_offset + size > *_size) {
		Sys_LogEntry(D3D12STMOD, LOG_WARNING, L"Flushing staging buffer, %llu bytes overflow", _offset + size - *_size);

		SignalFence(&Re_UploadFence, Re_Device.transferQueue);
		WaitForFenceCPU(&Re_UploadFence);
	
		_offset = ROUND_UP(0, alignment);
	}
}

