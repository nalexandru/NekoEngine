#ifndef _VULKAN_DRIVER_H_
#define _VULKAN_DRIVER_H_

#include <stdbool.h>

#include <Engine/Types.h>
#include <Render/Render.h>
#include <Render/Buffer.h>
#include <Render/Texture.h>
#include <Render/Framebuffer.h>
#include <Runtime/Array.h>

#define VK_ENABLE_BETA_EXTENSIONS
#include "volk.h"

struct RenderDevice
{
	VkDevice dev;
	VkQueue transferQueue, graphicsQueue, computeQueue;
	uint32_t graphicsFamily, computeFamily, transferFamily;
	VkPhysicalDevice physDev;
	VkPhysicalDeviceProperties physDevProps;
	VkPhysicalDeviceMemoryProperties physDevMemProps;
	VkSemaphore frameSemaphore;
	uint64_t semaphoreValue;
	uint64_t *frameValues;
	VkCommandPool driverTransferPool;
	VkFence driverTransferFence;
};

struct RenderContext
{
	VkCommandBuffer cmdBuffer;
	struct Pipeline *boundPipeline;
	VkCommandPool *graphicsPools, *transferPools, *computePools;
	struct Array *graphicsCmdBuffers, *transferCmdBuffers, *computeCmdBuffers;
	VkFence executeFence;
	VkDevice dev;
};

struct VulkanDeviceInfo
{
	VkPhysicalDevice physicalDevice;
	uint32_t graphicsFamily, computeFamily, transferFamily;
};

struct Swapchain
{
	VkSwapchainKHR sw;
	VkSemaphore frameStart, frameEnd;
	VkImageView *views;
	VkImage *images;
	uint32_t imageCount;
	VkSurfaceKHR surface;
	VkSurfaceFormatKHR surfaceFormat;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VkPresentModeKHR presentMode;
};

struct Buffer
{
	VkBuffer buff;
	struct BufferDesc desc;
	VkDeviceMemory memory;
};

struct Texture
{
	VkImageView imageView;
	VkImage image;
	VkImageLayout layout;
	struct TextureDesc desc;
};

struct Framebuffer
{
	VkFramebuffer fb;
	VkImageView *attachments;
	uint32_t width, height, layers, attachmentCount;
};

struct RenderPass
{
	VkRenderPass rp;
};

struct Pipeline
{
	VkPipeline pipeline;
	VkPipelineBindPoint bindPoint;
};

struct PipelineLayout
{
	VkPipelineLayout layout;
};

