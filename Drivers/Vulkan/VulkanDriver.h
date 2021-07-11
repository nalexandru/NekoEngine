#ifndef _VULKAN_DRIVER_H_
#define _VULKAN_DRIVER_H_

#include <Render/Types.h>
#include <Render/Render.h>
#include <Render/Driver/RayTracing.h>
#include <Runtime/Array.h>

#define VK_ENABLE_BETA_EXTENSIONS
#include "volk.h"

#define VKDRV_MOD	L"VulkanDriver"

struct RenderDevice
{
	VkDevice dev;
	VkDeviceMemory transientHeap;
	VkQueue transferQueue, graphicsQueue, computeQueue;
	VkDescriptorSet descriptorSet;

	uint32_t graphicsFamily, computeFamily, transferFamily;
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
};

struct RenderContext
{
	VkCommandBuffer cmdBuffer;
	struct Pipeline *boundPipeline;
	VkCommandPool *graphicsPools, *xferPools, *computePools;
	struct Array *graphicsCmdBuffers, *secondaryCmdBuffers, *xferCmdBuffers, *computeCmdBuffers;
	VkFence executeFence;
	VkDevice vkDev;
	VkDescriptorSet descriptorSet;
	struct RenderDevice *neDev;
	uint32_t lastSubmittedXfer, lastSubmittedCompute;
};

struct VulkanDeviceInfo
{
	VkPhysicalDevice physicalDevice;
	uint32_t graphicsFamily, computeFamily, transferFamily;
};

struct Swapchain
{
	VkSwapchainKHR sw;
	VkSemaphore frameStart[RE_NUM_FRAMES], frameEnd[RE_NUM_FRAMES];
	VkImageView *views;
	VkImage *images;
	uint32_t imageCount;
	VkSurfaceKHR surface;
	VkSurfaceFormatKHR surfaceFormat;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkPresentModeKHR presentMode;
	VkImageUsageFlags imageUsage;
};

struct Buffer
{
	VkBuffer buff;
	VkDeviceMemory memory;
	void *staging;
	bool transient;
};

struct Texture
{
	VkImageView imageView;
	VkImage image;
	VkImageLayout layout;
	VkDeviceMemory memory;
	bool transient;
};

struct Framebuffer
{
	VkFramebuffer fb;
	VkImageView *attachments;
	uint32_t width, height, layers, attachmentCount;
};

struct RenderPassDesc
{
	VkRenderPass rp;
	uint32_t clearValueCount;
	VkClearValue *clearValues;
};

struct Pipeline
{
	VkPipeline pipeline;
	VkPipelineLayout layout;
	VkPipelineBindPoint bindPoint;
};

#define VKST_BINARY		0
#define VKST_TIMELINE	1
struct Semaphore
{
	VkSemaphore sem;
	uint64_t value;
};

struct AccelerationStructure
{
	VkAccelerationStructureKHR as;
};

extern VkInstance Vkd_inst;
extern VkAllocationCallbacks *Vkd_allocCb, *Vkd_transientAllocCb;
extern VkCommandPool Vkd_transferPool;
extern VkSemaphore Vkd_stagingSignal;

// Device
struct RenderDevice *Vk_CreateDevice(struct RenderDeviceInfo *info, struct RenderDeviceProcs *devProcs, struct RenderContextProcs *ctxProcs);
bool Vk_Execute(struct RenderDevice *dev, struct RenderContext *ctx, bool wait);
void Vk_WaitIdle(struct RenderDevice *dev);
void Vk_DestroyDevice(struct RenderDevice *dev);

// Pipeline
struct Pipeline *Vk_GraphicsPipeline(struct RenderDevice *dev, const struct GraphicsPipelineDesc *desc);
struct Pipeline *Vk_ComputePipeline(struct RenderDevice *dev, const struct ComputePipelineDesc *desc);
struct Pipeline *Vk_RayTracingPipeline(struct RenderDevice *dev, struct ShaderBindingTable *sbt, uint32_t maxDepth);
void Vk_LoadPipelineCache(struct RenderDevice *dev);
void Vk_SavePipelineCache(struct RenderDevice *dev);
void Vk_DestroyPipeline(struct RenderDevice *dev, struct Pipeline *pipeline);

// Swapchain
struct Swapchain *Vk_CreateSwapchain(struct RenderDevice *dev, VkSurfaceKHR surface, bool verticalSync);
void Vk_DestroySwapchain(struct RenderDevice *dev, struct Swapchain *sw);
void *Vk_AcquireNextImage(struct RenderDevice *, struct Swapchain *sw);
bool Vk_Present(struct RenderDevice *dev, struct RenderContext *ctx, struct Swapchain *sw, void *image);
enum TextureFormat Vk_SwapchainFormat(struct Swapchain *sw);
struct Texture *Vk_SwapchainTexture(struct Swapchain *sw, void *image);
void Vk_SwapchainDesc(struct Swapchain *sw, struct FramebufferAttachmentDesc *desc);
void Vk_ScreenResized(struct RenderDevice *dev, struct Swapchain *sw);

// Surface (Platform.c, Driver.c)
bool Vk_CheckPresentSupport(VkPhysicalDevice dev, uint32_t family);
VkSurfaceKHR Vk_CreateSurface(struct RenderDevice *dev, void *window);
void Vk_DestroySurface(struct RenderDevice *dev, VkSurfaceKHR surface);

// Context
struct RenderContext *Vk_CreateContext(struct RenderDevice *dev);
void Vk_ResetContext(struct RenderDevice *dev, struct RenderContext *ctx);
void Vk_DestroyContext(struct RenderDevice *dev, struct RenderContext *ctx);

