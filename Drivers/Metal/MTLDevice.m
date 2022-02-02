#include "MTLDriver.h"

struct NeRenderDevice *
MTL_CreateDevice(struct NeRenderDeviceInfo *info,
			  struct NeRenderDeviceProcs *devProcs,
			  struct NeRenderContextProcs *ctxProcs)
{
	devProcs->CreateContext = (struct NeRenderContext *(*)(struct NeRenderDevice *))MTL_CreateContext;
	devProcs->ResetContext = (void(*)(struct NeRenderDevice *, struct NeRenderContext *))MTL_ResetContext;
	devProcs->DestroyContext = (void(*)(struct NeRenderDevice *, struct NeRenderContext *))MTL_DestroyContext;
	
	devProcs->CreateSurface = (struct NeSurface *(*)(struct NeRenderDevice *, void *))MTL_CreateSurface;
	devProcs->DestroySurface = (void(*)(struct NeRenderDevice *, struct NeSurface *))MTL_DestroySurface;
	
	devProcs->CreateTexture = (struct NeTexture *(*)(struct NeRenderDevice *, const struct NeTextureDesc *, uint16_t))MTL_CreateTexture;
	devProcs->DestroyTexture = (void(*)(struct NeRenderDevice *, struct NeTexture *))MTL_DestroyTexture;
	
	devProcs->CreateSampler = (struct NeSampler *(*)(struct NeRenderDevice *, const struct NeSamplerDesc *))MTL_CreateSampler;
	devProcs->DestroySampler = (void(*)(struct NeRenderDevice *, struct NeSampler *s))MTL_DestroySampler;
	
	devProcs->CreateBuffer = (struct NeBuffer *(*)(struct NeRenderDevice *, const struct NeBufferDesc *, uint16_t))MTL_CreateBuffer;
	devProcs->UpdateBuffer = (void(*)(struct NeRenderDevice *, struct NeBuffer *, uint64_t, void *, uint64_t))MTL_UpdateBuffer;
	devProcs->MapBuffer = (void *(*)(struct NeRenderDevice *, struct NeBuffer *))MTL_MapBuffer;
	devProcs->FlushBuffer = (void (*)(struct NeRenderDevice *dev, struct NeBuffer *, uint64_t, uint64_t))MTL_FlushBuffer;
	devProcs->UnmapBuffer = (void (*)(struct NeRenderDevice *, struct NeBuffer *))MTL_UnmapBuffer;
	devProcs->BufferAddress = (uint64_t(*)(struct NeRenderDevice *, const struct NeBuffer *, uint64_t))MTL_BufferAddress;
	devProcs->DestroyBuffer = (void(*)(struct NeRenderDevice *, struct NeBuffer *))MTL_DestroyBuffer;
	
	devProcs->CreateAccelerationStructure = (struct NeAccelerationStructure *(*)(struct NeRenderDevice *, const struct NeAccelerationStructureCreateInfo *))MTL_CreateAccelerationStructure;
	devProcs->AccelerationStructureHandle = (uint64_t(*)(struct NeRenderDevice *, const struct NeAccelerationStructure *)) MTL_AccelerationStructureHandle;
	devProcs->DestroyAccelerationStructure = (void(*)(struct NeRenderDevice *, struct NeAccelerationStructure *))MTL_DestroyAccelerationStructure;
	
	devProcs->CreateSwapchain = (struct NeSwapchain *(*)(struct NeRenderDevice *, struct NeSurface *, bool))MTL_CreateSwapchain;
	devProcs->DestroySwapchain = (void(*)(struct NeRenderDevice *, struct NeSwapchain *))MTL_DestroySwapchain;
	devProcs->AcquireNextImage = (void *(*)(struct NeRenderDevice *, struct NeSwapchain *))MTL_AcquireNextImage;
	devProcs->Present = (bool(*)(struct NeRenderDevice *, struct NeRenderContext *, struct NeSwapchain *, void *, struct NeSemaphore *))MTL_Present;
	devProcs->SwapchainTexture = (struct NeTexture *(*)(struct NeSwapchain *, void *))MTL_SwapchainTexture;
	devProcs->SwapchainFormat = (enum NeTextureFormat(*)(struct NeSwapchain *))MTL_SwapchainFormat;
	devProcs->SwapchainDesc = (void(*)(struct NeSwapchain *, struct NeFramebufferAttachmentDesc *))MTL_SwapchainDesc;
	devProcs->ScreenResized = (void(*)(struct NeRenderDevice *, struct NeSwapchain *))MTL_ScreenResized;
	
	devProcs->GraphicsPipeline = (struct NePipeline *(*)(struct NeRenderDevice *, const struct NeGraphicsPipelineDesc *desc))MTL_GraphicsPipeline;
	devProcs->ComputePipeline = (struct NePipeline *(*)(struct NeRenderDevice *, const struct NeComputePipelineDesc *))MTL_ComputePipeline;
	devProcs->RayTracingPipeline = (struct NePipeline *(*)(struct NeRenderDevice *, struct NeShaderBindingTable *, uint32_t))MTL_RayTracingPipeline;
	devProcs->LoadPipelineCache = (void(*)(struct NeRenderDevice *))MTL_LoadPipelineCache;
	devProcs->SavePipelineCache = (void(*)(struct NeRenderDevice *))MTL_LoadPipelineCache;
	devProcs->DestroyPipeline = (void(*)(struct NeRenderDevice *, struct NePipeline *))MTL_DestroyPipeline;
	
