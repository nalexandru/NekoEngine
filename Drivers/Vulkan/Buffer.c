#include <stdlib.h>

#include "VulkanDriver.h"

struct Buffer *
Vk_CreateBuffer(struct RenderDevice *dev, struct BufferCreateInfo *bci)
{
	struct Buffer *buff = malloc(sizeof(*buff));
	if (!buff)
		return NULL;

	VkBufferCreateInfo buffInfo =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = bci->desc.size,
		.usage = bci->desc.usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	vkCreateBuffer(dev->dev, &buffInfo, Vkd_allocCb, &buff->buff);
	
	if (buffInfo.usage & BU_AS_BUILD_INPUT || buffInfo.usage & BU_AS_STORAGE || buffInfo.usage & BU_SHADER_BINDING_TABLE)
		buffInfo.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	
	VkMemoryRequirements req = { 0 };
	vkGetBufferMemoryRequirements(dev->dev, buff->buff, &req);

	VkMemoryAllocateInfo ai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = req.size,
		.memoryTypeIndex = Vkd_MemoryTypeIndex(dev, req.memoryTypeBits, NeToVkMemoryProperties(bci->desc.memoryType))
	};
	vkAllocateMemory(dev->dev, &ai, Vkd_allocCb, &buff->memory);

	vkBindBufferMemory(dev->dev, buff->buff, buff->memory, 0);

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
