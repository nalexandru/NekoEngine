#ifndef _VULKAN_DRIVER_H_
#define _VULKAN_DRIVER_H_

#include <stdbool.h>

#include <Engine/Types.h>
#include <Render/Render.h>
#include <Render/Buffer.h>
#include <Render/Texture.h>
#include <Render/Framebuffer.h>
#include <Runtime/Runtime.h>

#define VK_ENABLE_BETA_EXTENSIONS
#include "volk.h"

struct RenderDevice
{
	VkDevice dev;
	VkQueue transferQueue, graphicsQueue, computeQueue;
	uint32_t graphicsFamily, computeFamily, transferFamily;
	VkPhysicalDevice physDev;
	VkPhysicalDeviceProperties physDevProps;
	VkSemaphore frameSemaphore;
	uint64_t semaphoreValue;
	uint64_t *frameValues;
};

struct RenderContext
{
	VkCommandBuffer cmdBuffer;
	VkCommandPool *graphicsPools, *transferPools, *computePools;
	Array *graphicsCmdBuffers, *transferCmdBuffers, *computeCmdBuffers;
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

extern VkInstance Vkd_inst;
extern VkAllocationCallbacks *Vkd_allocCb;
extern Array Vkd_contexts;

// Device
struct RenderDevice *Vk_CreateDevice(struct RenderDeviceInfo *info, struct RenderDeviceProcs *devProcs, struct RenderContextProcs *ctxProcs);
bool Vk_Execute(struct RenderDevice *dev, struct RenderContext *ctx, bool wait);
void Vk_WaitIdle(struct RenderDevice *dev);
void Vk_DestroyDevice(struct RenderDevice *dev);

// Pipeline
struct Pipeline *Vk_GraphicsPipeline(struct RenderDevice * dev, struct Shader * sh, uint64_t flags, const struct BlendAttachmentDesc *atDesc, uint32_t atCount);
struct Pipeline *Vk_ComputePipeline(struct RenderDevice *dev, struct Shader *sh);
struct Pipeline *Vk_RayTracingPipeline(struct RenderDevice *dev, struct ShaderBindingTable *sbt);
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
void Vk_DestroyTexture(struct RenderDevice *dev, struct Texture *tex);

// Buffer
struct Buffer *Vk_CreateBuffer(struct RenderDevice *dev, struct BufferCreateInfo *bci);
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
Vkd_AllocateCmdBuffer(VkDevice dev, VkCommandBufferLevel level, VkCommandPool pool, Array *freeList)
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

#endif /* _VULKAN_DRIVER_H_ */
