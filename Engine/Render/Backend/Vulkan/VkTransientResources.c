#include <Engine/Config.h>

#include "VulkanBackend.h"

#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))

struct NeTexture *
Re_CreateTransientTexture(const struct NeTextureDesc *desc, uint16_t location, uint64_t offset, uint64_t *size)
{
	struct NeTexture *tex = Sys_Alloc(1, sizeof(*tex), MH_Frame);
	if (!tex)
		return NULL;

	tex->transient = true;

	if (!Vk_CreateImage(desc, tex, true))
		goto error;

	VkMemoryRequirements mr;
	vkGetImageMemoryRequirements(Re_device->dev, tex->image, &mr);

	uint64_t realOffset = ROUND_UP(offset, mr.alignment);
	*size = mr.size + realOffset - offset;

	vkBindImageMemory(Re_device->dev, tex->image, Re_device->transientHeap, realOffset);

	if (!Vk_CreateImageView(desc, tex))
		goto error;

	if (location)
			Vk_SetTexture(location, tex->imageView);

	return tex;

error:
	if (tex->image)
		vkDestroyImage(Re_device->dev, tex->image, Vkd_allocCb);
	
	Sys_Free(tex);

	return NULL;
}

struct NeBuffer *
Re_BkCreateTransientBuffer(const struct NeBufferDesc *desc, uint16_t location, uint64_t offset, uint64_t *size)
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
	vkCreateBuffer(Re_device->dev, &buffInfo, Vkd_transientAllocCb, &buff->buff);

	VkMemoryRequirements mr;
	vkGetBufferMemoryRequirements(Re_device->dev, buff->buff, &mr);

	uint64_t realOffset = ROUND_UP(offset, mr.alignment);
	*size = mr.size + realOffset - offset;

	vkBindBufferMemory(Re_device->dev, buff->buff, Re_device->transientHeap, realOffset);

#ifdef _DEBUG
	if (desc->name)
		Vkd_SetObjectName(Re_device->dev, buff->buff, VK_OBJECT_TYPE_BUFFER, desc->name);
#endif

	return buff;
}

bool
Re_InitTransientHeap(uint64_t size)
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
		.memoryTypeIndex = Vkd_MemoryTypeIndex(Re_device, 0x90, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) // this *might* break
	};
	if (vkAllocateMemory(Re_device->dev, &ai, Vkd_allocCb, &Re_device->transientHeap) != VK_SUCCESS)
		return false;

#ifdef _DEBUG
	Vkd_SetObjectName(Re_device->dev, Re_device->transientHeap, VK_OBJECT_TYPE_DEVICE_MEMORY, "Transient Memory Heap");
#endif

	return true;
}

bool
Re_ResizeTransientHeap(uint64_t size)
{
	return false;
}

void
Re_TermTransientHeap(void)
{
	vkFreeMemory(Re_device->dev, Re_device->transientHeap, Vkd_allocCb);
}
