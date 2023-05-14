#ifndef VULKAN_BACKEND_H
#define VULKAN_BACKEND_H

// Build configuration

#include <Engine/BuildConfig.h>

 // End build configuration

#define VK_ENABLE_BETA_EXTENSIONS
#include "volk.h"

#if ENABLE_OPENXR
#	define XR_USE_GRAPHICS_API_VULKAN
#	include <openxr/openxr.h>
#	include <openxr/openxr_platform.h>
#endif

#define RE_NATIVE_VULKAN
#include <Render/Types.h>
#include <Render/Render.h>
#include <Render/Backend.h>
#include <Render/RayTracing.h>
#include <Runtime/Array.h>
#include <System/Thread.h>

#define VKDRV_MOD	"VulkanBackend"

struct VkdRenderQueue
{
	NeFutex ftx;
	VkQueue queue;
	uint32_t family, id;
};

struct NeRenderDevice
{
	VkDevice dev;
	VkDeviceMemory transientHeap;
	VkDescriptorSet descriptorSet;

	struct VkdRenderQueue graphics, compute, transfer;

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
	VkDescriptorPool iaDescriptorPool[RE_NUM_FRAMES];
};

struct NeRenderContext
{
	VkCommandBuffer cmdBuffer;
	struct NePipeline *boundPipeline;
	VkCommandPool *graphicsPools, *xferPools, *computePools;
	struct NeArray *graphicsCmdBuffers, *secondaryCmdBuffers, *xferCmdBuffers, *computeCmdBuffers;
	VkFence executeFence;
	VkDevice vkDev;
	VkDescriptorSet descriptorSet, iaSet;
	struct NeRenderDevice *neDev;
	uint32_t lastSubmittedXfer, lastSubmittedCompute;
	struct
	{
		struct NeArray graphics, compute, xfer;
	} queued;
	struct NeSemaphore *wait;
};

struct VulkanDeviceInfo
{
	VkPhysicalDevice physicalDevice;
	uint32_t graphicsFamily, computeFamily, transferFamily;
};

struct NeSwapchain
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

#if ENABLE_OPENXR
struct XrExtProcs
{
	PFN_xrGetVulkanGraphicsRequirements2KHR GetVulkanGraphicsRequirements2;
};

extern struct XrExtProcs Vkd_XRExt;

struct XRSwapchain
{
	struct NeSwapchain vkSwapchain;
	XrSwapchain colorSwapchain, depthSwapchain;
	XrActionSet actionSet;
	XrPath paths[2];
	struct
	{
		XrAction place;
		XrAction pose;
		XrAction exit;
		XrAction vibrateLeft;
		XrAction vibrateRight;
	} actions;
	XrSystemId sysId;
	XrSpace sceneSpace, leftHandSpace, rightHandSpace;
	uint32_t imageCount;
	XrSwapchainImageBaseHeader *colorImages, *depthImages;
};

extern XrInstance Vkd_xrInst;
extern XrSession Vkd_xrSession;
extern XrSessionState Vkd_xrState;
#endif

struct NeBuffer
{
	VkBuffer buff;
	VkDeviceMemory memory;
	void *staging;
	bool transient;
};

struct NeTexture
{
	VkImageView imageView;
	VkImage image;
	VkImageLayout layout;
	VkDeviceMemory memory;
	bool transient;
};

struct NeFramebuffer
{
	VkFramebuffer fb;
	VkImageView *attachments;
	uint32_t width, height, layers, attachmentCount;
};

struct NeRenderPassDesc
{
	VkRenderPass rp;
	uint32_t clearValueCount;
	VkClearValue *clearValues;
	uint32_t inputAttachments;
};

struct NePipeline
{
	VkPipeline pipeline;
	VkPipelineLayout layout;
	VkPipelineBindPoint bindPoint;
};

#define VKST_BINARY		0
#define VKST_TIMELINE	1
struct NeSemaphore
{
	VkSemaphore sem;
	uint64_t value;
};

struct NeAccelerationStructure
{
	VkAccelerationStructureKHR as;
};

struct Vkd_SubmitInfo
{
	VkSemaphore wait, signal;
	uint64_t waitValue, signalValue;
	VkCommandBuffer cmdBuffer;
};

