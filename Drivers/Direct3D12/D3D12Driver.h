#ifndef _NE_D3D12_DRIVER_H_
#define _NE_D3D12_DRIVER_H_

#include <d3d12.h>
#include <dxgi1_6.h>

#include <Render/Types.h>
#include <Render/Render.h>
#include <Render/Driver/RayTracing.h>
#include <Runtime/Array.h>

#include "D3D12Downlevel.h"

#define D3DDRV_MOD "D3D12Drv"

struct NeRenderDevice
{
	ID3D12Device5 *dev;
	ID3D12CommandQueue *graphicsQueue, *copyQueue, *computeQueue;
	ID3D12Fence *renderFence[RE_NUM_FRAMES];
	UINT64 fenceValue[RE_NUM_FRAMES];
	HANDLE fenceEvent;

	ID3D12Heap *transientHeap;
	ID3D12CommandAllocator *driverCopyAllocator;

	ID3D12DescriptorHeap *cpuDescriptorHeap, *cpuSamplerDescriptorHeap, *rtvHeap;
	ID3D12DescriptorHeap *descriptorHeap[RE_NUM_FRAMES], *samplerDescriptorHeap[RE_NUM_FRAMES];
	uint64_t heapIncrement, samplerHeapIncrement, rtvHeapIncrement;

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

struct NeFence
{
	uint64_t value;
	ID3D12Fence *fence;
	HANDLE event;
};

struct NeRenderContext
{
	ID3D12GraphicsCommandList4 *cmdList;

	ID3D12CommandAllocator **graphicsAllocators;
	ID3D12CommandAllocator **copyAllocators;
	ID3D12CommandAllocator **computeAllocators;

	struct NeFence executeFence;

	struct {
		struct NeArray *graphics, *copy, *compute;
	} closedList;

	struct {
		struct NeArray *graphics, *copy, *compute;
	} freeList;

	ID3D12Device5 *dev;
	struct NeRenderDevice *neDev;

	struct NeRenderPassDesc *boundRenderPass;
	struct NeFramebuffer *boundFramebuffer;

