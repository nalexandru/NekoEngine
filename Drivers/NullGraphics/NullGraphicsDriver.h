#ifndef _NULL_GRAPHICS_DRIVER_H_
#define _NULL_GRAPHICS_DRIVER_H_

#include <Render/Types.h>
#include <Render/Render.h>
#include <Render/Driver/RayTracing.h>
#include <Runtime/Array.h>
#include <System/Log.h>

#define NGDRV_MOD	"NullGraphicsDriver"

struct NeRenderDevice
{
	void *a;
};

struct NeRenderContext
{
	void *a;
};

struct NeSwapchain
{
	void *a;
};

struct NeBuffer
{
	uint64_t size;
	void *ptr;
};

struct NeTexture
{
	void *t;
};

struct NeSampler
{
	void *s;
};

struct NeFramebuffer
{
	uint32_t width, height, layers, attachmentCount;
};

struct NeRenderPassDesc
{
	uint32_t clearValueCount;
	uint32_t inputAttachments;
};

struct NePipeline
{
	void *p;
};

struct NeSemaphore
{
	uint64_t value;
};

struct NeFence
{
	void *f;
};

struct NeSurface
{
	void *s;
};

// Device
struct NeRenderDevice *NG_CreateDevice(struct NeRenderDeviceInfo *info, struct NeRenderDeviceProcs *devProcs, struct NeRenderContextProcs *ctxProcs);
bool NG_Execute(struct NeRenderDevice *dev, struct NeRenderContext *ctx, bool wait);
void NG_WaitIdle(struct NeRenderDevice *dev);
void NG_DestroyDevice(struct NeRenderDevice *dev);

// NePipeline
struct NePipeline *NG_GraphicsPipeline(struct NeRenderDevice *dev, const struct NeGraphicsPipelineDesc *desc);
struct NePipeline *NG_ComputePipeline(struct NeRenderDevice *dev, const struct NeComputePipelineDesc *desc);
struct NePipeline *NG_RayTracingPipeline(struct NeRenderDevice *dev, struct NeShaderBindingTable *sbt, uint32_t maxDepth);
void NG_LoadPipelineCache(struct NeRenderDevice *dev);
void NG_SavePipelineCache(struct NeRenderDevice *dev);
void NG_DestroyPipeline(struct NeRenderDevice *dev, struct NePipeline *pipeline);

// NeSwapchain
struct NeSwapchain *NG_CreateSwapchain(struct NeRenderDevice *dev, struct NeSurface *surface, bool verticalSync);
void NG_DestroySwapchain(struct NeRenderDevice *dev, struct NeSwapchain *sw);
void *NG_AcquireNextImage(struct NeRenderDevice *, struct NeSwapchain *sw);
bool NG_Present(struct NeRenderDevice *dev, struct NeRenderContext *ctx, struct NeSwapchain *sw, void *image, struct NeSemaphore *wait);
enum NeTextureFormat NG_SwapchainFormat(struct NeSwapchain *sw);
struct NeTexture *NG_SwapchainTexture(struct NeSwapchain *sw, void *image);
void NG_SwapchainDesc(struct NeSwapchain *sw, struct NeFramebufferAttachmentDesc *desc);
void NG_ScreenResized(struct NeRenderDevice *dev, struct NeSwapchain *sw);

// Surface
struct NeSurface *NG_CreateSurface(struct NeRenderDevice *dev, void *window);
void NG_DestroySurface(struct NeRenderDevice *dev, struct NeSurface *surface);

// Context
struct NeRenderContext *NG_CreateContext(struct NeRenderDevice *dev);
void NG_ResetContext(struct NeRenderDevice *dev, struct NeRenderContext *ctx);
void NG_DestroyContext(struct NeRenderDevice *dev, struct NeRenderContext *ctx);

// Texture
struct NeTexture *NG_CreateTexture(struct NeRenderDevice *dev, const struct NeTextureDesc *desc, uint16_t location);
enum NeTextureLayout NG_TextureLayout(const struct NeTexture *tex);
void NG_DestroyTexture(struct NeRenderDevice *dev, struct NeTexture *tex);