extern VkInstance Vkd_inst;
extern VkAllocationCallbacks *Vkd_allocCb, *Vkd_transientAllocCb;
extern VkCommandPool Vkd_transferPool;
extern VkPipelineCache Vkd_pipelineCache;
extern struct NeSemaphore Vkd_stagingSignal;
extern uint32_t Vkd_instanceVersion;

// Debug
bool VkBk_InitDebug(void);
bool VkBk_SetObjectName(VkDevice dev, void *handle, VkObjectType type, const char *name);
void VkBk_TermDebug(void);

// Surface (Platform.c, Driver.c)
bool Vk_CheckPresentSupport(VkPhysicalDevice dev, uint32_t family);

// Context
void VkBk_ExecuteCommands(struct NeSwapchain *sw, struct NeSemaphore *waitSemaphore);

// Texture
bool Vk_CreateImage(const struct NeTextureDesc *desc, struct NeTexture *tex, bool alias);
bool Vk_CreateImageView(const struct NeTextureDesc *desc, struct NeTexture *tex);

// Descriptor Set
bool Vk_CreateDescriptorSet(struct NeRenderDevice *dev);
VkDescriptorSet Vk_AllocateIADescriptorSet(struct NeRenderDevice *dev);
void Vk_SetSampler(struct NeRenderDevice *dev, uint16_t location, VkSampler sampler);
void Vk_SetTexture(uint16_t location, VkImageView imageView);
void Vk_SetInputAttachment(struct NeRenderDevice *dev, VkDescriptorSet set, uint16_t location, VkImageView imageView);
void Vk_TermDescriptorSet(struct NeRenderDevice *dev);

// Shader
bool Vk_LoadShaders(VkDevice dev);
void Vk_UnloadShaders(VkDevice dev);

// Staging; support for systems without CPU visible device local memory (Windows 7)
bool VkBk_InitStagingArea(struct NeRenderDevice *dev);
void *VkBk_AllocateStagingMemory(VkDevice dev, VkBuffer buff, VkMemoryRequirements *mr);
void VkBk_CommitStagingArea(struct NeRenderDevice *dev, VkSemaphore wait);
void VkBk_TermStagingArea(struct NeRenderDevice *dev);
void VkBk_StagingBarrier(VkCommandBuffer cmdBuffer);

// DirectStorage; only for Windows 10 and newer
bool VkBk_InitDStorage(void);
NeDirectIOHandle VkBk_DStorageOpenFile(const char *path);
void VkBk_DStorageCloseFile(NeDirectIOHandle handle);
void VkBk_TermDStorage(void);