// Texture
bool Vk_CreateImage(struct RenderDevice *dev, const struct TextureDesc *desc, struct Texture *tex, bool alias);
bool Vk_CreateImageView(struct RenderDevice *dev, const struct TextureDesc *desc, struct Texture *tex);
struct Texture *Vk_CreateTexture(struct RenderDevice *dev, const struct TextureDesc *desc, uint16_t location);
enum TextureLayout Vk_TextureLayout(const struct Texture *tex);
void Vk_DestroyTexture(struct RenderDevice *dev, struct Texture *tex);

// Buffer
struct Buffer *Vk_CreateBuffer(struct RenderDevice *dev, const struct BufferDesc *desc, uint16_t location);
void Vk_UpdateBuffer(struct RenderDevice *dev, struct Buffer *buff, uint64_t offset, void *data, uint64_t size);
void *Vk_MapBuffer(struct RenderDevice *dev, struct Buffer *buff);
void Vk_FlushBuffer(struct RenderDevice *dev, struct Buffer *buff, uint64_t offset, uint64_t size);
void Vk_UnmapBuffer(struct RenderDevice *dev, struct Buffer *buff);
uint64_t Vk_BufferAddress(struct RenderDevice *dev, const struct Buffer *buff, uint64_t offset);
void Vk_DestroyBuffer(struct RenderDevice *dev, struct Buffer *buff);

// Acceleration Structure
struct AccelerationStructure *Vk_CreateAccelerationStructure(struct RenderDevice *dev, const struct AccelerationStructureCreateInfo *asci);
uint64_t Vk_AccelerationStructureHandle(struct RenderDevice *dev, const struct AccelerationStructure *as);
void Vk_DestroyAccelerationStructure(struct RenderDevice *dev, struct AccelerationStructure *as);

// Framebuffer
struct Framebuffer *Vk_CreateFramebuffer(struct RenderDevice *dev, const struct FramebufferDesc *desc);
void Vk_SetAttachment(struct Framebuffer *fb, uint32_t pos, struct Texture *tex);
void Vk_DestroyFramebuffer(struct RenderDevice *dev, struct Framebuffer *fb);

// Render Pass
struct RenderPassDesc *Vk_CreateRenderPassDesc(struct RenderDevice *dev, const struct AttachmentDesc *attachments, uint32_t count, const struct AttachmentDesc *depthAttachment);
void Vk_DestroyRenderPassDesc(struct RenderDevice *dev, struct RenderPassDesc *fb);

// Descriptor Set
bool Vk_CreateDescriptorSet(struct RenderDevice *dev);
void Vk_SetSampler(struct RenderDevice *dev, uint16_t location, VkSampler sampler);
void Vk_SetBuffer(struct RenderDevice *dev, uint16_t location, VkBuffer buffer);
void Vk_SetTexture(struct RenderDevice *dev, uint16_t location, VkImageView imageView);
void Vk_TermDescriptorSet(struct RenderDevice *dev);

// Shader
void *Vk_ShaderModule(struct RenderDevice *dev, const char *name);
bool Vk_LoadShaders(VkDevice dev);
void Vk_UnloadShaders(VkDevice dev);

// Sampler
VkSampler Vk_CreateSampler(struct RenderDevice *dev, const struct SamplerDesc *desc);
void Vk_DestroySampler(struct RenderDevice *dev, VkSampler s);

// TransientResources
struct Texture *Vk_CreateTransientTexture(struct RenderDevice *dev, const struct TextureDesc *desc, uint16_t location, uint64_t offset, uint64_t *size);
struct Buffer *Vk_CreateTransientBuffer(struct RenderDevice *dev, const struct BufferDesc *desc, uint16_t location, uint64_t offset, uint64_t *size);
bool Vk_InitTransientHeap(struct RenderDevice *dev, uint64_t size);
bool Vk_ResizeTransientHeap(struct RenderDevice *dev, uint64_t size);
void Vk_TermTransientHeap(struct RenderDevice *dev);

// Synchronization
VkSemaphore Vk_CreateSemaphore(struct RenderDevice *dev);
void Vk_DestroySemaphore(struct RenderDevice *dev, VkSemaphore *s);

VkFence Vk_CreateFence(struct RenderDevice *dev, bool createSignaled);
void Vk_SignalFence(struct RenderDevice *dev, VkFence f);
bool Vk_WaitForFence(struct RenderDevice *dev, VkFence f, uint64_t timeout);
void Vk_DestroyFence(struct RenderDevice *dev, VkFence f);

// Staging; support for systems without CPU visible device local memory (eg. Windows 7)
bool Vkd_InitStagingArea(struct RenderDevice *dev);
void *Vkd_AllocateStagingMemory(VkDevice dev, VkBuffer buff, VkMemoryRequirements *mr);
void Vkd_CommitStagingArea(struct RenderDevice *dev, VkSemaphore wait);
void Vkd_TermStagingArea(struct RenderDevice *dev);

// Utility functions
void Vk_InitContextProcs(struct RenderContextProcs *p);

static inline VkImageAspectFlags
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
Vkd_MemoryTypeIndex(const struct RenderDevice *dev, uint32_t filter, VkMemoryPropertyFlags flags)
{
	for (uint32_t i = 0; i < dev->physDevMemProps.memoryTypeCount; ++i)
		if (/*(filter & (1 << i)) &&*/ ((dev->physDevMemProps.memoryTypes[i].propertyFlags & flags) == flags))
			return i;
	return 0;
}

static inline VkCommandBuffer
Vkd_TransferCmdBuffer(struct RenderDevice *dev)
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
Vkd_ExecuteCmdBuffer(struct RenderDevice *dev, VkCommandBuffer cb)
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
		default: return TL_UNKNOWN;
	}
}

#endif /* _VULKAN_DRIVER_H_ */
