#include "D3D12Driver.h"

struct RenderDevice *
D3D12_CreateDevice(struct RenderDeviceInfo *info, struct RenderDeviceProcs *devProcs, struct RenderContextProcs *ctxProcs)
{
	struct RenderDevice *dev = Sys_Alloc(sizeof(*dev), 1, MH_RenderDriver);

	if (FAILED(IDXGIFactory1_EnumAdapters1(D3D12_dxgiFactory, (UINT)(uint64_t)info->private, &dev->adapter)))
		goto error;

	if (FAILED(D3D12CreateDevice((IUnknown *)dev->adapter, D3D_FEATURE_LEVEL_12_0, &IID_ID3D12Device5, &dev->dev)))
		goto error;

	D3D12_COMMAND_QUEUE_DESC cqd = 
	{
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
		D3D12_COMMAND_QUEUE_FLAG_NONE,
		0
	};

	if (FAILED(ID3D12Device5_CreateCommandQueue(dev->dev, &cqd, &IID_ID3D12CommandQueue, &dev->graphicsQueue)))
		goto error;
	ID3D12CommandQueue_SetName(dev->graphicsQueue, L"Graphics Queue");
//	ID3D12CommandQueue_QueryInterface(dev->graphicsQueue, &IID_ID3D12CommandQueueDownlevel, &dev->graphicsQueueDownlevel);

	cqd.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	if (FAILED(ID3D12Device5_CreateCommandQueue(dev->dev, &cqd, &IID_ID3D12CommandQueue, &dev->copyQueue)))
		goto error;
	ID3D12CommandQueue_SetName(dev->graphicsQueue, L"Copy Queue");

	cqd.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	if (FAILED(ID3D12Device5_CreateCommandQueue(dev->dev, &cqd, &IID_ID3D12CommandQueue, &dev->computeQueue)))
		goto error;
	ID3D12CommandQueue_SetName(dev->graphicsQueue, L"Compute Queue");

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		if (FAILED(ID3D12Device5_CreateFence(dev->dev, 0, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, &dev->renderFence[i])))
			return false;

		ID3D12Fence_SetName(dev->renderFence[i], L"Render Fence");
		dev->fenceValue[i] = 0;
	}

	dev->fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!dev->fenceEvent)
		goto error;

	devProcs->GraphicsPipeline = D3D12_GraphicsPipeline;
	devProcs->ComputePipeline = D3D12_ComputePipeline;
	devProcs->RayTracingPipeline = D3D12_RayTracingPipeline;
	devProcs->DestroyPipeline = D3D12_DestroyPipeline;

	devProcs->AcquireNextImage = D3D12_AcquireNextImage;
	devProcs->Present = D3D12_Present;
	devProcs->SwapchainFormat = D3D12_SwapchainFormat;
	devProcs->SwapchainTexture = D3D12_SwapchainTexture;
	devProcs->ScreenResized = D3D12_ScreenResized;

	devProcs->CreateTexture = D3D12_CreateTexture;
	devProcs->TextureLayout = D3D12_TextureLayout;
	devProcs->DestroyTexture = D3D12_DestroyTexture;

	devProcs->CreateSampler = (struct Sampler *(*)(struct RenderDevice *, const struct SamplerDesc *))D3D12_CreateSampler;
	devProcs->DestroySampler = (void (*)(struct RenderDevice *dev, struct Sampler *))D3D12_DestroySampler;

	devProcs->CreateBuffer = D3D12_CreateBuffer;
	devProcs->UpdateBuffer = D3D12_UpdateBuffer;
	devProcs->DestroyBuffer = D3D12_DestroyBuffer;

	devProcs->CreateAccelerationStructure = D3D12_CreateAccelerationStructure;
	devProcs->DestroyAccelerationStructure = D3D12_DestroyAccelerationStructure;

	devProcs->LoadPipelineCache = D3D12_LoadPipelineCache;
	devProcs->SavePipelineCache = D3D12_SavePipelineCache;

	devProcs->CreateContext = D3D12_CreateContext;
	devProcs->ResetContext = D3D12_ResetContext;
	devProcs->DestroyContext = D3D12_DestroyContext;

	// TODO: detect UWP
	devProcs->CreateSurface = (struct Surface *(*)(struct RenderDevice *, void *))D3D12_CreateWin32Surface;
	devProcs->DestroySurface = (void (*)(struct RenderDevice *, struct Surface *))D3D12_DestroyWin32Surface;

	devProcs->CreateSwapchain = (struct Swapchain *(*)(struct RenderDevice *dev, struct Surface *, bool))D3D12_CreateSwapchain;
	devProcs->DestroySwapchain = (void(*)(struct RenderDevice *dev, struct Swapchain *))D3D12_DestroySwapchain;

	devProcs->CreateFramebuffer = D3D12_CreateFramebuffer;
	devProcs->SetAttachment = D3D12_SetAttachment;
	devProcs->DestroyFramebuffer = D3D12_DestroyFramebuffer;

	devProcs->CreateRenderPassDesc = D3D12_CreateRenderPassDesc;
	devProcs->DestroyRenderPassDesc = D3D12_DestroyRenderPassDesc;

	devProcs->ShaderModule = D3D12_ShaderModule;

	devProcs->Execute = D3D12_Execute;
	devProcs->WaitIdle = D3D12_WaitIdle;

	devProcs->CreateTransientBuffer = D3D12_CreateTransientBuffer;
	devProcs->CreateTransientTexture = D3D12_CreateTransientTexture;
	devProcs->InitTransientHeap = D3D12_InitTransientHeap;
	devProcs->ResizeTransientHeap = D3D12_ResizeTransientHeap;
	devProcs->TermTransientHeap = D3D12_TermTransientHeap;

	devProcs->MapBuffer = D3D12_MapBuffer;
	devProcs->FlushBuffer = D3D12_FlushBuffer;
	devProcs->UnmapBuffer = D3D12_UnmapBuffer;
	devProcs->BufferAddress = D3D12_BufferAddress;

	devProcs->CreateSemaphore = D3D12_CreateSemaphore;
	devProcs->DestroySemaphore = D3D12_DestroySemaphore;

	devProcs->CreateFence = D3D12_CreateFence;
	devProcs->SignalFence = D3D12_SignalFence;
	devProcs->WaitForFence = D3D12_WaitForFence;
	devProcs->DestroyFence = D3D12_DestroyFence;

	devProcs->OffsetAddress = D3D12_OffsetAddress;

	D3D12_InitContextProcs(ctxProcs);

	return dev;

