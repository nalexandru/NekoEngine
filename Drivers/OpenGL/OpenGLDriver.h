#ifndef _OPENGL_DRIVER_H_
#define _OPENGL_DRIVER_H_

#include <Render/Types.h>
#include <Render/Render.h>
#include <Render/Driver/RayTracing.h>
#include <Runtime/Array.h>

#include "glad.h"

#define GLDRV_MOD	L"OpenGLDriver"

struct NeRenderDevice
{
	void *context;

	//VkDescriptorSet descriptorSet;

/*	uint32_t graphicsFamily, computeFamily, transferFamily;
	VkPhysicalDevice physDev;
	VkPhysicalDeviceProperties physDevProps;
	VkPhysicalDeviceMemoryProperties physDevMemProps;
	VkSemaphore frameSemaphore;
	uint64_t semaphoreValue;
	uint64_t *frameValues;
	VkCommandPool driverTransferPool;
	VkFence driverTransferFence;

	VkDescriptorSetLayout setLayout;
	VkDescriptorPool descriptorPool;

	VkDescriptorSetLayout iaSetLayout;
	VkDescriptorPool iaDescriptorPool[RE_NUM_FRAMES];*/
};

struct NeRenderContext
{
	struct NePipeline *boundPipeline;
/*	VkCommandPool *graphicsPools, *xferPools, *computePools;
	struct Array *graphicsCmdBuffers, *secondaryCmdBuffers, *xferCmdBuffers, *computeCmdBuffers;
	VkFence executeFence;
	VkDevice vkDev;
	VkDescriptorSet descriptorSet, iaSet;
	struct NeRenderDevice *neDev;
	uint32_t lastSubmittedXfer, lastSubmittedCompute;
	struct
	{
		struct Array graphics, compute, xfer;
	} queued;*/
};

struct NeSwapchain
{
	void *a;
};

struct NeBuffer
{
	GLuint id;
	GLuint64EXT addr;
	bool cpuVisible;
};

struct NeTexture
{
	GLuint id;
	GLuint64EXT addr;
	GLenum target;
	bool transient;
};

struct NeSampler
{
	void *a;
};

struct NeFramebuffer
{
	GLuint id;
//	VkFramebuffer fb;
//	VkImageView *attachments;
	GLuint attachments[9];
	uint32_t width, height, layers, attachmentCount;
};

struct NeRenderPassDesc
{
//	VkRenderPass rp;
	uint32_t clearValueCount;
//	VkClearValue *clearValues;
	uint32_t inputAttachments;
};

struct NePipeline
{
	GLuint program;

	union {
	struct {
		GLenum topology, polygonMode, cullMode, frontFace;
		bool discard, depthBias, depthClamp, depthTest, depthWrite, depthBounds,
				sampleShading, alphaToCoverage, alphaToOne, stencilTest;
		GLenum depthOp;
	} graphics;
	struct {
		void *a;
	} compute;
	};
};