struct DescriptorSetLayout
{
	VkDescriptorSetLayout layout;
	uint32_t sizeCount;
	VkDescriptorPoolSize *sizes;
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
extern VkAllocationCallbacks *Vkd_allocCb;
extern struct Array Vkd_contexts;
extern VkCommandPool Vkd_transferPool;

// Device
struct RenderDevice *Vk_CreateDevice(struct RenderDeviceInfo *info, struct RenderDeviceProcs *devProcs, struct RenderContextProcs *ctxProcs);
bool Vk_Execute(struct RenderDevice *dev, struct RenderContext *ctx, bool wait);
void Vk_WaitIdle(struct RenderDevice *dev);
void Vk_DestroyDevice(struct RenderDevice *dev);

// Pipeline
struct PipelineLayout *Vk_CreatePipelineLayout(struct RenderDevice *dev, const struct PipelineLayoutDesc *desc);
void Vk_DestroyPipelineLayout(struct RenderDevice *dev, struct PipelineLayout *layout);
struct Pipeline *Vk_GraphicsPipeline(struct RenderDevice *dev, const struct GraphicsPipelineDesc *desc);
struct Pipeline *Vk_ComputePipeline(struct RenderDevice *dev, struct Shader *sh);
struct Pipeline *Vk_RayTracingPipeline(struct RenderDevice *dev, struct ShaderBindingTable *sbt, uint32_t maxDepth);
void Vk_LoadPipelineCache(struct RenderDevice *dev);
void Vk_SavePipelineCache(struct RenderDevice *dev);

// Swapchain
struct Swapchain *Vk_CreateSwapchain(struct RenderDevice *dev, VkSurfaceKHR surface);
void Vk_DestroySwapchain(struct RenderDevice *dev, struct Swapchain *sw);
void *Vk_AcquireNextImage(struct RenderDevice *, struct Swapchain *sw);
bool Vk_Present(struct RenderDevice *dev, struct RenderContext *ctx, struct Swapchain *sw, void *image);
enum TextureFormat Vk_SwapchainFormat(struct Swapchain *sw);
struct Texture *Vk_SwapchainTexture(struct Swapchain *sw, void *image);

// Surface (Platform.c, Driver.c)
bool Vk_CheckPresentSupport(VkPhysicalDevice dev, uint32_t family);
VkSurfaceKHR Vk_CreateSurface(struct RenderDevice *dev, void *window);
void Vk_DestroySurface(struct RenderDevice *dev, VkSurfaceKHR surface);

// Context
struct RenderContext *Vk_CreateContext(struct RenderDevice *dev);
void Vk_DestroyContext(struct RenderDevice *dev, struct RenderContext *ctx);

// Texture
struct Texture *Vk_CreateTexture(struct RenderDevice *dev, struct TextureCreateInfo *tci);
const struct TextureDesc *Vk_TextureDesc(const struct Texture *tex);
enum TextureLayout Vk_TextureLayout(const struct Texture *tex);
void Vk_DestroyTexture(struct RenderDevice *dev, struct Texture *tex);

// Buffer
struct Buffer *Vk_CreateBuffer(struct RenderDevice *dev, struct BufferCreateInfo *bci);
void Vk_UpdateBuffer(struct RenderDevice *dev, struct Buffer *buff, uint64_t offset, void *data, uint64_t size);
const struct BufferDesc *Vk_BufferDesc(const struct Buffer *buff);
void Vk_DestroyBuffer(struct RenderDevice *dev, struct Buffer *buff);

// Acceleration Structure
struct AccelerationStructure *Vk_CreateAccelerationStructure(struct RenderDevice *dev, struct AccelerationStructureCreateInfo *asci);
void Vk_DestroyAccelerationStructure(struct RenderDevice *dev, struct AccelerationStructure *as);

// Framebuffer
struct Framebuffer *Vk_CreateFramebuffer(struct RenderDevice *dev, const struct FramebufferDesc *desc);
void Vk_SetAttachment(struct Framebuffer *fb, uint32_t pos, struct Texture *tex);
void Vk_DestroyFramebuffer(struct RenderDevice *dev, struct Framebuffer *fb);

// Render Pass
struct RenderPass *Vk_CreateRenderPass(struct RenderDevice *dev, const struct RenderPassDesc *desc);
void Vk_DestroyRenderPass(struct RenderDevice *dev, struct RenderPass *fb);

// Descriptor Set
struct DescriptorSetLayout *Vk_CreateDescriptorSetLayout(struct RenderDevice *dev, const struct DescriptorSetLayoutDesc *desc);
void Vk_DestroyDescriptorSetLayout(struct RenderDevice *dev, struct DescriptorSetLayout *dsl);
VkDescriptorSet Vk_CreateDescriptorSet(struct RenderDevice *dev, const struct DescriptorSetLayout *layout);
void Vk_WriteDescriptorSet(struct RenderDevice *dev, VkDescriptorSet ds, const struct DescriptorWrite *writes, uint32_t writeCount);
void Vk_DestroyDescriptorSet(struct RenderDevice *dev, VkDescriptorSet ds);
bool Vk_InitDescriptorPools(VkDevice dev);
void Vk_TermDescriptorPools(VkDevice dev);

// Shader
void *Vk_ShaderModule(struct RenderDevice *dev, const char *name);
bool Vk_LoadShaders(VkDevice dev);
void Vk_UnloadShaders(VkDevice dev);

// Utility functions
void Vk_InitContextProcs(struct RenderContextProcs *p);

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
		if ((filter & (1 << i)) && ((dev->physDevMemProps.memoryTypes[i].propertyFlags & flags) == flags))
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

	vkFreeCommandBuffers(dev->dev, dev->driverTransferPool, 1, &cb);
}

#endif /* _VULKAN_DRIVER_H_ */
