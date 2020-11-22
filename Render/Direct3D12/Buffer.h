#pragma once

#include <Render/Device.h>

#include "D3D12Render.h"

struct Buffer
{
	ID3D12Resource *res;
	D3D12_RESOURCE_STATES state;
	UINT64 size;
};

static inline bool
InitBuffer(struct Buffer *b, UINT64 size, D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT)
{
	b->state = initialState;
	b->size = size;

	HRESULT hr;
	D3D12_HEAP_PROPERTIES hp{ heapType };
	CD3DX12_RESOURCE_DESC rd = CD3DX12_RESOURCE_DESC::Buffer(b->size);
	hr = Re_Device.dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, b->state, NULL, IID_PPV_ARGS(&b->res));

	if (FAILED(hr))
		return false;

	return true;
}

static inline void
TermBuffer(struct Buffer *b)
{
	D3D12_DestroyResource(b->res);
	ZeroMemory(b, sizeof(*b));
}