struct NeSemaphore
{
//	VkSemaphore sem;
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

struct Vkd_SubmitInfo
{
//	VkSemaphore wait, signal;
	uint64_t waitValue, signalValue;
//	VkCommandBuffer cmdBuffer;
};

// Debug
//void GLDrv_DebugCallback(void *userData, const char *context, DkResult result, const char *message);

// Device
struct NeRenderDevice *GL_CreateDevice(struct NeRenderDeviceInfo *info, struct NeRenderDeviceProcs *devProcs, struct NeRenderContextProcs *ctxProcs);
bool GL_Execute(struct NeRenderDevice *dev, struct NeRenderContext *ctx, bool wait);
void GL_WaitIdle(struct NeRenderDevice *dev);
void GL_DestroyDevice(struct NeRenderDevice *dev);

// NePipeline
struct NePipeline *GL_GraphicsPipeline(struct NeRenderDevice *dev, const struct NeGraphicsPipelineDesc *desc);
struct NePipeline *GL_ComputePipeline(struct NeRenderDevice *dev, const struct NeComputePipelineDesc *desc);
struct NePipeline *GL_RayTracingPipeline(struct NeRenderDevice *dev, struct NeShaderBindingTable *sbt, uint32_t maxDepth);
void GL_LoadPipelineCache(struct NeRenderDevice *dev);
void GL_SavePipelineCache(struct NeRenderDevice *dev);
void GL_DestroyPipeline(struct NeRenderDevice *dev, struct NePipeline *pipeline);

// NeSwapchain
struct NeSwapchain *GL_CreateSwapchain(struct NeRenderDevice *dev, struct NeSurface *surface, bool verticalSync);
void GL_DestroySwapchain(struct NeRenderDevice *dev, struct NeSwapchain *sw);
void *GL_AcquireNextImage(struct NeRenderDevice *, struct NeSwapchain *sw);
bool GL_Present(struct NeRenderDevice *dev, struct NeRenderContext *ctx, struct NeSwapchain *sw, void *image, struct NeSemaphore *wait);
enum NeTextureFormat GL_SwapchainFormat(struct NeSwapchain *sw);
struct NeTexture *GL_SwapchainTexture(struct NeSwapchain *sw, void *image);
void GL_SwapchainDesc(struct NeSwapchain *sw, struct NeFramebufferAttachmentDesc *desc);
void GL_ScreenResized(struct NeRenderDevice *dev, struct NeSwapchain *sw);

// Surface
struct NeSurface *GL_CreateSurface(struct NeRenderDevice *dev, void *window);
void GL_DestroySurface(struct NeRenderDevice *dev, struct NeSurface *surface);

// Context
struct NeRenderContext *GL_CreateContext(struct NeRenderDevice *dev);
void GL_ResetContext(struct NeRenderDevice *dev, struct NeRenderContext *ctx);
void GL_DestroyContext(struct NeRenderDevice *dev, struct NeRenderContext *ctx);

// Texture
struct NeTexture *GL_CreateTexture(struct NeRenderDevice *dev, const struct NeTextureDesc *desc, uint16_t location);
enum NeTextureLayout GL_TextureLayout(const struct NeTexture *tex);
void GL_DestroyTexture(struct NeRenderDevice *dev, struct NeTexture *tex);

// Buffer
struct NeBuffer *GL_CreateBuffer(struct NeRenderDevice *dev, const struct NeBufferDesc *desc, uint16_t location);
void GL_UpdateBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff, uint64_t offset, void *data, uint64_t size);
void *GL_MapBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff);
void GL_FlushBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff, uint64_t offset, uint64_t size);
void GL_UnmapBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff);
uint64_t GL_BufferAddress(struct NeRenderDevice *dev, const struct NeBuffer *buff, uint64_t offset);
uint64_t GL_OffsetAddress(uint64_t addr, uint64_t offset);
void GL_DestroyBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff);

// Acceleration Structure
struct NeAccelerationStructure *GL_CreateAccelerationStructure(struct NeRenderDevice *dev, const struct NeAccelerationStructureCreateInfo *asci);
uint64_t GL_AccelerationStructureHandle(struct NeRenderDevice *dev, const struct NeAccelerationStructure *as);
void GL_DestroyAccelerationStructure(struct NeRenderDevice *dev, struct NeAccelerationStructure *as);

// NeFramebuffer
struct NeFramebuffer *GL_CreateFramebuffer(struct NeRenderDevice *dev, const struct NeFramebufferDesc *desc);
void GL_SetAttachment(struct NeFramebuffer *fb, uint32_t pos, struct NeTexture *tex);
void GL_DestroyFramebuffer(struct NeRenderDevice *dev, struct NeFramebuffer *fb);

