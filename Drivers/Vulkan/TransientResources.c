#include <Engine/Config.h>

#include "VulkanDriver.h"

/*static VkDeviceMemory _heapMemory;
static VkDeviceSize *_heapSize, _heapOffset, _peakSize;*/

struct Texture *
Vk_CreateTransientTexture(struct RenderDevice *dev, const struct TextureCreateInfo *tci, uint64_t offset)
{
	struct Texture *tex = Sys_Alloc(1, sizeof(*tex), MH_RenderDriver);
	if (!tex)
		return NULL;

	if (!Vk_CreateImage(dev, tci, tex, true))
		goto error;

	vkBindImageMemory(dev->dev, tex->image, dev->transientHeap, offset);

	if (!Vk_CreateImageView(dev, tci, tex))
		goto error;

error:
	Sys_Free(tex);
	return NULL;
}

struct Buffer *
Vk_CreateTransientBuffer(struct RenderDevice *dev, const struct BufferCreateInfo *bci, uint64_t offset)
{
	return NULL;
}

bool
Vk_InitTransientHeap(struct RenderDevice *dev, uint64_t size)
{
	VkMemoryAllocateInfo ai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = size,
		.memoryTypeIndex = 0
	};
	return vkAllocateMemory(dev->dev, &ai, Vkd_allocCb, &dev->transientHeap) == VK_SUCCESS;
}

bool
Vk_ResizeTransientHeap(struct RenderDevice *dev, uint64_t size)
{
	return false;
}

void
Vk_TermTransientHeap(struct RenderDevice *dev)
{
	vkFreeMemory(dev->dev, dev->transientHeap, Vkd_allocCb);
}
