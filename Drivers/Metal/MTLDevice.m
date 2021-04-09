#define Handle __EngineHandle

#include <Render/Device.h>

#undef Handle

#include "MTLDriver.h"

struct RenderDevice *
MTL_CreateDevice(struct RenderDeviceInfo *info,
			  struct RenderDeviceProcs *devProcs,
			  struct RenderContextProcs *ctxProcs)
{
	devProcs->CreateContext = (struct RenderContext *(*)(struct RenderDevice *))MTL_CreateContext;
	devProcs->ResetContext = (void(*)(struct RenderDevice *, struct RenderContext *))MTL_ResetContext;
	devProcs->DestroyContext = (void(*)(struct RenderDevice *, struct RenderContext *))MTL_DestroyContext;
	
	devProcs->CreateSurface = (struct Surface *(*)(struct RenderDevice *, void *))MTL_CreateSurface;
	devProcs->DestroySurface = (void(*)(struct RenderDevice *, struct Surface *))MTL_DestroySurface;
	
	devProcs->CreateTexture = (struct Texture *(*)(struct RenderDevice *, const struct TextureCreateInfo *, uint16_t))MTL_CreateTexture;
	devProcs->DestroyTexture = (void(*)(struct RenderDevice *, struct Texture *))MTL_DestroyTexture;
	
	devProcs->CreateSampler = (struct Sampler *(*)(struct RenderDevice *, const struct SamplerDesc *))MTL_CreateSampler;
	devProcs->DestroySampler = (void(*)(struct RenderDevice *, struct Sampler *s))MTL_DestroySampler;
	
	devProcs->CreateBuffer = (struct Buffer *(*)(struct RenderDevice *, const struct BufferCreateInfo *, uint16_t))MTL_CreateBuffer;
	devProcs->UpdateBuffer = (void(*)(struct RenderDevice *, struct Buffer *, uint64_t, void *, uint64_t))MTL_UpdateBuffer;
	devProcs->DestroyBuffer = (void(*)(struct RenderDevice *, struct Buffer *))MTL_DestroyBuffer;
	
	devProcs->CreateAccelerationStructure = (struct AccelerationStructure *(*)(struct RenderDevice *, const struct AccelerationStructureCreateInfo *))MTL_CreateAccelerationStructure;
	devProcs->DestroyAccelerationStructure = (void(*)(struct RenderDevice *, struct AccelerationStructure *))MTL_DestroyAccelerationStructure;
	
	devProcs->CreateSwapchain = (struct Swapchain *(*)(struct RenderDevice *, struct Surface *))MTL_CreateSwapchain;
	devProcs->DestroySwapchain = (void(*)(struct RenderDevice *, struct Swapchain *))MTL_DestroySwapchain;
	devProcs->AcquireNextImage = (void *(*)(struct RenderDevice *, struct Swapchain *))MTL_AcquireNextImage;
	devProcs->Present = (bool(*)(struct RenderDevice *, struct RenderContext *, struct Swapchain *, void *))MTL_Present;
	devProcs->SwapchainTexture = (struct Texture *(*)(struct Swapchain *, void *))MTL_SwapchainTexture;
	devProcs->SwapchainFormat = (enum TextureFormat(*)(struct Swapchain *))MTL_SwapchainFormat;
	devProcs->ScreenResized = (void(*)(struct RenderDevice *, struct Swapchain *))MTL_ScreenResized;
	
	devProcs->GraphicsPipeline = (struct Pipeline *(*)(struct RenderDevice *, const struct GraphicsPipelineDesc *desc))MTL_GraphicsPipeline;
	devProcs->ComputePipeline = (struct Pipeline *(*)(struct RenderDevice *, struct Shader *))MTL_ComputePipeline;
	devProcs->RayTracingPipeline = (struct Pipeline *(*)(struct RenderDevice *, struct ShaderBindingTable *, uint32_t))MTL_RayTracingPipeline;
	devProcs->LoadPipelineCache = (void(*)(struct RenderDevice *))MTL_LoadPipelineCache;
	devProcs->SavePipelineCache = (void(*)(struct RenderDevice *))MTL_LoadPipelineCache;
	
	devProcs->CreateRenderPass = (struct RenderPass *(*)(struct RenderDevice *, const struct RenderPassDesc *))MTL_CreateRenderPass;
	devProcs->DestroyRenderPass = (void(*)(struct RenderDevice *, struct RenderPass *))MTL_DestroyRenderPass;
	
	devProcs->CreateFramebuffer = (struct Framebuffer *(*)(struct RenderDevice *, const struct FramebufferDesc *))MTL_CreateFramebuffer;
	devProcs->SetAttachment = (void(*)(struct Framebuffer *, uint32_t, struct Texture *))MTL_SetAttachment;
	devProcs->DestroyFramebuffer = (void(*)(struct RenderDevice *, struct Framebuffer *))MTL_DestroyFramebuffer;
	
	devProcs->Execute = (bool(*)(struct RenderDevice *, struct RenderContext *, bool))MTL_Execute;
	devProcs->WaitIdle = (void(*)(struct RenderDevice *))MTL_WaitIdle;
	
	devProcs->ShaderModule = (void *(*)(struct RenderDevice *, const char *))MTL_ShaderModule;
	
	devProcs->CreateTransientBuffer = (struct Buffer *(*)(struct RenderDevice *, const struct BufferCreateInfo *, uint64_t))MTL_CreateTransientBuffer;
	devProcs->CreateTransientTexture = (struct Texture *(*)(struct RenderDevice *, const struct TextureCreateInfo *, uint64_t))MTL_CreateTransientTexture;
	
	devProcs->InitTransientHeap = (bool(*)(struct RenderDevice *, uint64_t))MTL_InitTransientHeap;
	devProcs->ResizeTransientHeap = (bool(*)(struct RenderDevice *, uint64_t))MTL_ResizeTransientHeap;
	devProcs->TermTransientHeap = (void(*)(struct RenderDevice *))MTL_TermTransientHeap;
	
	MTL_InitLibrary((id<MTLDevice>)info->private);
	MTL_InitContextProcs(ctxProcs);
	MTL_InitArgumentBuffer((id<MTLDevice>)info->private);
	
	return info->private;
}

bool MTL_Execute(id<MTLDevice> dev, struct RenderContext *ctx, bool wait)
{
	return false;
}

void MTL_WaitIdle(id<MTLDevice> dev)
{
	//
}

void
MTL_DestroyDevice(id<MTLDevice> dev)
{
	MTL_TermArgumentBuffer(dev);
	MTL_TermLibrary();
	[dev release];
}