// Render Pass
struct NeRenderPassDesc *GL_CreateRenderPassDesc(struct NeRenderDevice *dev, const struct NeAttachmentDesc *attachments, uint32_t count,
												const struct NeAttachmentDesc *depthAttachment, const struct NeAttachmentDesc *inputAttachments, uint32_t inputCount);
void GL_DestroyRenderPassDesc(struct NeRenderDevice *dev, struct NeRenderPassDesc *fb);

// Descriptor Set
/*bool GL_CreateDescriptorSet(struct NeRenderDevice *dev);
VkDescriptorSet GL_AllocateIADescriptorSet(struct NeRenderDevice *dev);
void GL_SetSampler(struct NeRenderDevice *dev, uint16_t location, VkSampler sampler);
void GL_SetTexture(struct NeRenderDevice *dev, uint16_t location, VkImageView imageView);
void GL_SetInputAttachment(struct NeRenderDevice *dev, VkDescriptorSet set, uint16_t location, VkImageView imageView);
void GL_TermDescriptorSet(struct NeRenderDevice *dev);*/

// Shader
void *GL_ShaderModule(struct NeRenderDevice *dev, const char *name);
bool GL_LoadShaders(void);
void GL_UnloadShaders(void);

// NeSampler
struct NeSampler *GL_CreateSampler(struct NeRenderDevice *dev, const struct NeSamplerDesc *desc);
void GL_DestroySampler(struct NeRenderDevice *dev, struct NeSampler *s);

// TransientResources
struct NeTexture *GL_CreateTransientTexture(struct NeRenderDevice *dev, const struct NeTextureDesc *desc, uint16_t location, uint64_t offset, uint64_t *size);
struct NeBuffer *GL_CreateTransientBuffer(struct NeRenderDevice *dev, const struct NeBufferDesc *desc, uint16_t location, uint64_t offset, uint64_t *size);
bool GL_InitTransientHeap(struct NeRenderDevice *dev, uint64_t size);
bool GL_ResizeTransientHeap(struct NeRenderDevice *dev, uint64_t size);
void GL_TermTransientHeap(struct NeRenderDevice *dev);

// Synchronization
struct NeSemaphore *GL_CreateSemaphore(struct NeRenderDevice *dev);
bool GL_WaitSemaphore(struct NeRenderDevice *dev, struct NeSemaphore *s, uint64_t value, uint64_t timeout);
bool GL_WaitSemaphores(struct NeRenderDevice *dev, uint32_t count, struct NeSemaphore *s, uint64_t *values, uint64_t timeout);
bool GL_SignalSemaphore(struct NeRenderDevice *dev, struct NeSemaphore *s, uint64_t value);
void GL_DestroySemaphore(struct NeRenderDevice *dev, struct NeSemaphore *s);

struct NeFence *GL_CreateFence(struct NeRenderDevice *dev, bool createSignaled);
void GL_SignalFence(struct NeRenderDevice *dev, struct NeFence *f);
bool GL_WaitForFence(struct NeRenderDevice *dev, struct NeFence *f, uint64_t timeout);
void GL_DestroyFence(struct NeRenderDevice *dev, struct NeFence *f);

// Platform
bool GL_InitContext(void);
void GL_HardwareInfo(struct NeRenderDeviceInfo *info);
void GL_EnableVerticalSync(bool enable);
void GL_SwapBuffers(void);
void *GL_GetProcAddress(const char *name);
void GL_TermContext(void);

// Utility functions
void GL_InitContextProcs(struct NeRenderContextProcs *p);

