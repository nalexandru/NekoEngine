#include <Engine/Config.h>

#include "VulkanDriver.h"

struct Texture *
Vk_CreateTransientTexture(struct RenderDevice *dev, const struct TextureDesc *desc, uint16_t location, uint64_t offset)
{
	struct Texture *tex = Sys_Alloc(1, sizeof(*tex), MH_Frame);
	if (!tex)
		return NULL;

	if (!Vk_CreateImage(dev, desc, tex, true))
		goto error;

	vkBindImageMemory(dev->dev, tex->image, dev->transientHeap, offset);

	if (!Vk_CreateImageView(dev, desc, tex))
		goto error;

	if (location)
			Vk_SetTexture(dev, location, tex->imageView);

	return tex;

error:
	if (tex->image)
		vkDestroyImage(dev->dev, tex->image, Vkd_allocCb);
	Sys_Free(tex);
	return NULL;
}

struct Buffer *
Vk_CreateTransientBuffer(struct RenderDevice *dev, const struct BufferDesc *desc, uint16_t location, uint64_t offset)
{
	struct Buffer *buff = Sys_Alloc(1, sizeof(*buff), MH_Frame);
	if (!buff)
		return NULL;

	VkBufferCreateInfo buffInfo =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = desc->size,
		.usage = desc->usage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	vkCreateBuffer(dev->dev, &buffInfo, Vkd_allocCb, &buff->buff);

	vkBindBufferMemory(dev->dev, buff->buff, dev->transientHeap, offset);

	if (location)
		Vk_SetBuffer(dev, location, buff->buff);

	return buff;
}

bool
Vk_InitTransientHeap(struct RenderDevice *dev, uint64_t size)
{
	VkMemoryAllocateInfo ai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = size,
		.memoryTypeIndex = Vkd_MemoryTypeIndex(dev, 0x90, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) // this *might* break
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
