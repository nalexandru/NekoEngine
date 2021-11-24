#ifndef _NE_D3D12_DRIVER_H_
#define _NE_D3D12_DRIVER_H_

#include <d3d12.h>
#include <dxgi1_6.h>

#include <Render/Types.h>
#include <Render/Render.h>
#include <Render/Driver/RayTracing.h>
#include <Runtime/Array.h>

#include "D3D12Downlevel.h"

#define D3DDRV_MOD L"D3D12Drv"

struct RenderDevice
{
	ID3D12Device5 *dev;
	ID3D12CommandQueue *graphicsQueue, *copyQueue, *computeQueue;
	ID3D12Fence *renderFence[RE_NUM_FRAMES];
	UINT64 fenceValue[RE_NUM_FRAMES];
	HANDLE fenceEvent;

	ID3D12Debug3 *debug;
	IDXGIAdapter1 *adapter;

	ID3D12CommandQueueDownlevel *graphicsQueueDownlevel;
	ID3D12DeviceDownlevel *devDownlevel;

/*	VkDevice dev;
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
	VkDescriptorPool descriptorPool;*/
};

struct RenderContext
{
	ID3D12GraphicsCommandList *graphicsList;
	ID3D12GraphicsCommandList4 *rtGraphicsList;
	ID3D12CommandAllocator *graphicsAllocators;

	ID3D12GraphicsCommandList *copyList;
	ID3D12CommandAllocator *copyAllocators;

	ID3D12GraphicsCommandList *computeList;
	ID3D12GraphicsCommandList4 *rtComputeList;
	ID3D12CommandAllocator *computeAllocators;

	/*VkCommandBuffer cmdBuffer;
	struct Pipeline *boundPipeline;
	VkCommandPool *graphicsPools, *transferPools, *computePools;
	struct Array *graphicsCmdBuffers, *secondaryCmdBuffers, *transferCmdBuffers, *computeCmdBuffers;
	VkFence executeFence;
	VkDevice dev;
	VkDescriptorSet descriptorSet;*/
};

struct VulkanDeviceInfo
{
	/*VkPhysicalDevice physicalDevice;
	uint32_t graphicsFamily, computeFamily, transferFamily;*/
	void *a;
};

struct Swapchain
{
	IDXGISwapChain3 *sw;
	ID3D12Resource *buffers[RE_NUM_FRAMES];
	struct Surface *surface;
	UINT presentInterval;
	DXGI_SWAP_CHAIN_DESC1 desc;
};

struct Buffer
{
	ID3D12Resource *res;
	D3D12_INDEX_BUFFER_VIEW idxView;
};

struct Texture
{
	ID3D12Resource *res;
};

struct Framebuffer
{
/*	VkFramebuffer fb;
	VkImageView *attachments;*/
	uint32_t width, height, layers, attachmentCount;
};

struct RenderPassDesc
{
//	VkRenderPass rp;
	uint32_t clearValueCount;
//	VkClearValue *clearValues;
};

struct Pipeline
{
	ID3D12PipelineState *ps;
	ID3D12RootSignature *rs;
};

#define VKST_BINARY		0
#define VKST_TIMELINE	1
struct Semaphore
{
//	VkSemaphore sem;
	uint64_t value;
};

struct AccelerationStructure
{
	ID3D12Resource *buffer;
};

struct Surface
{
	IUnknown *coreWindow;
	HWND hWnd;
};

extern IDXGIFactory1 *D3D12_dxgiFactory;

// Device
struct RenderDevice *D3D12_CreateDevice(struct RenderDeviceInfo *info, struct RenderDeviceProcs *devProcs, struct RenderContextProcs *ctxProcs);
bool D3D12_Execute(struct RenderDevice *dev, struct RenderContext *ctx, bool wait);
void D3D12_WaitIdle(struct RenderDevice *dev);
void D3D12_DestroyDevice(struct RenderDevice *dev);

// Pipeline
struct Pipeline *D3D12_GraphicsPipeline(struct RenderDevice *dev, const struct GraphicsPipelineDesc *desc);
struct Pipeline *D3D12_ComputePipeline(struct RenderDevice *dev, const struct ComputePipelineDesc *desc);
struct Pipeline *D3D12_RayTracingPipeline(struct RenderDevice *dev, struct ShaderBindingTable *sbt, uint32_t maxDepth);
void D3D12_LoadPipelineCache(struct RenderDevice *dev);
void D3D12_SavePipelineCache(struct RenderDevice *dev);
void D3D12_DestroyPipeline(struct RenderDevice *dev, struct Pipeline *pipeline);

