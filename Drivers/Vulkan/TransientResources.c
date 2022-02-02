#include <Engine/Config.h>

#include "VulkanDriver.h"

#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))

struct NeTexture *
Vk_CreateTransientTexture(struct NeRenderDevice *dev, const struct NeTextureDesc *desc, uint16_t location, uint64_t offset, uint64_t *size)
{
	struct NeTexture *tex = Sys_Alloc(1, sizeof(*tex), MH_Frame);
	if (!tex)
		return NULL;

	tex->transient = true;

	if (!Vk_CreateImage(dev, desc, tex, true))
		goto error;

	VkMemoryRequirements mr;
	vkGetImageMemoryRequirements(dev->dev, tex->image, &mr);

	uint64_t realOffset = ROUND_UP(offset, mr.alignment);
	*size = mr.size + realOffset - offset;

	vkBindImageMemory(dev->dev, tex->image, dev->transientHeap, realOffset);

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

struct NeBuffer *
Vk_CreateTransientBuffer(struct NeRenderDevice *dev, const struct NeBufferDesc *desc, uint16_t location, uint64_t offset, uint64_t *size)
{
	struct NeBuffer *buff = Sys_Alloc(1, sizeof(*buff), MH_Frame);
	if (!buff)
		return NULL;

	buff->transient = true;

	VkBufferCreateInfo buffInfo =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = desc->size,
		.usage = desc->usage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	vkCreateBuffer(dev->dev, &buffInfo, Vkd_transientAllocCb, &buff->buff);

	VkMemoryRequirements mr;
	vkGetBufferMemoryRequirements(dev->dev, buff->buff, &mr);

	uint64_t realOffset = ROUND_UP(offset, mr.alignment);
	*size = mr.size + realOffset - offset;

	vkBindBufferMemory(dev->dev, buff->buff, dev->transientHeap, realOffset);

#ifdef _DEBUG
	if (desc->name)
		Vkd_SetObjectName(dev->dev, buff->buff, VK_OBJECT_TYPE_BUFFER, desc->name);
#endif

	return buff;
}

bool
Vk_InitTransientHeap(struct NeRenderDevice *dev, uint64_t size)
{
	VkMemoryAllocateFlagsInfo fi =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
		.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
	};
	VkMemoryAllocateInfo ai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &fi,
		.allocationSize = size,
		.memoryTypeIndex = Vkd_MemoryTypeIndex(dev, 0x90, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) // this *might* break
	};
	if (vkAllocateMemory(dev->dev, &ai, Vkd_allocCb, &dev->transientHeap) != VK_SUCCESS)
		return false;

#ifdef _DEBUG
	Vkd_SetObjectName(dev->dev, dev->transientHeap, VK_OBJECT_TYPE_DEVICE_MEMORY, "Transient Memory Heap");
#endif

	return true;
}

bool
Vk_ResizeTransientHeap(struct NeRenderDevice *dev, uint64_t size)
{
	return false;
}

void
Vk_TermTransientHeap(struct NeRenderDevice *dev)
{
	vkFreeMemory(dev->dev, dev->transientHeap, Vkd_allocCb);
}