error:
	if (dev) {
		for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
			if (dev->renderFence[i])
				ID3D12Fence_Release(dev->renderFence[i]);

		if (dev->computeQueue)
			ID3D12CommandQueue_Release(dev->computeQueue);
		
		if (dev->copyQueue)
			ID3D12CommandQueue_Release(dev->copyQueue);

		if (dev->graphicsQueueDownlevel)
			ID3D12CommandQueueDownlevel_Release(dev->graphicsQueueDownlevel);

		if (dev->graphicsQueue)
			ID3D12CommandQueue_Release(dev->graphicsQueue);

		if (dev->adapter)
			IDXGIAdapter1_Release(dev->adapter);
	
		Sys_Free(dev);
	}

	return NULL;
}

bool
D3D12_Execute(struct RenderDevice *dev, struct RenderContext *ctx, bool wait)
{
	return false;
}

void
D3D12_WaitIdle(struct RenderDevice *dev)
{
	for (int i = 0; i < RE_NUM_FRAMES; ++i) {
		ID3D12CommandQueue_Signal(dev->graphicsQueue, dev->renderFence[i], ++dev->fenceValue[i]);
		if (ID3D12Fence_GetCompletedValue(dev->renderFence[i]) < dev->fenceValue[i]) {
			ID3D12Fence_SetEventOnCompletion(dev->renderFence[i], dev->fenceValue[i], dev->fenceEvent);
			WaitForSingleObject(dev->fenceEvent, INFINITE);
		}
	}
	Re_frameId = 0;
}

void
D3D12_DestroyDevice(struct RenderDevice *dev)
{
	ID3D12CommandQueue_Release(dev->computeQueue);
	ID3D12CommandQueue_Release(dev->copyQueue);

	if (dev->graphicsQueueDownlevel)
		ID3D12CommandQueueDownlevel_Release(dev->graphicsQueueDownlevel);

	ID3D12CommandQueue_Release(dev->graphicsQueue);
	IDXGIAdapter1_Release(dev->adapter);
	
	Sys_Free(dev);
}
