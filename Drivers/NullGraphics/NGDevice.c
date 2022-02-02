#include <stdlib.h>

#include <System/Log.h>
#include <System/Memory.h>
#include <Engine/Config.h>

#include "NullGraphicsDriver.h"

struct NeRenderDevice *
NG_CreateDevice(struct NeRenderDeviceInfo *info, struct NeRenderDeviceProcs *devProcs, struct NeRenderContextProcs *ctxProcs)
{
	struct NeRenderDevice *dev = Sys_Alloc(1, sizeof(*dev), MH_RenderDriver);

	if (!dev)
		return NULL;

	devProcs->GraphicsPipeline = NG_GraphicsPipeline;
	devProcs->ComputePipeline = NG_ComputePipeline;
	devProcs->RayTracingPipeline = NG_RayTracingPipeline;
	devProcs->DestroyPipeline = NG_DestroyPipeline;

	devProcs->AcquireNextImage = NG_AcquireNextImage;
	devProcs->Present = NG_Present;
	devProcs->SwapchainFormat = NG_SwapchainFormat;
	devProcs->SwapchainTexture = NG_SwapchainTexture;
	devProcs->SwapchainDesc = NG_SwapchainDesc;
	devProcs->ScreenResized = NG_ScreenResized;

	devProcs->CreateTexture = NG_CreateTexture;
	devProcs->TextureLayout = NG_TextureLayout;
	devProcs->DestroyTexture = NG_DestroyTexture;

	devProcs->CreateSampler = NG_CreateSampler;
	devProcs->DestroySampler = NG_DestroySampler;

	devProcs->CreateBuffer = NG_CreateBuffer;
	devProcs->UpdateBuffer = NG_UpdateBuffer;
	devProcs->DestroyBuffer = NG_DestroyBuffer;

	devProcs->CreateAccelerationStructure = NG_CreateAccelerationStructure;
	devProcs->DestroyAccelerationStructure = NG_DestroyAccelerationStructure;

	devProcs->LoadPipelineCache = NG_LoadPipelineCache;
	devProcs->SavePipelineCache = NG_SavePipelineCache;

	devProcs->CreateContext = NG_CreateContext;
	devProcs->ResetContext = NG_ResetContext;
	devProcs->DestroyContext = NG_DestroyContext;

	devProcs->CreateSurface = NG_CreateSurface;
	devProcs->DestroySurface = NG_DestroySurface;

	devProcs->CreateSwapchain = NG_CreateSwapchain;
	devProcs->DestroySwapchain = NG_DestroySwapchain;

	devProcs->CreateFramebuffer = NG_CreateFramebuffer;
	devProcs->SetAttachment = NG_SetAttachment;
	devProcs->DestroyFramebuffer = NG_DestroyFramebuffer;

	devProcs->CreateRenderPassDesc = NG_CreateRenderPassDesc;
	devProcs->DestroyRenderPassDesc = NG_DestroyRenderPassDesc;

	devProcs->ShaderModule = NG_ShaderModule;

	devProcs->Execute = NG_Execute;
	devProcs->WaitIdle = NG_WaitIdle;

	devProcs->CreateTransientBuffer = NG_CreateTransientBuffer;
	devProcs->CreateTransientTexture = NG_CreateTransientTexture;
	devProcs->InitTransientHeap = NG_InitTransientHeap;
	devProcs->ResizeTransientHeap = NG_ResizeTransientHeap;
	devProcs->TermTransientHeap = NG_TermTransientHeap;

	devProcs->MapBuffer = NG_MapBuffer;
	devProcs->FlushBuffer = NG_FlushBuffer;
	devProcs->UnmapBuffer = NG_UnmapBuffer;
	devProcs->BufferAddress = NG_BufferAddress;

	devProcs->CreateSemaphore = NG_CreateSemaphore;
	devProcs->WaitSemaphore = NG_WaitSemaphore;
	devProcs->WaitSemaphores = NG_WaitSemaphores;
	devProcs->SignalSemaphore = NG_SignalSemaphore;
	devProcs->DestroySemaphore = NG_DestroySemaphore;

	devProcs->CreateFence = NG_CreateFence;
	devProcs->SignalFence = NG_SignalFence;
	devProcs->WaitForFence = NG_WaitForFence;
	devProcs->DestroyFence = NG_DestroyFence;

	devProcs->OffsetAddress = NG_OffsetAddress;

	NG_InitContextProcs(ctxProcs);

	return dev;
}

bool
NG_Execute(struct NeRenderDevice *dev, struct NeRenderContext *ctx, bool wait)
{
	return true;
}

void
NG_WaitIdle(struct NeRenderDevice *dev)
{
}

void
NG_DestroyDevice(struct NeRenderDevice *dev)
{
	Sys_Free(dev);
}
