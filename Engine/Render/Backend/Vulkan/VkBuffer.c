#include <stdlib.h>

#include "VulkanBackend.h"

struct NeBuffer *
Re_BkCreateBuffer(const struct NeBufferDesc *desc, uint16_t location)
{
	struct NeBuffer *buff = Sys_Alloc(1, sizeof(*buff), MH_RenderDriver);
	if (!buff)
		return NULL;

	VkBufferCreateInfo buffInfo =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = desc->size,
		.usage = desc->usage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	vkCreateBuffer(Re_device->dev, &buffInfo, Vkd_allocCb, &buff->buff);

	VkMemoryRequirements req = { 0 };
	vkGetBufferMemoryRequirements(Re_device->dev, buff->buff, &req);

	if (desc->memoryType == MT_CPU_COHERENT && !Re_deviceInfo.features.coherentMemory) {
		buff->staging = Vkd_AllocateStagingMemory(Re_device->dev, buff->buff, &req);
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
			.memoryTypeIndex = Vkd_MemoryTypeIndex(Re_device, req.memoryTypeBits, NeToVkMemoryProperties(desc->memoryType))
		};
		vkAllocateMemory(Re_device->dev, &ai, Vkd_allocCb, &buff->memory);
		vkBindBufferMemory(Re_device->dev, buff->buff, buff->memory, 0);
	}

#ifdef _DEBUG
	if (desc->name) {
		Vkd_SetObjectName(Re_device->dev, buff->buff, VK_OBJECT_TYPE_BUFFER, desc->name);
	
		if (buff->memory) {
			size_t tmpLen = strlen(desc->name) + 8;
			char *tmp = Sys_Alloc(sizeof(*tmp), tmpLen, MH_Transient);
			snprintf(tmp, tmpLen, "%s memory", desc->name);

			Vkd_SetObjectName(Re_device->dev, buff->memory, VK_OBJECT_TYPE_DEVICE_MEMORY, tmp);
		}
	}
#endif

	return buff;
}

void
Re_BkUpdateBuffer(struct NeBuffer *buff, uint64_t offset, void *data, uint64_t size)
{
	if (size < 65535) {
		VkCommandBuffer cb = Vkd_TransferCmdBuffer(Re_device);
		vkCmdUpdateBuffer(cb, buff->buff, offset, size, data);
		Vkd_ExecuteCmdBuffer(Re_device, cb);
	} else {
		VkBufferCreateInfo stagingInfo =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = size,
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
		};

		VkBuffer staging;
		vkCreateBuffer(Re_device->dev, &stagingInfo, Vkd_allocCb, &staging);

		VkMemoryRequirements bMemReq;
		vkGetBufferMemoryRequirements(Re_device->dev, staging, &bMemReq);

		VkMemoryAllocateInfo bMemAI =
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = bMemReq.size,
			.memoryTypeIndex = Vkd_MemoryTypeIndex(Re_device, bMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
		};

		VkDeviceMemory stagingMem;
		vkAllocateMemory(Re_device->dev, &bMemAI, Vkd_allocCb, &stagingMem);
		vkBindBufferMemory(Re_device->dev, staging, stagingMem, 0);

		void *stagingData = NULL;
		vkMapMemory(Re_device->dev, stagingMem, 0, VK_WHOLE_SIZE, 0, &stagingData);

		memcpy(stagingData, data, size);

		vkUnmapMemory(Re_device->dev, stagingMem);

		VkCommandBuffer cb = Vkd_TransferCmdBuffer(Re_device);

		VkBufferCopy copy =
		{
			.srcOffset = 0,
			.dstOffset = offset,
			.size = size
		};
		vkCmdCopyBuffer(cb, staging, buff->buff, 1, &copy);

		Vkd_ExecuteCmdBuffer(Re_device, cb);

		vkDestroyBuffer(Re_device->dev, staging, Vkd_allocCb);
		vkFreeMemory(Re_device->dev, stagingMem, Vkd_allocCb);
	}
}

void *
Re_BkMapBuffer(struct NeBuffer *buff)
{
	if (buff->staging)
		return buff->staging;

	void *mem;
	if (vkMapMemory(Re_device->dev, buff->memory, 0, VK_WHOLE_SIZE, 0, &mem) != VK_SUCCESS)
		return NULL;
	else
		return mem;
}

void
Re_BkFlushBuffer(struct NeBuffer *buff, uint64_t offset, uint64_t size)
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
	vkFlushMappedMemoryRanges(Re_device->dev, 1, &mr);
}

void
Re_BkUnmapBuffer(struct NeBuffer *buff)
{
	if (buff->memory)
		vkUnmapMemory(Re_device->dev, buff->memory);
}

uint64_t
Re_BkBufferAddress(const struct NeBuffer *buff, uint64_t offset)
{
	VkBufferDeviceAddressInfo bdai =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = buff->buff
	};
	VkDeviceAddress addr = vkGetBufferDeviceAddress(Re_device->dev, &bdai);
	return addr ? addr + offset : 0;
}

uint64_t
Re_OffsetAddress(uint64_t addr, uint64_t offset)
{
	return addr + offset;
}

void
Re_BkDestroyBuffer(struct NeBuffer *buff)
{
	if (!buff->transient) {
		vkDestroyBuffer(Re_device->dev, buff->buff, Vkd_allocCb);
		vkFreeMemory(Re_device->dev, buff->memory, Vkd_allocCb);
	} else {
		vkDestroyBuffer(Re_device->dev, buff->buff, Vkd_transientAllocCb);
	}

	Sys_Free(buff);
}
