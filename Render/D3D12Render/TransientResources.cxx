#include <assert.h>

#include <System/Log.h>
#include <Engine/Config.h>
#include <Render/Render.h>
#include <Render/Device.h>

#include "D3D12Render.h"

#define D3D12TRMOD L"D3D12Transient"

static ID3D12Heap *_transientHeap;
static uint64_t *_heapSize, _heapOffset, _frameStart, _peakSize;

bool
D3D12_InitTransientHeap(void)
{
	_heapSize = &E_GetCVarU64(L"D3D12_TransientHeapSize", 64 * 1024 * 1024)->u64;

	D3D12_HEAP_DESC hd{};
	hd.Flags = D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES;
	hd.Alignment = D3D12_DEFAULT_MSAA_RESOURCE_PLACEMENT_ALIGNMENT;
	hd.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
	hd.SizeInBytes = *_heapSize * RE_NUM_BUFFERS;

	HRESULT hr = Re_Device.dev->CreateHeap(&hd, IID_PPV_ARGS(&_transientHeap));

	return SUCCEEDED(hr);
}

ID3D12Resource *
D3D12_CreateTransientResource(D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState, const D3D12_CLEAR_VALUE* clearValue)
{
	ID3D12Resource *res;

	D3D12_RESOURCE_ALLOCATION_INFO ai = Re_Device.dev->GetResourceAllocationInfo(0, 1, desc);
	_heapOffset = ROUND_UP(_heapOffset, ai.Alignment);

	assert(_heapOffset - _frameStart + ai.SizeInBytes < *_heapSize);

	D3DCHK(Re_Device.dev->CreatePlacedResource(_transientHeap, _heapOffset, desc, initialState, clearValue, IID_PPV_ARGS(&res)));
	_heapOffset += ai.SizeInBytes;

	D3D12_DestroyResource(res);

	return res;
}

void
D3D12_ResetTransientHeap(void)
{
	_peakSize = max(_peakSize, _heapOffset - _frameStart);
	_frameStart = Re_Device.frame * *_heapSize;
	_heapOffset = _frameStart;
}

void
D3D12_TermTransientHeap(void)
{
	Sys_LogEntry(D3D12TRMOD, LOG_INFORMATION, L"Peak heap size: %llu/%llu B (%.02f%%)", _peakSize, *_heapSize, ((double)_peakSize / (double)*_heapSize) * 100.0);

	_transientHeap->Release();
}