// Buffer
struct NeBuffer *NG_CreateBuffer(struct NeRenderDevice *dev, const struct NeBufferDesc *desc, uint16_t location);
void NG_UpdateBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff, uint64_t offset, void *data, uint64_t size);
void *NG_MapBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff);
void NG_FlushBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff, uint64_t offset, uint64_t size);
void NG_UnmapBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff);
uint64_t NG_BufferAddress(struct NeRenderDevice *dev, const struct NeBuffer *buff, uint64_t offset);
uint64_t NG_OffsetAddress(uint64_t addr, uint64_t offset);
void NG_DestroyBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff);

// Acceleration Structure
struct NeAccelerationStructure *NG_CreateAccelerationStructure(struct NeRenderDevice *dev, const struct NeAccelerationStructureCreateInfo *asci);
uint64_t NG_AccelerationStructureHandle(struct NeRenderDevice *dev, const struct NeAccelerationStructure *as);
void NG_DestroyAccelerationStructure(struct NeRenderDevice *dev, struct NeAccelerationStructure *as);

// NeFramebuffer
struct NeFramebuffer *NG_CreateFramebuffer(struct NeRenderDevice *dev, const struct NeFramebufferDesc *desc);
void NG_SetAttachment(struct NeFramebuffer *fb, uint32_t pos, struct NeTexture *tex);
void NG_DestroyFramebuffer(struct NeRenderDevice *dev, struct NeFramebuffer *fb);

// Render Pass
struct NeRenderPassDesc *NG_CreateRenderPassDesc(struct NeRenderDevice *dev, const struct NeAttachmentDesc *attachments, uint32_t count,
												const struct NeAttachmentDesc *depthAttachment, const struct NeAttachmentDesc *inputAttachments, uint32_t inputCount);
void NG_DestroyRenderPassDesc(struct NeRenderDevice *dev, struct NeRenderPassDesc *fb);

// Shader
void *NG_ShaderModule(struct NeRenderDevice *dev, const char *name);

// NeSampler
struct NeSampler *NG_CreateSampler(struct NeRenderDevice *dev, const struct NeSamplerDesc *desc);
void NG_DestroySampler(struct NeRenderDevice *dev, struct NeSampler *s);

// TransientResources
struct NeTexture *NG_CreateTransientTexture(struct NeRenderDevice *dev, const struct NeTextureDesc *desc, uint16_t location, uint64_t offset, uint64_t *size);
struct NeBuffer *NG_CreateTransientBuffer(struct NeRenderDevice *dev, const struct NeBufferDesc *desc, uint16_t location, uint64_t offset, uint64_t *size);
bool NG_InitTransientHeap(struct NeRenderDevice *dev, uint64_t size);
bool NG_ResizeTransientHeap(struct NeRenderDevice *dev, uint64_t size);
void NG_TermTransientHeap(struct NeRenderDevice *dev);

// Synchronization
struct NeSemaphore *NG_CreateSemaphore(struct NeRenderDevice *dev);
bool NG_WaitSemaphore(struct NeRenderDevice *dev, struct NeSemaphore *s, uint64_t value, uint64_t timeout);
bool NG_WaitSemaphores(struct NeRenderDevice *dev, uint32_t count, struct NeSemaphore *s, uint64_t *values, uint64_t timeout);
bool NG_SignalSemaphore(struct NeRenderDevice *dev, struct NeSemaphore *s, uint64_t value);
void NG_DestroySemaphore(struct NeRenderDevice *dev, struct NeSemaphore *s);

struct NeFence *NG_CreateFence(struct NeRenderDevice *dev, bool createSignaled);
void NG_SignalFence(struct NeRenderDevice *dev, struct NeFence *f);
bool NG_WaitForFence(struct NeRenderDevice *dev, struct NeFence *f, uint64_t timeout);
void NG_DestroyFence(struct NeRenderDevice *dev, struct NeFence *f);

// Utility functions
void NG_InitContextProcs(struct NeRenderContextProcs *p);

#endif /* _NULL_GRAPHICS_DRIVER_H_ */
