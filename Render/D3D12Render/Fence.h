#pragma once

#include <Render/Device.h>

#include "D3D12Render.h"

struct Fence
{
	ID3D12Fence *fence;
	HANDLE event;
	UINT64 value;
};

static inline bool
InitFence(struct Fence *f)
{
	if (FAILED(Re_Device.dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&f->fence))))
		return false;

	f->event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (f->event == INVALID_HANDLE_VALUE)
		return false;

	f->value = 0;

	return true;
}

static inline HRESULT
SignalFence(struct Fence *f, ID3D12CommandQueue *queue)
{
	return queue->Signal(f->fence, ++f->value);
}

static inline void
WaitForFenceCPU(struct Fence *f)
{
	if (f->fence->GetCompletedValue() < f->value) {
		D3DCHK(f->fence->SetEventOnCompletion(f->value, f->event));
		WaitForSingleObject(f->event, INFINITE);
	}

	++f->value;
}

static inline HRESULT
WaitForFenceGPU(struct Fence *f, ID3D12CommandQueue *queue)
{
	return queue->Wait(f->fence, f->value);
}

static inline void
TermFence(struct Fence *f)
{
	f->fence->Release();
	CloseHandle(f->event);
}