static inline VkImageAspectFlags
NeFormatAspect(enum NeTextureFormat fmt)
{
	switch (fmt) {
	case TF_D32_SFLOAT: return VK_IMAGE_ASPECT_DEPTH_BIT;
	case TF_D24_STENCIL8: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	default: return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}

static inline VkFormat
NeToVkTextureFormat(enum NeTextureFormat fmt)
{
	switch (fmt) {
	case TF_R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
	case TF_R8G8B8A8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
	case TF_B8G8R8A8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
	case TF_B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
	case TF_R16G16B16A16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
	case TF_R16G16B16A16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
	case TF_R32G32B32A32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
	case TF_R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
	case TF_A2R10G10B10_UNORM: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
	case TF_D32_SFLOAT: return VK_FORMAT_D32_SFLOAT;
	case TF_D24_STENCIL8: return VK_FORMAT_D24_UNORM_S8_UINT;
	case TF_R8G8_UNORM: return VK_FORMAT_R8G8_UNORM;
	case TF_R16G16_UNORM: return VK_FORMAT_R16G16_UNORM;
	case TF_R32G32_UINT: return VK_FORMAT_R32G32_UINT;
	case TF_R8_UNORM: return VK_FORMAT_R8_UNORM;
	case TF_R16_UNORM: return VK_FORMAT_R16_UNORM;
	case TF_R32_UINT: return VK_FORMAT_R32_UINT;
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

static inline enum NeTextureFormat
VkToNeTextureFormat(VkFormat fmt)
{
	switch (fmt) {
	case VK_FORMAT_R8G8B8A8_UNORM: return TF_R8G8B8A8_UNORM;
	case VK_FORMAT_R8G8B8A8_SRGB: return TF_R8G8B8A8_SRGB;
	case VK_FORMAT_B8G8R8A8_UNORM: return TF_B8G8R8A8_UNORM;
	case VK_FORMAT_B8G8R8A8_SRGB: return TF_B8G8R8A8_SRGB;
	case VK_FORMAT_R16G16B16A16_UNORM: return TF_R16G16B16A16_UNORM;
	case VK_FORMAT_R16G16B16A16_SFLOAT: return TF_R16G16B16A16_SFLOAT;
	case VK_FORMAT_R32G32B32A32_UINT: return TF_R32G32B32A32_UINT;
	case VK_FORMAT_R32G32B32A32_SFLOAT: return TF_R32G32B32A32_SFLOAT;
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return TF_A2R10G10B10_UNORM;
	case VK_FORMAT_D32_SFLOAT: return TF_D32_SFLOAT;
	case VK_FORMAT_D24_UNORM_S8_UINT: return TF_D24_STENCIL8;
	case VK_FORMAT_R8G8_UNORM: return TF_R8G8_UNORM;
	case VK_FORMAT_R16G16_UNORM: return TF_R16G16_UNORM;
	case VK_FORMAT_R32G32_UINT: return TF_R32G32_UINT;
	case VK_FORMAT_R8_UNORM: return TF_R8_UNORM;
	case VK_FORMAT_R16_UNORM: return TF_R16_UNORM;
	case VK_FORMAT_R32_UINT: return TF_R32_UINT;
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

static inline VkFormat
NeToVkVertexFormat(enum NeVertexFormat fmt)
{
	switch (fmt) {
	case VF_FLOAT: return VK_FORMAT_R32_SFLOAT;
	case VF_FLOAT2: return VK_FORMAT_R32G32_SFLOAT;
	case VF_FLOAT3: return VK_FORMAT_R32G32B32_SFLOAT;
	case VF_FLOAT4: return VK_FORMAT_R32G32B32A32_SFLOAT;
	default: return VK_FORMAT_R32_SFLOAT;
	}
}

static inline enum NeVertexFormat
VkToNeVertexFormat(VkFormat fmt)
{
	switch (fmt) {
	case VK_FORMAT_R32_SFLOAT: return VF_FLOAT;
	case VK_FORMAT_R32G32_SFLOAT: return VF_FLOAT2;
	case VK_FORMAT_R32G32B32_SFLOAT: return VF_FLOAT3;
	case VK_FORMAT_R32G32B32A32_SFLOAT: return VF_FLOAT4;
	default: return VF_FLOAT;
	}
}

static inline VkMemoryPropertyFlags
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
Vkd_AllocateCmdBuffer(VkDevice dev, VkCommandBufferLevel level, VkCommandPool pool)
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

	return cmdBuff;
}

static inline uint32_t
Vkd_MemoryTypeIndex(const struct NeRenderDevice *dev, uint32_t filter, VkMemoryPropertyFlags flags)
{
	for (uint32_t i = 0; i < dev->physDevMemProps.memoryTypeCount; ++i)
		if (/*(filter & (1 << i)) &&*/ ((dev->physDevMemProps.memoryTypes[i].propertyFlags & flags) == flags))
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

static inline VkResult
Vkd_QueueSubmit(struct VkdRenderQueue *q, uint32_t count, const VkSubmitInfo2KHR *submits, VkFence fence)
{
	Sys_LockFutex(q->ftx);
	VkResult rc = vkQueueSubmit2KHR(q->queue, count, submits, fence);
	Sys_UnlockFutex(q->ftx);

	return rc;
}

static inline void
Vkd_ExecuteCmdBuffer(struct NeRenderDevice *dev, VkCommandBuffer cb)
{
	vkEndCommandBuffer(cb);

	VkCommandBufferSubmitInfoKHR cbsi =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR,
		.commandBuffer = cb
	};
	VkSubmitInfo2KHR si =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cbsi
	};
	Vkd_QueueSubmit(&dev->transfer, 1, &si, dev->driverTransferFence);

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
NeToVkImageLayout(enum NeTextureLayout tl)
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
	case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL: return TL_DEPTH_READ_ONLY_ATTACHMENT;
	default: return TL_UNKNOWN;
	}
}

#endif /* VULKAN_BACKEND_H */

/* NekoEngine
 *
 * VulkanBackend.h
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