static inline GLenum
NeToGLInternalFormat(enum TextureFormat fmt)
{
	switch (fmt) {
	case TF_R8G8B8A8_UNORM: return GL_RGBA8;
	case TF_R8G8B8A8_SRGB: return GL_RGBA8_SNORM;
	case TF_B8G8R8A8_UNORM: return GL_BGRA8_EXT;
	case TF_B8G8R8A8_SRGB: return GL_BGRA8_EXT;
	case TF_R16G16B16A16_SFLOAT: return GL_RGBA16F;
	case TF_R32G32B32A32_SFLOAT: return GL_RGBA32F;
	case TF_A2R10G10B10_UNORM: return GL_RGBA12;
	case TF_D32_SFLOAT: return GL_DEPTH_COMPONENT32F;
	case TF_D24_STENCIL8: return GL_DEPTH24_STENCIL8;
	case TF_R8G8_UNORM: return GL_RG8;
	case TF_R8_UNORM: return GL_R8;
/*	case TF_BC5_UNORM: return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
	case TF_BC5_SNORM: return VK_FORMAT_BC5_SNORM_BLOCK;
	case TF_BC6H_UF16: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
	case TF_BC6H_SF16: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
	case TF_BC7_UNORM: return VK_FORMAT_BC7_UNORM_BLOCK;
	case TF_BC7_SRGB: return VK_FORMAT_BC7_SRGB_BLOCK;
	case TF_ETC2_R8G8B8_UNORM: return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
	case TF_ETC2_R8G8B8_SRGB: return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
	case TF_ETC2_R8G8B8A1_UNORM: return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
	case TF_ETC2_R8G8B8A1_SRGB: return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
	case TF_EAC_R11_UNORM: return VK_FORMAT_EAC_R11_UNORM_BLOCK;
	case TF_EAC_R11_SNORM: return VK_FORMAT_EAC_R11_SNORM_BLOCK;
	case TF_EAC_R11G11_UNORM: return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
	case TF_EAC_R11G11_SNORM: return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;*/
	default: return GL_NONE;
	}
}