// Swapchain
struct Swapchain *D3D12_CreateSwapchain(struct RenderDevice *dev, void *surface, bool verticalSync);
void D3D12_DestroySwapchain(struct RenderDevice *dev, struct Swapchain *sw);
void *D3D12_AcquireNextImage(struct RenderDevice *, struct Swapchain *sw);
bool D3D12_Present(struct RenderDevice *dev, struct RenderContext *ctx, struct Swapchain *sw, void *image, struct Semaphore *sem);
enum TextureFormat D3D12_SwapchainFormat(struct Swapchain *sw);
struct Texture *D3D12_SwapchainTexture(struct Swapchain *sw, void *image);
void D3D12_ScreenResized(struct RenderDevice *dev, struct Swapchain *sw);

// Surface (Platform)
struct Surface *D3D12_CreateWin32Surface(struct RenderDevice *dev, void *window);
void D3D12_DestroyWin32Surface(struct RenderDevice *dev, struct Surface *surface);
struct Surface *D3D12_CreateUWPSurface(struct RenderDevice *dev, void *window);
void D3D12_DestroyUWPSurface(struct RenderDevice *dev, struct Surface *surface);

// Context
struct RenderContext *D3D12_CreateContext(struct RenderDevice *dev);
void D3D12_ResetContext(struct RenderDevice *dev, struct RenderContext *ctx);
void D3D12_DestroyContext(struct RenderDevice *dev, struct RenderContext *ctx);

// Texture
bool D3D12_CreateImage(struct RenderDevice *dev, const struct TextureDesc *desc, struct Texture *tex, bool alias);
bool D3D12_CreateImageView(struct RenderDevice *dev, const struct TextureDesc *desc, struct Texture *tex);
struct Texture *D3D12_CreateTexture(struct RenderDevice *dev, const struct TextureDesc *desc, uint16_t location);
enum TextureLayout D3D12_TextureLayout(const struct Texture *tex);
void D3D12_DestroyTexture(struct RenderDevice *dev, struct Texture *tex);

// Buffer
struct Buffer *D3D12_CreateBuffer(struct RenderDevice *dev, const struct BufferDesc *desc, uint16_t location);
void D3D12_UpdateBuffer(struct RenderDevice *dev, struct Buffer *buff, uint64_t offset, void *data, uint64_t size);
void *D3D12_MapBuffer(struct RenderDevice *dev, struct Buffer *buff);
void D3D12_FlushBuffer(struct RenderDevice *dev, struct Buffer *buff, uint64_t offset, uint64_t size);
void D3D12_UnmapBuffer(struct RenderDevice *dev, struct Buffer *buff);
uint64_t D3D12_BufferAddress(struct RenderDevice *dev, const struct Buffer *buff, uint64_t offset);
uint64_t D3D12_OffsetAddress(uint64_t addr, uint64_t offset);
void D3D12_DestroyBuffer(struct RenderDevice *dev, struct Buffer *buff);

// Acceleration Structure
struct AccelerationStructure *D3D12_CreateAccelerationStructure(struct RenderDevice *dev, const struct AccelerationStructureCreateInfo *asci);
uint64_t D3D12_AccelerationStructureHandle(struct RenderDevice *dev, const struct AccelerationStructure *as);
void D3D12_DestroyAccelerationStructure(struct RenderDevice *dev, struct AccelerationStructure *as);

// Framebuffer
struct Framebuffer *D3D12_CreateFramebuffer(struct RenderDevice *dev, const struct FramebufferDesc *desc);
void D3D12_SetAttachment(struct Framebuffer *fb, uint32_t pos, struct Texture *tex);
void D3D12_DestroyFramebuffer(struct RenderDevice *dev, struct Framebuffer *fb);

// Render Pass
struct RenderPassDesc *D3D12_CreateRenderPassDesc(struct RenderDevice *dev, const struct AttachmentDesc *attachments, uint32_t count,
													const struct AttachmentDesc *depthAttachment, const struct AttachmentDesc *inputAttachments, uint32_t inputCount);
void D3D12_DestroyRenderPassDesc(struct RenderDevice *dev, struct RenderPassDesc *fb);

// Descriptor Set
bool D3D12_CreateDescriptorSet(struct RenderDevice *dev);
/*void D3D12_SetSampler(struct RenderDevice *dev, uint16_t location, VkSampler sampler);
void D3D12_SetBuffer(struct RenderDevice *dev, uint16_t location, VkBuffer buffer);
void D3D12_SetTexture(struct RenderDevice *dev, uint16_t location, VkImageView imageView);*/
void D3D12_TermDescriptorSet(struct RenderDevice *dev);

// Shader
void *D3D12_ShaderModule(struct RenderDevice *dev, const char *name);
//bool D3D12_LoadShaders(VkDevice dev);
//void D3D12_UnloadShaders(VkDevice dev);

// Sampler
D3D12_SAMPLER_DESC *D3D12_CreateSampler(struct RenderDevice *dev, const struct SamplerDesc *desc);
void D3D12_DestroySampler(struct RenderDevice *dev, D3D12_SAMPLER_DESC *s);