	/*VkCommandBuffer cmdBuffer;
	struct NePipeline *boundPipeline;
	VkCommandPool *graphicsPools, *transferPools, *computePools;
	struct NeArray *graphicsCmdBuffers, *secondaryCmdBuffers, *transferCmdBuffers, *computeCmdBuffers;
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

struct NeSwapchain
{
	IDXGISwapChain3 *sw;
	ID3D12Resource *buffers[RE_NUM_FRAMES];
	struct NeSurface *surface;
	UINT presentInterval;
	DXGI_SWAP_CHAIN_DESC1 desc;
};

struct NeBuffer
{
	ID3D12Resource *res;
	uint64_t size;
};

struct NeTexture
{
	ID3D12Resource *res;
};

struct NeFramebuffer
{
	ID3D12Resource *attachments[8];
	ID3D12Resource *depthAttachment;
	uint32_t width, height, layers, attachmentCount;
};

struct NeRenderPassDesc
{
	uint32_t attachmentCount, depthAttachmentId;
	DXGI_FORMAT rtvFormats[8];
	struct vec4 clearValues[8];
	enum NeAttachmentLoadOp loadOp[8];
	enum D3D12_RESOURCE_STATES rtvInitialState[8], rtvState[8], rtvFinalState[8];

	DXGI_FORMAT depthFormat;
	enum NeAttachmentLoadOp depthLoadOp;
	float depthClearValue;
	enum D3D12_RESOURCE_STATES depthInitialState, depthState, depthFinalState;
};

struct NePipeline
{
	ID3D12PipelineState *ps;
	ID3D12RootSignature *rs;
	D3D12_PRIMITIVE_TOPOLOGY topology;
};

#define VKST_BINARY		0
#define VKST_TIMELINE	1
struct NeSemaphore
{
//	VkSemaphore sem;
	uint64_t value;
};

struct NeAccelerationStructure
{
	ID3D12Resource *buffer;
};

struct NeSurface
{
	IUnknown *coreWindow;
	HWND hWnd;
};

struct D3D12DSubmitInfo
{
//	VkSemaphore wait, signal;
	uint64_t waitValue, signalValue;
//	VkCommandBuffer cmdBuffer;
	ID3D12CommandList *cmdList;
};

struct D3D12DShaderModule
{
	void *bytecode;
	size_t len;
};

extern IDXGIFactory1 *D3D12_dxgiFactory;

// Device
struct NeRenderDevice *D3D12_CreateDevice(struct NeRenderDeviceInfo *info, struct NeRenderDeviceProcs *devProcs, struct NeRenderContextProcs *ctxProcs);
bool D3D12_Execute(struct NeRenderDevice *dev, struct NeRenderContext *ctx, bool wait);
void D3D12_WaitIdle(struct NeRenderDevice *dev);
void D3D12_DestroyDevice(struct NeRenderDevice *dev);

// Pipeline
struct NePipeline *D3D12_GraphicsPipeline(struct NeRenderDevice *dev, const struct NeGraphicsPipelineDesc *desc);
struct NePipeline *D3D12_ComputePipeline(struct NeRenderDevice *dev, const struct NeComputePipelineDesc *desc);
struct NePipeline *D3D12_RayTracingPipeline(struct NeRenderDevice *dev, struct NeShaderBindingTable *sbt, uint32_t maxDepth);
void D3D12_LoadPipelineCache(struct NeRenderDevice *dev);
void D3D12_SavePipelineCache(struct NeRenderDevice *dev);
void D3D12_DestroyPipeline(struct NeRenderDevice *dev, struct NePipeline *pipeline);

// Swapchain
struct NeSwapchain *D3D12_CreateSwapchain(struct NeRenderDevice *dev, void *surface, bool verticalSync);
void D3D12_DestroySwapchain(struct NeRenderDevice *dev, struct NeSwapchain *sw);
void *D3D12_AcquireNextImage(struct NeRenderDevice *, struct NeSwapchain *sw);
bool D3D12_Present(struct NeRenderDevice *dev, struct NeRenderContext *ctx, struct NeSwapchain *sw, void *image, struct NeSemaphore *sem);
enum NeTextureFormat D3D12_SwapchainFormat(struct NeSwapchain *sw);
struct NeTexture *D3D12_SwapchainTexture(struct NeSwapchain *sw, void *image);
void D3D12_ScreenResized(struct NeRenderDevice *dev, struct NeSwapchain *sw);

// Surface (Platform)
struct NeSurface *D3D12_CreateWin32Surface(struct NeRenderDevice *dev, void *window);
void D3D12_DestroyWin32Surface(struct NeRenderDevice *dev, struct NeSurface *surface);
struct NeSurface *D3D12_CreateUWPSurface(struct NeRenderDevice *dev, void *window);
void D3D12_DestroyUWPSurface(struct NeRenderDevice *dev, struct NeSurface *surface);

// Context
struct NeRenderContext *D3D12_CreateContext(struct NeRenderDevice *dev);
void D3D12_ResetContext(struct NeRenderDevice *dev, struct NeRenderContext *ctx);
void D3D12_DestroyContext(struct NeRenderDevice *dev, struct NeRenderContext *ctx);

// Texture
void D3D12D_InitTextureDesc(const struct NeTextureDesc *desc, D3D12_HEAP_PROPERTIES *heapProperties, D3D12_RESOURCE_DESC *resDesc);
struct NeTexture *D3D12_CreateTexture(struct NeRenderDevice *dev, const struct NeTextureDesc *desc, uint16_t location);
enum NeTextureLayout D3D12_TextureLayout(const struct NeTexture *tex);
void D3D12_DestroyTexture(struct NeRenderDevice *dev, struct NeTexture *tex);

// Buffer
void D3D12D_InitBufferDesc(const struct NeBufferDesc *desc, D3D12_HEAP_PROPERTIES *heapProperties, D3D12_RESOURCE_DESC *resDesc);
struct NeBuffer *D3D12_CreateBuffer(struct NeRenderDevice *dev, const struct NeBufferDesc *desc, uint16_t location);
void D3D12_UpdateBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff, uint64_t offset, void *data, uint64_t size);
void *D3D12_MapBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff);
void D3D12_FlushBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff, uint64_t offset, uint64_t size);
void D3D12_UnmapBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff);
uint64_t D3D12_BufferAddress(struct NeRenderDevice *dev, const struct NeBuffer *buff, uint64_t offset);
uint64_t D3D12_OffsetAddress(uint64_t addr, uint64_t offset);
void D3D12_DestroyBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff);

// Acceleration Structure
struct NeAccelerationStructure *D3D12_CreateAccelerationStructure(struct NeRenderDevice *dev, const struct NeAccelerationStructureCreateInfo *asci);
uint64_t D3D12_AccelerationStructureHandle(struct NeRenderDevice *dev, const struct NeAccelerationStructure *as);
void D3D12_DestroyAccelerationStructure(struct NeRenderDevice *dev, struct NeAccelerationStructure *as);

// Framebuffer
struct NeFramebuffer *D3D12_CreateFramebuffer(struct NeRenderDevice *dev, const struct NeFramebufferDesc *desc);
void D3D12_SetAttachment(struct NeFramebuffer *fb, uint32_t pos, struct NeTexture *tex);
void D3D12_DestroyFramebuffer(struct NeRenderDevice *dev, struct NeFramebuffer *fb);

// Render Pass
struct NeRenderPassDesc *D3D12_CreateRenderPassDesc(struct NeRenderDevice *dev, const struct NeAttachmentDesc *attachments, uint32_t count,
													const struct NeAttachmentDesc *depthAttachment, const struct NeAttachmentDesc *inputAttachments, uint32_t inputCount);
void D3D12_DestroyRenderPassDesc(struct NeRenderDevice *dev, struct NeRenderPassDesc *fb);

// Descriptor Heap
bool D3D12_InitDescriptorHeap(struct NeRenderDevice *dev);
/*void D3D12_SetSampler(struct NeRenderDevice *dev, uint16_t location, VkSampler sampler);
void D3D12_SetBuffer(struct NeRenderDevice *dev, uint16_t location, VkBuffer buffer);
void D3D12_SetTexture(struct NeRenderDevice *dev, uint16_t location, VkImageView imageView);*/
void D3D12_SetTexture(struct NeRenderDevice *dev, uint16_t location, ID3D12Resource *res);
void D3D12_TermDescriptorHeap(struct NeRenderDevice *dev);

// Shader
void *D3D12_ShaderModule(struct NeRenderDevice *dev, const char *name);
//bool D3D12_LoadShaders(VkDevice dev);
//void D3D12_UnloadShaders(VkDevice dev);

// Sampler
D3D12_SAMPLER_DESC *D3D12_CreateSampler(struct NeRenderDevice *dev, const struct NeSamplerDesc *desc);
void D3D12_DestroySampler(struct NeRenderDevice *dev, D3D12_SAMPLER_DESC *s);

// TransientResources
struct NeTexture *D3D12_CreateTransientTexture(struct NeRenderDevice *dev, const struct NeTextureDesc *desc, uint16_t location, uint64_t offset, uint64_t *size);
struct NeBuffer *D3D12_CreateTransientBuffer(struct NeRenderDevice *dev, const struct NeBufferDesc *desc, uint16_t location, uint64_t offset, uint64_t *size);
bool D3D12_InitTransientHeap(struct NeRenderDevice *dev, uint64_t size);
bool D3D12_ResizeTransientHeap(struct NeRenderDevice *dev, uint64_t size);
void D3D12_TermTransientHeap(struct NeRenderDevice *dev);

// Synchronization
struct NeSemaphore *D3D12_CreateSemaphore(struct NeRenderDevice *dev);
void D3D12_DestroySemaphore(struct NeRenderDevice *dev, struct NeSemaphore *s);

struct NeFence *D3D12_CreateFence(struct NeRenderDevice *dev, bool createSignaled);
bool D3D12Drv_InitFence(ID3D12Device5 *dev, struct NeFence *f, bool signaled);
void D3D12_SignalFence(struct NeRenderDevice *dev, struct NeFence *f);
bool D3D12_WaitForFence(struct NeRenderDevice *dev, struct NeFence *f, uint64_t timeout);
void D3D12Drv_TermFence(struct NeFence *f);
void D3D12_DestroyFence(struct NeRenderDevice *dev, struct NeFence *f);

// Utility functions
void D3D12_InitContextProcs(struct NeRenderContextProcs *p);

/*static inline VkImageAspectFlags
NeFormatAspect(enum NeTextureFormat fmt)
{
	switch (fmt) {
	case TF_D32_SFLOAT: return VK_IMAGE_ASPECT_DEPTH_BIT;
	case TF_D24_STENCIL8: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	default: return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}*/

static inline ID3D12GraphicsCommandList4 *
D3D12D_TransferCmdList(struct NeRenderDevice *dev)
{
	/*VkCommandBuffer cb;

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

	return cb;*/
	return NULL;
}

static inline DXGI_FORMAT
NeToDXGITextureFormat(enum NeTextureFormat fmt)
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

static inline enum NeTextureFormat
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

static inline D3D12_RESOURCE_STATES
NeTextureLayoutToD3D12ResourceState(enum NeTextureLayout lyt)
{
	switch (lyt) {
	case TL_COLOR_ATTACHMENT: return D3D12_RESOURCE_STATE_RENDER_TARGET;
	case TL_DEPTH_STENCIL_ATTACHMENT: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
	case TL_DEPTH_STENCIL_READ_ONLY_ATTACHMENT: return D3D12_RESOURCE_STATE_DEPTH_READ;
	case TL_DEPTH_ATTACHMENT: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
	case TL_STENCIL_ATTACHMENT: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
	case TL_DEPTH_READ_ONLY_ATTACHMENT: return D3D12_RESOURCE_STATE_DEPTH_READ;
	case TL_TRANSFER_SRC: return D3D12_RESOURCE_STATE_COPY_SOURCE;
	case TL_TRANSFER_DST: return D3D12_RESOURCE_STATE_COPY_DEST;
	case TL_SHADER_READ_ONLY: return D3D12_RESOURCE_STATE_GENERIC_READ;
	case TL_PRESENT_SRC: return D3D12_RESOURCE_STATE_PRESENT;
	}

	return D3D12_RESOURCE_STATE_COMMON;
}

/*static inline VkMemoryPropertyFlags
NeToVkMemoryProperties(enum NeGPUMemoryType type)
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
Vkd_AllocateCmdBuffer(VkDevice dev, VkCommandBufferLevel level, VkCommandPool pool, struct NeArray *freeList)
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
NeToVkImageLayout(enum NeTextureLayout tl)
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

static inline enum NeTextureLayout
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
