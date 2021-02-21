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
	devProcs->DestroyContext = (void(*)(struct RenderDevice *, struct RenderContext *))MTL_DestroyContext;
	
	devProcs->CreateSurface = (void *(*)(struct RenderDevice *, void *))MTL_CreateSurface;
	devProcs->DestroySurface = (void(*)(struct RenderDevice *, void *))MTL_DestroySurface;
	
	devProcs->CreateTexture = (struct Texture *(*)(struct RenderDevice *, const struct TextureCreateInfo *))MTL_CreateTexture;
	devProcs->TextureDesc = (const struct TextureDesc *(*)(const struct Texture *))MTL_TextureDesc;
	devProcs->DestroyTexture = (void(*)(struct RenderDevice *, struct Texture *))MTL_DestroyTexture;
	
	devProcs->CreateBuffer = (struct Buffer *(*)(struct RenderDevice *, const struct BufferCreateInfo *))MTL_CreateBuffer;
	devProcs->UpdateBuffer = (void(*)(struct RenderDevice *, struct Buffer *, uint64_t, void *, uint64_t))MTL_UpdateBuffer;
	devProcs->BufferDesc = (const struct BufferDesc *(*)(const struct Buffer *))MTL_BufferDesc;
	devProcs->DestroyBuffer = (void(*)(struct RenderDevice *, struct Buffer *))MTL_DestroyBuffer;
	
	devProcs->CreateAccelerationStructure = (struct AccelerationStructure *(*)(struct RenderDevice *, const struct AccelerationStructureCreateInfo *))MTL_CreateAccelerationStructure;
	devProcs->DestroyAccelerationStructure = (void(*)(struct RenderDevice *, struct AccelerationStructure *))MTL_DestroyAccelerationStructure;
	
	devProcs->CreateSwapchain = (void *(*)(struct RenderDevice *, void *))MTL_CreateSwapchain;
	devProcs->DestroySwapchain = (void(*)(struct RenderDevice *, void *))MTL_DestroySwapchain;
	devProcs->AcquireNextImage = (void *(*)(struct RenderDevice *, void *))MTL_AcquireNextImage;
	devProcs->Present = (bool(*)(struct RenderDevice *, struct RenderContext *, void *, void *))MTL_Present;
	devProcs->SwapchainTexture = (struct Texture *(*)(void *, void *))MTL_SwapchainTexture;
	devProcs->SwapchainFormat = (enum TextureFormat(*)(void *))MTL_SwapchainFormat;
	
	devProcs->CreatePipelineLayout = (struct PipelineLayout *(*)(struct RenderDevice *, const struct PipelineLayoutDesc *))MTL_CreatePipelineLayout;
	devProcs->DestroyPipelineLayout = (void(*)(struct RenderDevice *, struct PipelineLayout *))MTL_DestroyPipelineLayout;
	devProcs->GraphicsPipeline = (struct Pipeline *(*)(struct RenderDevice *, struct Shader *, uint64_t, const struct BlendAttachmentDesc *, uint32_t))MTL_GraphicsPipeline;
	devProcs->ComputePipeline = (struct Pipeline *(*)(struct RenderDevice *, struct Shader *))MTL_ComputePipeline;
	devProcs->RayTracingPipeline = (struct Pipeline *(*)(struct RenderDevice *, struct ShaderBindingTable *))MTL_RayTracingPipeline;
	devProcs->LoadPipelineCache = (void(*)(struct RenderDevice *))MTL_LoadPipelineCache;
	devProcs->SavePipelineCache = (void(*)(struct RenderDevice *))MTL_LoadPipelineCache;
	
	devProcs->CreateRenderPass = (struct RenderPass *(*)(struct RenderDevice *, const struct RenderPassDesc *))MTL_CreateRenderPass;
	devProcs->DestroyRenderPass = (void(*)(struct RenderDevice *, struct RenderPass *))MTL_DestroyRenderPass;
	
	devProcs->CreateFramebuffer = (struct Framebuffer *(*)(struct RenderDevice *, const struct FramebufferDesc *))MTL_CreateFramebuffer;
	devProcs->SetAttachment = (void(*)(struct Framebuffer *, uint32_t, struct Texture *))MTL_SetAttachment;
	devProcs->DestroyFramebuffer = (void(*)(struct RenderDevice *, struct Framebuffer *))MTL_DestroyFramebuffer;
	
	devProcs->CreateDescriptorSetLayout = (struct DescriptorSetLayout *(*)(struct RenderDevice *, const struct DescriptorSetLayoutDesc *))MTL_CreateDescriptorSetLayout;
	devProcs->DestroyDescriptorSetLayout = (void(*)(struct RenderDevice *, struct DescriptorSetLayout *))MTL_DestroyDescriptorSetLayout;
	
	devProcs->CreateDescriptorSet = (struct DescriptorSet *(*)(struct RenderDevice *dev, const struct DescriptorSetLayout *layout))MTL_CreateDescriptorSet;
	devProcs->WriteDescriptorSet = (void(*)(struct RenderDevice *, struct DescriptorSet *, const struct DescriptorWrite *, uint32_t))MTL_WriteDescriptorSet;
	devProcs->DestroyDescriptorSet = (void(*)(struct RenderDevice *dev, struct DescriptorSet *ds))MTL_DestroyDescriptorSet;
	
	devProcs->Execute = (bool(*)(struct RenderDevice *, struct RenderContext *, bool))MTL_Execute;
	devProcs->WaitIdle = (void(*)(struct RenderDevice *))MTL_WaitIdle;
	
	MTL_InitContextProcs(ctxProcs);
	
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
	[dev release];
}