// TransientResources
struct Texture *D3D12_CreateTransientTexture(struct RenderDevice *dev, const struct TextureDesc *desc, uint16_t location, uint64_t offset, uint64_t *size);
struct Buffer *D3D12_CreateTransientBuffer(struct RenderDevice *dev, const struct BufferDesc *desc, uint16_t location, uint64_t offset, uint64_t *size);
bool D3D12_InitTransientHeap(struct RenderDevice *dev, uint64_t size);
bool D3D12_ResizeTransientHeap(struct RenderDevice *dev, uint64_t size);
void D3D12_TermTransientHeap(struct RenderDevice *dev);

// Synchronization
struct Semaphore *D3D12_CreateSemaphore(struct RenderDevice *dev);
void D3D12_DestroySemaphore(struct RenderDevice *dev, struct Semaphore *s);

struct Fence *D3D12_CreateFence(struct RenderDevice *dev, bool createSignaled);
void D3D12_SignalFence(struct RenderDevice *dev, struct Fence *f);
bool D3D12_WaitForFence(struct RenderDevice *dev, struct Fence *f, uint64_t timeout);
void D3D12_DestroyFence(struct RenderDevice *dev, struct Fence *f);

// Utility functions
void D3D12_InitContextProcs(struct RenderContextProcs *p);

/*static inline VkImageAspectFlags
NeFormatAspect(enum TextureFormat fmt)
{
	switch (fmt) {
	case TF_D32_SFLOAT: return VK_IMAGE_ASPECT_DEPTH_BIT;
	case TF_D24_STENCIL8: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	default: return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}*/

static inline DXGI_FORMAT
NeToDXGITextureFormat(enum TextureFormat fmt)
{
	switch (fmt) {
	case TF_R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
	case TF_R8G8B8A8_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case TF_B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
	case TF_B8G8R8A8_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	case TF_R16G16B16A16_SFLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case TF_R32G32B32A32_SFLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case TF_A2R10G10B10_UNORM: return DXGI_FORMAT_R10G10B10A2_UNORM;
	case TF_D32_SFLOAT: return DXGI_FORMAT_D32_FLOAT;
	case TF_D24_STENCIL8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case TF_R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
	case TF_R8_UNORM: return DXGI_FORMAT_R8_UNORM;
	case TF_BC5_UNORM: return DXGI_FORMAT_BC5_UNORM;
	case TF_BC5_SNORM: return DXGI_FORMAT_BC5_SNORM;
	case TF_BC6H_UF16: return DXGI_FORMAT_BC6H_UF16;
	case TF_BC6H_SF16: return DXGI_FORMAT_BC6H_SF16;
	case TF_BC7_UNORM: return DXGI_FORMAT_BC7_UNORM;
	case TF_BC7_SRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;
	default: return DXGI_FORMAT_UNKNOWN;
	}
}

static inline enum TextureFormat
DXGIToNeTextureFormat(DXGI_FORMAT fmt)
{
	switch (fmt) {
	case DXGI_FORMAT_R8G8B8A8_UNORM: return TF_R8G8B8A8_UNORM;
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return TF_R8G8B8A8_SRGB;
	case DXGI_FORMAT_B8G8R8A8_UNORM: return TF_B8G8R8A8_UNORM;
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return TF_B8G8R8A8_SRGB;
	case DXGI_FORMAT_R16G16B16A16_FLOAT: return TF_R16G16B16A16_SFLOAT;
	case DXGI_FORMAT_R32G32B32A32_FLOAT: return TF_R32G32B32A32_SFLOAT;
	case DXGI_FORMAT_R10G10B10A2_UNORM: return TF_A2R10G10B10_UNORM;
	case DXGI_FORMAT_D32_FLOAT: return TF_D32_SFLOAT;
	case DXGI_FORMAT_D24_UNORM_S8_UINT: return TF_D24_STENCIL8;
	case DXGI_FORMAT_R8G8_UNORM: return TF_R8G8_UNORM;
	case DXGI_FORMAT_R8_UNORM: return TF_R8_UNORM;
	case DXGI_FORMAT_BC5_UNORM: return TF_BC5_UNORM;
	case DXGI_FORMAT_BC5_SNORM: return TF_BC5_SNORM;
	case DXGI_FORMAT_BC6H_UF16: return TF_BC6H_UF16;
	case DXGI_FORMAT_BC6H_SF16: return TF_BC6H_SF16;
	case DXGI_FORMAT_BC7_UNORM: return TF_BC7_UNORM;
	case DXGI_FORMAT_BC7_UNORM_SRGB: return TF_BC7_SRGB;
	default: return TF_INVALID;
	}
}

/*static inline VkMemoryPropertyFlags
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
Vkd_TransitionImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
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
		.subresourceRange =
		{
			.baseMipLevel = 0,
			.baseArrayLayer = 0,
			.levelCount = 1,
			.layerCount = 1
		}
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
}*/

/*static inline VkImageLayout
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
}*/

#endif /* _NE_D3D12_DRIVER_H_ */