	devProcs->CreateRenderPassDesc = (struct NeRenderPassDesc *(*)(struct NeRenderDevice *, const struct NeAttachmentDesc *, uint32_t, const struct NeAttachmentDesc *, const struct NeAttachmentDesc *inputAttachments, uint32_t inputCount))MTL_CreateRenderPassDesc;
	devProcs->DestroyRenderPassDesc = (void(*)(struct NeRenderDevice *, struct NeRenderPassDesc *))MTL_DestroyRenderPassDesc;
	
	devProcs->CreateFramebuffer = (struct NeFramebuffer *(*)(struct NeRenderDevice *, const struct NeFramebufferDesc *))MTL_CreateFramebuffer;
	devProcs->SetAttachment = (void(*)(struct NeFramebuffer *, uint32_t, struct NeTexture *))MTL_SetAttachment;
	devProcs->DestroyFramebuffer = (void(*)(struct NeRenderDevice *, struct NeFramebuffer *))MTL_DestroyFramebuffer;
	
	devProcs->Execute = (bool(*)(struct NeRenderDevice *, struct NeRenderContext *, bool))MTL_Execute;
	devProcs->WaitIdle = (void(*)(struct NeRenderDevice *))MTL_WaitIdle;
	
	devProcs->ShaderModule = (void *(*)(struct NeRenderDevice *, const char *))MTL_ShaderModule;
	
	devProcs->CreateTransientBuffer = (struct NeBuffer *(*)(struct NeRenderDevice *, const struct NeBufferDesc *, uint16_t, uint64_t, uint64_t *))MTL_CreateTransientBuffer;
	devProcs->CreateTransientTexture = (struct NeTexture *(*)(struct NeRenderDevice *, const struct NeTextureDesc *, uint16_t, uint64_t, uint64_t *))MTL_CreateTransientTexture;
	
	devProcs->InitTransientHeap = (bool(*)(struct NeRenderDevice *, uint64_t))MTL_InitTransientHeap;
	devProcs->ResizeTransientHeap = (bool(*)(struct NeRenderDevice *, uint64_t))MTL_ResizeTransientHeap;
	devProcs->TermTransientHeap = (void(*)(struct NeRenderDevice *))MTL_TermTransientHeap;
	
	devProcs->CreateSemaphore = (struct NeSemaphore *(*)(struct NeRenderDevice *))MTL_CreateSemaphore;
	devProcs->DestroySemaphore = (void(*)(struct NeRenderDevice *, struct NeSemaphore *))MTL_DestroySemaphore;
	
	devProcs->CreateFence = (struct NeFence *(*)(struct NeRenderDevice *, bool))MTL_CreateFence;
	devProcs->SignalFence = (void(*)(struct NeRenderDevice *dev, struct NeFence *))MTL_SignalFence;
	devProcs->WaitForFence = (bool(*)(struct NeRenderDevice *dev, struct NeFence *, uint64_t))MTL_WaitForFence;
	devProcs->DestroyFence = (void(*)(struct NeRenderDevice *, struct NeFence *))MTL_DestroyFence;

	devProcs->WaitSemaphore = (bool(*)(struct NeRenderDevice *, struct NeSemaphore *, uint64_t, uint64_t))MTL_WaitSemaphore;
	devProcs->WaitSemaphores = (bool(*)(struct NeRenderDevice *, uint32_t, struct NeSemaphore *, uint64_t *, uint64_t))MTL_WaitSemaphores;
	devProcs->SignalSemaphore = (bool(*)(struct NeRenderDevice *, struct NeSemaphore *, uint64_t))MTL_SignalSemaphore;

	devProcs->OffsetAddress = MTL_OffsetAddress;

	devProcs->CreateRenderInterface = MTL_CreateRenderInterface;
	devProcs->DestroyRenderInterface = MTL_DestroyRenderInterface;

	MTL_InitLibrary((id<MTLDevice>)info->private);
	MTL_InitContextProcs(ctxProcs);
	MTL_InitArgumentBuffer((id<MTLDevice>)info->private);
	
	return info->private;
}

bool MTL_Execute(id<MTLDevice> dev, struct NeRenderContext *ctx, bool wait)
{
	return false;
}

void MTL_WaitIdle(id<MTLDevice> dev)
{
	dispatch_semaphore_wait(MTL_frameSemaphore, DISPATCH_TIME_FOREVER);

	id<MTLEvent> event = [dev newEvent];
	uint64_t value = 0;

	for (uint32_t i = 0; i < E_JobWorkerThreads() + 1; ++i) {
		id<MTLCommandBuffer> cb = [Re_contexts[i]->queue commandBufferWithUnretainedReferences];
		[cb encodeSignalEvent: event value: ++value];
		[cb commit];
	}

	id<MTLCommandBuffer> cb = [Re_contexts[0]->queue commandBufferWithUnretainedReferences];
	[cb encodeWaitForEvent: event value: value];

	__block dispatch_semaphore_t bds = dispatch_semaphore_create(1);
	[cb addCompletedHandler:^(id<MTLCommandBuffer> _Nonnull cmdBuff) {
		dispatch_semaphore_signal(bds);
	}];
	[cb commit];

	dispatch_semaphore_wait(bds, DISPATCH_TIME_FOREVER);
}

void
MTL_DestroyDevice(id<MTLDevice> dev)
{
	MTL_TermArgumentBuffer(dev);
	MTL_TermLibrary();
	[dev release];
}