/*static inline VkImageAspectFlags
NeFormatAspect(enum TextureFormat fmt)
{
	switch (fmt) {
	case TF_D32_SFLOAT: return VK_IMAGE_ASPECT_DEPTH_BIT;
	case TF_D24_STENCIL8: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	default: return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}

static inline VkFormat
NeToVkTextureFormat(enum TextureFormat fmt)
{
	switch (fmt) {
	case TF_R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
	case TF_R8G8B8A8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
	case TF_B8G8R8A8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
	case TF_B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
	case TF_R16G16B16A16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
	case TF_R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
	case TF_A2R10G10B10_UNORM: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
	case TF_D32_SFLOAT: return VK_FORMAT_D32_SFLOAT;
	case TF_D24_STENCIL8: return VK_FORMAT_D24_UNORM_S8_UINT;
	case TF_R8G8_UNORM: return VK_FORMAT_R8G8_UNORM;
	case TF_R8_UNORM: return VK_FORMAT_R8_UNORM;
	case TF_BC5_UNORM: return VK_FORMAT_BC5_UNORM_BLOCK;
	case TF_BC5_SNORM: return VK_FORMAT_BC5_SNORM_BLOCK;
	case TF_BC6H_UF16: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
	case TF_BC6H_SF16: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
	case TF_BC7_UNORM: return VK_FORMAT_BC7_UNORM_BLOCK;
	case TF_BC7_SRGB: return VK_FORMAT_BC7_SRGB_BLOCK;
	case TF_ETC2_R8G8B8_UNORM: return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
	case TF_ETC2_R8G8B8_SRGB: return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
	case TF_ETC2_R8G8B8A1_UNORM: return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
	case TF_ETC2_R8G8B8A1_SRGB: return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
	case TF_EAC_R11_UNORM: return VK_FORMAT_EAC_R11_UNORM_BLOCK;
	case TF_EAC_R11_SNORM: return VK_FORMAT_EAC_R11_SNORM_BLOCK;
	case TF_EAC_R11G11_UNORM: return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
	case TF_EAC_R11G11_SNORM: return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;
	default: return VK_FORMAT_UNDEFINED;
	}
}

static inline enum TextureFormat
VkToNeTextureFormat(VkFormat fmt)
{
	switch (fmt) {
	case VK_FORMAT_R8G8B8A8_UNORM: return TF_R8G8B8A8_UNORM;
	case VK_FORMAT_R8G8B8A8_SRGB: return TF_R8G8B8A8_SRGB;
	case VK_FORMAT_B8G8R8A8_UNORM: return TF_B8G8R8A8_UNORM;
	case VK_FORMAT_B8G8R8A8_SRGB: return TF_B8G8R8A8_SRGB;
	case VK_FORMAT_R16G16B16A16_SFLOAT: return TF_R16G16B16A16_SFLOAT;
	case VK_FORMAT_R32G32B32A32_SFLOAT: return TF_R32G32B32A32_SFLOAT;
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return TF_A2R10G10B10_UNORM;
	case VK_FORMAT_D32_SFLOAT: return TF_D32_SFLOAT;
	case VK_FORMAT_D24_UNORM_S8_UINT: return TF_D24_STENCIL8;
	case VK_FORMAT_R8G8_UNORM: return TF_R8G8_UNORM;
	case VK_FORMAT_R8_UNORM: return TF_R8_UNORM;
	case VK_FORMAT_BC5_UNORM_BLOCK: return TF_BC5_UNORM;
	case VK_FORMAT_BC5_SNORM_BLOCK: return TF_BC5_SNORM;
	case VK_FORMAT_BC6H_UFLOAT_BLOCK: return TF_BC6H_UF16;
	case VK_FORMAT_BC6H_SFLOAT_BLOCK: return TF_BC6H_SF16;
	case VK_FORMAT_BC7_UNORM_BLOCK: return TF_BC7_UNORM;
	case VK_FORMAT_BC7_SRGB_BLOCK: return TF_BC7_SRGB;
	case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK: return TF_ETC2_R8G8B8_UNORM;
	case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK: return TF_ETC2_R8G8B8_SRGB;
	case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK: return TF_ETC2_R8G8B8A1_UNORM;
	case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: return TF_ETC2_R8G8B8A1_SRGB;
	case VK_FORMAT_EAC_R11_UNORM_BLOCK: return TF_EAC_R11_UNORM;
	case VK_FORMAT_EAC_R11_SNORM_BLOCK: return TF_EAC_R11_SNORM;
	case VK_FORMAT_EAC_R11G11_UNORM_BLOCK: return TF_EAC_R11G11_UNORM;
	case VK_FORMAT_EAC_R11G11_SNORM_BLOCK: return TF_EAC_R11G11_SNORM;
	default: return TF_INVALID;
	}
}

static inline VkMemoryPropertyFlags
NeToVkMemoryProperties(enum GPUMemoryType type)
{
	switch (type) {
	case MT_CPU_READ: return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	case MT_CPU_WRITE: return VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	case MT_CPU_COHERENT: return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	case MT_GPU_LOCAL:
	default: return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	}
}

static inline VkCommandBuffer
Vkd_AllocateCmdBuffer(VkDevice dev, VkCommandBufferLevel level, VkCommandPool pool, struct Array *freeList)
{
	VkCommandBuffer cmdBuff;
	VkCommandBufferAllocateInfo ai =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = pool,
		.level = level,
		.commandBufferCount = 1
	};
	if (vkAllocateCommandBuffers(dev, &ai, &cmdBuff) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	Rt_ArrayAddPtr(freeList, cmdBuff);

	return cmdBuff;
}

static inline uint32_t
Vkd_MemoryTypeIndex(const struct NeRenderDevice *dev, uint32_t filter, VkMemoryPropertyFlags flags)
{
	for (uint32_t i = 0; i < dev->physDevMemProps.memoryTypeCount; ++i)
		if (/(filter & (1 << i)) &&/ ((dev->physDevMemProps.memoryTypes[i].propertyFlags & flags) == flags))
			return i;
	return 0;
}

static inline VkCommandBuffer
Vkd_TransferCmdBuffer(struct NeRenderDevice *dev)
{
	VkCommandBuffer cb;

	VkCommandBufferAllocateInfo cbai =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = dev->driverTransferPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1
	};
	vkAllocateCommandBuffers(dev->dev, &cbai, &cb);

	VkCommandBufferBeginInfo cbbi =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};
	vkBeginCommandBuffer(cb, &cbbi);

	return cb;
}

static inline void
Vkd_ExecuteCmdBuffer(struct NeRenderDevice *dev, VkCommandBuffer cb)
{
	vkEndCommandBuffer(cb);

	VkSubmitInfo si =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &cb,
	};

	vkQueueSubmit(dev->transferQueue, 1, &si, dev->driverTransferFence);
	vkWaitForFences(dev->dev, 1, &dev->driverTransferFence, VK_TRUE, UINT64_MAX);
	vkResetFences(dev->dev, 1, &dev->driverTransferFence);

	vkFreeCommandBuffers(dev->dev, dev->driverTransferPool, 1, &cb);
}

static inline void
Vkd_TransitionImageLayoutRange(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange *range)
{
	VkPipelineStageFlagBits src = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, dst = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkImageMemoryBarrier barrier =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = oldLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange = *range
	};

	switch (barrier.oldLayout) {
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		src = VK_PIPELINE_STAGE_HOST_BIT;
	break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		src = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		src = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		src = VK_PIPELINE_STAGE_TRANSFER_BIT;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		src = VK_PIPELINE_STAGE_TRANSFER_BIT;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		src = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	break;
	default:
		barrier.srcAccessMask = 0;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	break;
	}

	switch (barrier.newLayout) {
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dst = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dst = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		dst = VK_PIPELINE_STAGE_TRANSFER_BIT;
	break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		dst = VK_PIPELINE_STAGE_TRANSFER_BIT;
	break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		dst = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	default:
		barrier.dstAccessMask = 0;
	break;
	}

	vkCmdPipelineBarrier(cmdBuffer, src, dst, 0, 0, NULL, 0, NULL, 1, &barrier);
}

static inline void
Vkd_TransitionImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkImageSubresourceRange range =
	{
		.baseMipLevel = 0,
		.baseArrayLayer = 0,
		.levelCount = 1,
		.layerCount = 1
	};
	Vkd_TransitionImageLayoutRange(cmdBuffer, image, oldLayout, newLayout, &range);
}

static inline VkImageLayout
NeToVkImageLayout(enum TextureLayout tl)
{
	switch (tl) {
		case TL_COLOR_ATTACHMENT: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		case TL_DEPTH_STENCIL_ATTACHMENT: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		case TL_DEPTH_STENCIL_READ_ONLY_ATTACHMENT: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		case TL_DEPTH_ATTACHMENT: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		case TL_DEPTH_READ_ONLY_ATTACHMENT: return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
		case TL_STENCIL_ATTACHMENT: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		case TL_TRANSFER_SRC: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		case TL_TRANSFER_DST: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		case TL_SHADER_READ_ONLY: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case TL_PRESENT_SRC: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		case TL_UNKNOWN:
		default: return VK_IMAGE_LAYOUT_UNDEFINED;
	}
}

static inline enum TextureLayout
VkToNeImageLayout(VkImageLayout il)
{
	switch (il) {
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return TL_COLOR_ATTACHMENT;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return TL_DEPTH_STENCIL_ATTACHMENT;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: return TL_DEPTH_STENCIL_READ_ONLY_ATTACHMENT;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return TL_TRANSFER_SRC;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return TL_TRANSFER_DST;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return TL_SHADER_READ_ONLY;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return TL_PRESENT_SRC;
		case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL: return TL_DEPTH_READ_ONLY_ATTACHMENT;
		default: return TL_UNKNOWN;
	}
}*/

#endif /* _OPENGL_DRIVER_H_ */
