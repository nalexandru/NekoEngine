#ifndef _NE_RENDER_BACKEND_H_
#define _NE_RENDER_BACKEND_H_

#include <Render/Types.h>

extern const char *Re_backendName;

bool Re_InitBackend(void);
void Re_TermBackend(void);

bool Re_EnumerateDevices(uint32_t *count, struct NeRenderDeviceInfo *devices);
struct NeRenderDevice *Re_CreateDevice(struct NeRenderDeviceInfo *info);
void Re_DestroyDevice(struct NeRenderDevice *dev);

void Re_AppendXrExtensions(struct NeArray *a);

// Buffer
struct NeBuffer *Re_BkCreateBuffer(const struct NeBufferDesc *desc, uint16_t location);
struct NeBuffer *Re_BkCreateTransientBuffer(const struct NeBufferDesc *desc, NeBufferHandle location, uint64_t offset, uint64_t *size);
void Re_BkUpdateBuffer(struct NeBuffer *buff, uint64_t offset, void *data, uint64_t size);
void *Re_BkMapBuffer(struct NeBuffer *buff);
void Re_BkFlushBuffer(struct NeBuffer *buff, uint64_t offset, uint64_t size);
void Re_BkUnmapBuffer(struct NeBuffer *buff);
uint64_t Re_BkBufferAddress(const struct NeBuffer *buff, uint64_t offset);
void Re_BkDestroyBuffer(struct NeBuffer *buff);
void Re_BkCmdCopyBuffer(const struct NeBuffer *src, uint64_t srcOffset, struct NeBuffer *dst, uint64_t dstOffset, uint64_t size);
void Re_BkCmdCopyBufferToTexture(const struct NeBuffer *src, struct NeTexture *dst, const struct NeBufferImageCopy *bic);
void Re_BkCmdCopyTextureToBuffer(const struct NeTexture *src, struct NeBuffer *dst, const struct NeBufferImageCopy *bic);
void Re_BkCmdBindIndexBuffer(struct NeBuffer *buff, uint64_t offset, enum NeIndexType type);
void Re_BkCmdBindVertexBuffer(struct NeBuffer *buff, uint64_t offset);
void Re_BkCmdBindVertexBuffers(uint32_t count, struct NeBuffer **buffers, uint64_t *offsets, uint32_t start);

// Texture
struct NeTexture *Re_BkCreateTexture(const struct NeTextureDesc *desc, uint16_t location);
enum NeTextureLayout Re_BkTextureLayout(const struct NeTexture *tex);
void Re_BkDestroyTexture(struct NeTexture *tex);

// Pipeline
struct NePipeline *Re_BkGraphicsPipeline(const struct NeGraphicsPipelineDesc *desc);
struct NePipeline *Re_BkComputePipeline(const struct NeComputePipelineDesc *desc);
struct NePipeline *Re_BkRayTracingPipeline(struct NeShaderBindingTable *sbt, uint32_t maxDepth);
void Re_BkDestroyPipeline(struct NePipeline *pipeline);

// Direct I/O
NeDirectIOHandle Re_BkOpenFile(const char *path);
void Re_BkCloseFile(NeDirectIOHandle handle);

void Re_ScreenResized(struct NeSwapchain *sw);

// Render Interfaces

#ifdef RE_NATIVE_VULKAN
#include <vulkan/vulkan.h>

struct NeRenderInterface
{
	VkCommandBuffer(*CurrentCommandBuffer)(struct NeRenderContext *);
	
	VkSemaphore frameSemaphore;
	uint64_t(*FrameSemaphoreValue)(struct NeRenderDevice *);
	
	VkSemaphore(*SemaphoreHandle)(struct NeSemaphore *);
	uint64_t(*CurrentSemaphoreValue)(struct NeSemaphore *);
	
	VkImage(*Image)(struct NeTexture *);
	VkImageView(*ImageView)(struct NeTexture *);
	VkBuffer(*Buffer)(struct NeBuffer *);
	VkAccelerationStructureKHR(*AccelerationStructure)(struct NeAccelerationStructure *);
	VkFramebuffer(*Framebuffer)(struct NeFramebuffer *);
	VkRenderPass(*RenderPass)(struct NeRenderPassDesc *);
	VkSampler(*Sampler)(struct NeSampler *);
	VkPipeline(*Pipeline)(struct NePipeline *);
	
	VkDevice device;
	VkQueue graphicsQueue, computeQueue, transferQueue;
	uint32_t graphicsFamily, computeFamily, transferFamily;
	VkPhysicalDevice physicalDevice;
	VkPipelineCache pipelineCache;
	VkAllocationCallbacks *allocationCallbacks;
	
	VkInstance instance;
	PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
};
#endif

#ifdef RE_NATIVE_METAL
#import <Metal/Metal.h>

struct NeRenderInterface
{
	id<MTLCommandQueue>(*CommandQueue)(struct NeRenderContext *);
	id<MTLCommandBuffer>(*CurrentCommandBuffer)(struct NeRenderContext *);
	id<MTLBlitCommandEncoder>(*CurrentBlitEncoder)(struct NeRenderContext *);
	id<MTLRenderCommandEncoder>(*CurrentRenderEncoder)(struct NeRenderContext *);
	id<MTLComputeCommandEncoder>(*CurrentComputeEncoder)(struct NeRenderContext *);

	id<MTLTexture>(*Texture)(struct NeTexture *);
	id<MTLBuffer>(*Buffer)(struct NeBuffer *);
	id<MTLAccelerationStructure>(*AccelerationStructure)(struct NeAccelerationStructure *);
	const MTLRenderPassDescriptor *(*RenderPassDescriptor)(struct NeRenderPassDesc *);

	id<MTLDevice> device;
};
#endif

#endif /* _NE_RENDER_BACKEND_H_ */
