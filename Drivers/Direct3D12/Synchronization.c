#include "D3D12Driver.h"

struct NeSemaphore *
D3D12_CreateSemaphore(struct NeRenderDevice *dev)
{
	return NULL;
}

void
D3D12_DestroySemaphore(struct NeRenderDevice *dev, struct NeSemaphore *s)
{

}

struct NeFence *
D3D12_CreateFence(struct NeRenderDevice *dev, bool createSignaled)
{
	struct NeFence *f = Sys_Alloc(sizeof(*f), 1, MH_RenderDriver);
	if (!f)
		return NULL;

	if (!D3D12Drv_InitFence(dev->dev, f, createSignaled)) {
		if (f->event)
			CloseHandle(f->event);

		if (f->fence)
			ID3D12Fence_Release(f->fence);

		Sys_Free(f);
		return NULL;
	}

	return f;
}

bool
D3D12Drv_InitFence(ID3D12Device5 *dev, struct NeFence *f, bool signaled)
{
	if (!SUCCEEDED(ID3D12Device5_CreateFence(dev, f->value, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, &f->fence)))
		return false;

	f->event = CreateEvent(NULL, FALSE, signaled, NULL);
	return f->event != NULL;
}

void
D3D12_SignalFence(struct NeRenderDevice *dev, struct NeFence *f)
{
//	++ctx->executeFence.value;
//	ID3D12CommandQueue_Signal(ctx->neDev->computeQueue, ctx->executeFence.fence, ctx->executeFence.value);
}

bool
D3D12_WaitForFence(struct NeRenderDevice *dev, struct NeFence *f, uint64_t timeout)
{
	DWORD rc = WAIT_FAILED;
	if (ID3D12Fence_GetCompletedValue(f->fence) < f->value) {
		ID3D12Fence_SetEventOnCompletion(f->fence, f->value, f->event);
		rc = WaitForSingleObject(f->event, timeout);
	}
	return rc == WAIT_OBJECT_0;
}

void
D3D12Drv_TermFence(struct NeFence *f)
{
	ID3D12Fence_Release(f->fence);
	CloseHandle(f->event);
}

void
D3D12_DestroyFence(struct NeRenderDevice *dev, struct NeFence *f)
{
	D3D12Drv_TermFence(f);
	Sys_Free(f);
}
