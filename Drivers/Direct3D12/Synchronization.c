#include "D3D12Driver.h"

struct Semaphore *
D3D12_CreateSemaphore(struct RenderDevice *dev)
{
	return NULL;
}

void
D3D12_DestroySemaphore(struct RenderDevice *dev, struct Semaphore *s)
{

}

struct Fence *
D3D12_CreateFence(struct RenderDevice *dev, bool createSignaled)
{
	return NULL;
}

void
D3D12_SignalFence(struct RenderDevice *dev, struct Fence *f)
{
}

bool
D3D12_WaitForFence(struct RenderDevice *dev, struct Fence *f, uint64_t timeout)
{
	return false;
}

void
D3D12_DestroyFence(struct RenderDevice *dev, struct Fence *f)
{
}
