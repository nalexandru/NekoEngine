#include <stdlib.h>

#include "VulkanDriver.h"

struct Buffer *
Vk_CreateBuffer(struct RenderDevice *dev, const struct BufferDesc *desc, uint16_t location)
{
	struct Buffer *buff = Sys_Alloc(1, sizeof(*buff), MH_RenderDriver);
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

	VkMemoryRequirements req = { 0 };
	vkGetBufferMemoryRequirements(dev->dev, buff->buff, &req);

	if (desc->memoryType == MT_CPU_COHERENT && !Re_deviceInfo.features.coherentMemory) {
		buff->staging = Vkd_AllocateStagingMemory(dev->dev, buff->buff, &req);
	} else {
		VkMemoryDedicatedAllocateInfo dai =
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
			.buffer = buff->buff
		};
		VkMemoryAllocateFlagsInfo fi =
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
			.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
			.pNext = &dai
		};
		VkMemoryAllocateInfo ai =
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.pNext = &fi,
			.allocationSize = req.size,
			.memoryTypeIndex = Vkd_MemoryTypeIndex(dev, req.memoryTypeBits, NeToVkMemoryProperties(desc->memoryType))
		};
		vkAllocateMemory(dev->dev, &ai, Vkd_allocCb, &buff->memory);
		vkBindBufferMemory(dev->dev, buff->buff, buff->memory, 0);
	}

	Vk_SetBuffer(dev, location, buff->buff);

	return buff;
}

void
Vk_UpdateBuffer(struct RenderDevice *dev, struct Buffer *buff, uint64_t offset, void *data, uint64_t size)
{
	if (size < 65535) {
		VkCommandBuffer cb = Vkd_TransferCmdBuffer(dev);
		vkCmdUpdateBuffer(cb, buff->buff, offset, size, data);
		Vkd_ExecuteCmdBuffer(dev, cb);
	} else {
		VkBufferCreateInfo stagingInfo =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = size,
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
		};

		VkBuffer staging;
		vkCreateBuffer(dev->dev, &stagingInfo, Vkd_allocCb, &staging);

		VkMemoryRequirements bMemReq;
		vkGetBufferMemoryRequirements(dev->dev, staging, &bMemReq);

		VkMemoryAllocateInfo bMemAI =
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = bMemReq.size,
			.memoryTypeIndex = Vkd_MemoryTypeIndex(dev, bMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
		};

		VkDeviceMemory stagingMem;
		vkAllocateMemory(dev->dev, &bMemAI, Vkd_allocCb, &stagingMem);
		vkBindBufferMemory(dev->dev, staging, stagingMem, 0);

		void *stagingData = NULL;
		vkMapMemory(dev->dev, stagingMem, 0, VK_WHOLE_SIZE, 0, &stagingData);

		memcpy(stagingData, data, size);

		vkUnmapMemory(dev->dev, stagingMem);

		VkCommandBuffer cb = Vkd_TransferCmdBuffer(dev);

		VkBufferCopy copy =
		{
			.srcOffset = 0,
			.dstOffset = offset,
			.size = size
		};
		vkCmdCopyBuffer(cb, staging, buff->buff, 1, &copy);

		Vkd_ExecuteCmdBuffer(dev, cb);

		vkDestroyBuffer(dev->dev, staging, Vkd_allocCb);
		vkFreeMemory(dev->dev, stagingMem, Vkd_allocCb);
	}
}

void *
Vk_MapBuffer(struct RenderDevice *dev, struct Buffer *buff)
{
	if (buff->staging)
		return buff->staging;

	void *mem;
	if (vkMapMemory(dev->dev, buff->memory, 0, VK_WHOLE_SIZE, 0, &mem) != VK_SUCCESS)
		return NULL;
	else
		return mem;
}

void
Vk_FlushBuffer(struct RenderDevice *dev, struct Buffer *buff, uint64_t offset, uint64_t size)
{
	if (!buff->memory)
		return;

	VkMappedMemoryRange mr =
	{
		.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
		.memory = buff->memory,
		.offset = offset,
		.size = size
	};
	vkFlushMappedMemoryRanges(dev->dev, 1, &mr);
}

void
Vk_UnmapBuffer(struct RenderDevice *dev, struct Buffer *buff)
{
	if (buff->memory)
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

uint64_t
Vk_OffsetAddress(uint64_t addr, uint64_t offset)
{
	return addr + offset;
}

void
Vk_DestroyBuffer(struct RenderDevice *dev, struct Buffer *buff)
{
	if (!buff->transient) {
		vkDestroyBuffer(dev->dev, buff->buff, Vkd_allocCb);
		vkFreeMemory(dev->dev, buff->memory, Vkd_allocCb);
	} else {
		vkDestroyBuffer(dev->dev, buff->buff, Vkd_transientAllocCb);
	}

	Sys_Free(buff);
}
