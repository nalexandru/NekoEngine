#include <stdlib.h>

#include "VulkanDriver.h"

struct Buffer *
Vk_CreateBuffer(struct RenderDevice *dev, struct BufferCreateInfo *bci)
{
	struct Buffer *buff = malloc(sizeof(*buff));
	if (!buff)
		return NULL;

	VkMemoryPropertyFlags memFlags;
	switch (bci->desc.memoryType) {
	case MT_CPU_READ: memFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT; break;
	case MT_CPU_WRITE: memFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT; break;
	case MT_CPU_COHERENT: memFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; break;
	case MT_GPU_LOCAL:
	default: memFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; break;
	}

	VkBufferCreateInfo buffInfo =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = bci->desc.size,
		.usage = bci->desc.usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	vkCreateBuffer(dev->dev, &buffInfo, Vkd_allocCb, &buff->buff);

	VkMemoryRequirements req = { 0 };
	vkGetBufferMemoryRequirements(dev->dev, buff->buff, &req);

	VkMemoryAllocateInfo ai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = req.size,
		.memoryTypeIndex = Vkd_MemoryTypeIndex(dev, req.memoryTypeBits, memFlags)
	};
	vkAllocateMemory(dev->dev, &ai, Vkd_allocCb, &buff->memory);

	vkBindBufferMemory(dev->dev, buff->buff, buff->memory, 0);

	// TODO: add VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT where needed

	if (!bci->data)
		return buff;

	VkCommandBuffer cb = Vkd_TransferCmdBuffer(dev);

	vkCmdUpdateBuffer(cb, buff->buff, 0, bci->dataSize, bci->data);

	Vkd_ExecuteCmdBuffer(dev, cb);

	return buff;
}

void Vk_UpdateBuffer(struct RenderDevice *dev, struct Buffer *buff, uint64_t offset, void *data, uint64_t size)
{
	//
}

const struct BufferDesc *
Vk_BufferDesc(const struct Buffer *buff)
{
	return &buff->desc;
}

void
Vk_DestroyBuffer(struct RenderDevice *dev, struct Buffer *buff)
{
	vkDestroyBuffer(dev->dev, buff->buff, Vkd_allocCb);
	free(buff);
}
