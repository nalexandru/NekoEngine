#ifndef _VK_RENDER_H_
#define _VK_RENDER_H_

#define VK_ENABLE_BETA_EXTENSIONS
#include "volk.h"

#include <assert.h>

#include <Engine/Types.h>
#include <Render/Render.h>
#include <Render/Device.h>
#include <Runtime/Runtime.h>

#define PREPARE_SCENE_DATA_SYS		L"VKPrepareInstanceBuffer"

// From: https://nvpro-samples.github.io/vk_raytracing_tutorial_KHR/vkrt_tuto_manyhits.md.htm
#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))

struct Swapchain
{
	VkSwapchainKHR sw;
	VkSurfaceKHR surface;
	VkExtent2D extent;
	VkSurfaceFormatKHR format;
	VkSurfaceCapabilitiesKHR caps;
	VkPresentModeKHR presentMode;
	VkImage *images;
	VkImageView *views;
	VkSemaphore *imgRdy, *imgDone;
	uint32_t imgCount;
};

struct RenderDevice
{
	uint32_t frame;
	VkDevice dev;
	VkSemaphore frameStart, frameEnd;
	VkPhysicalDevice physicalDevice;
	VkQueue graphicsQueue, transferQueue, computeQueue;
	uint32_t graphicsQueueFamily, transferQueueFamily, computeQueueFamily;
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	uint32_t deviceLocalMemoryType;
};

struct ModelRenderData
{
	struct ModelAccelerationStructure *structures;
	VkBuffer vertexBuffer, indexBuffer, scratchBuffer;
	VkDeviceAddress vertexAddress, indexAddress, scratchAddress;
	VkDeviceMemory memory;
};

struct SceneRenderData
{
	VkAccelerationStructureKHR topLevelAS;
	Array instanceData, materialData;
};

struct TextureRenderData
{
	VkImageView view;
	VkImage image;
	VkFormat format;
};

struct RenderWorker
{
	VkCommandPool *graphicsCmdPools[RE_NUM_BUFFERS];
	VkCommandPool *transferCmdPools[RE_NUM_BUFFERS];
	VkCommandPool *computeCmdPools[RE_NUM_BUFFERS];
};

struct ModelAccelerationStructure
{
	VkAccelerationStructureKHR as;
	VkDeviceAddress address;
	VkDeviceAddress scratchAddress;
};

extern VkInstance VK_Instance;
extern struct Swapchain VK_Swapchain;
extern VkAllocationCallbacks *VK_CPUAllocator;

bool VK_InitDevice(void);
bool VK_CreateSwapchain(void);
void VK_TermDevice(void);

static inline uint32_t
VK_MemoryTypeIndex(uint32_t filter, VkMemoryPropertyFlags flags)
{
	for (uint32_t i = 0; i < Re_Device.memoryProperties.memoryTypeCount; ++i)
		if ((filter & (1 << i)) && ((Re_Device.memoryProperties.memoryTypes[i].propertyFlags & flags) == flags))
			return i;

	assert(!"Failed to retrieve memory type");
	return 0;
}

extern struct RenderWorker Re_MainThreadWorker;

struct RenderWorker *VK_CurrentThreadWorker(void);
VkCommandBuffer VK_GraphicsCommandBuffer(int worker, VkCommandBufferLevel level);
VkCommandBuffer VK_TransferCommandBuffer(int worker, VkCommandBufferLevel level);
VkCommandBuffer VK_ComputeCommandBuffer(int worker, VkCommandBufferLevel level);

// Staging
void *VK_AllocStagingArea(VkDeviceSize size, VkDeviceSize alignment, VkDeviceAddress *gpuHandle);
void VK_StageBLASBuild(struct Model *model, VkBool32 update);

// Scene
void VK_BuildTLAS(VkCommandBuffer cmdBuffer, struct Scene *scene, struct Camera *cam);


// Swapchain


// Transient resources
bool VK_InitTransientHeap(void);
VkBuffer VK_CreateTransientBuffer(VkBufferCreateInfo *bci);
VkImage VK_CreateTransientImage(VkImageCreateInfo *ici);
VkAccelerationStructureKHR VK_CreateTransientAccelerationStructure(VkAccelerationStructureCreateInfoKHR *asci);
void VK_ResetTransientHeap(void);
void VK_TermTransientHeap(void);

// Platform specific
bool VK_CheckPresentSupport(VkPhysicalDevice dev, uint32_t family);
bool VK_CreateSurface(void);

// Helpers
static inline void
VK_TransitionImageRange(VkImage image, VkImageLayout oldLyt, VkImageLayout newLyt, VkImageSubresourceRange *range, VkCommandBuffer cmdBuff)
{
	VkPipelineStageFlags srcFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dstFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkImageMemoryBarrier barrier =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = oldLyt,
		.newLayout = newLyt,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange = *range
	};

	switch (oldLyt) {
	case VK_IMAGE_LAYOUT_PREINITIALIZED: {
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		srcFlags = VK_PIPELINE_STAGE_HOST_BIT;
	} break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		srcFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		srcFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		srcFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		srcFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		srcFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	break;
	default:
		barrier.srcAccessMask = 0;
	break;
	}

	switch (newLyt) {
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		dstFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		dstFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dstFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dstFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		dstFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	break;
	default:
		barrier.dstAccessMask = 0;
	break;
	}

	vkCmdPipelineBarrier(cmdBuff, srcFlags, dstFlags, 0, 0, NULL, 0, NULL, 1, &barrier);
}

static inline void
VK_TransitionImage(VkImage image, VkImageLayout oldLyt, VkImageLayout newLyt, VkImageAspectFlags aspect, VkCommandBuffer cmdBuff)
{
	VkImageSubresourceRange range =
	{
		.aspectMask = aspect,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};
	VK_TransitionImageRange(image, oldLyt, newLyt, &range, cmdBuff);
}

#endif /* _VK_RENDER_H_ */
