#include <stdlib.h>

#include "VulkanDriver.h"

struct Buffer *
Vk_CreateBuffer(struct RenderDevice *dev, const struct BufferCreateInfo *bci, uint16_t location)
{
	struct Buffer *buff = Sys_Alloc(1, sizeof(*buff), MH_RenderDriver);
	if (!buff)
		return NULL;

	VkBufferCreateInfo buffInfo =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = bci->desc.size,
		.usage = bci->desc.usage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	vkCreateBuffer(dev->dev, &buffInfo, Vkd_allocCb, &buff->buff);

	VkMemoryRequirements req = { 0 };
	vkGetBufferMemoryRequirements(dev->dev, buff->buff, &req);

	VkMemoryAllocateFlagsInfo fi =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
		.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
	};
	VkMemoryAllocateInfo ai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &fi,
		.allocationSize = req.size,
		.memoryTypeIndex = Vkd_MemoryTypeIndex(dev, req.memoryTypeBits, NeToVkMemoryProperties(bci->desc.memoryType))
	};
	vkAllocateMemory(dev->dev, &ai, Vkd_allocCb, &buff->memory);

	vkBindBufferMemory(dev->dev, buff->buff, buff->memory, 0);

	Vk_SetBuffer(dev, location, buff->buff);

	if (!bci->data)
		return buff;

	VkCommandBuffer cb = Vkd_TransferCmdBuffer(dev);

	vkCmdUpdateBuffer(cb, buff->buff, 0, bci->dataSize, bci->data);

	Vkd_ExecuteCmdBuffer(dev, cb);

	return buff;
}

void
Vk_UpdateBuffer(struct RenderDevice *dev, struct Buffer *buff, uint64_t offset, void *data, uint64_t size)
{
	//
}

void *
Vk_MapBuffer(struct RenderDevice *dev, struct Buffer *buff)
{
	void *mem;
	if (vkMapMemory(dev->dev, buff->memory, 0, VK_WHOLE_SIZE, 0, &mem) != VK_SUCCESS)
		return NULL;
	else
		return mem;
}

void
Vk_UnmapBuffer(struct RenderDevice *dev, struct Buffer *buff)
{
	vkUnmapMemory(dev->dev, buff->memory);
}

uint64_t
Vk_BufferAddress(struct RenderDevice *dev, const struct Buffer *buff, uint64_t offset)
{
	VkBufferDeviceAddressInfo bdai =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = buff->buff
	};
	VkDeviceAddress addr = vkGetBufferDeviceAddress(dev->dev, &bdai);
	return addr ? addr + offset : 0;
}

void
Vk_DestroyBuffer(struct RenderDevice *dev, struct Buffer *buff)
{
	vkDestroyBuffer(dev->dev, buff->buff, Vkd_allocCb);
	vkFreeMemory(dev->dev, buff->memory, Vkd_allocCb);

	Sys_Free(buff);
}
