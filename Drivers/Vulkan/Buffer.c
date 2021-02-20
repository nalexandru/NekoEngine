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

	// TODO: add VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT where needed


	return buff;
}

void Vk_UpdateBuffer(struct RenderDevice* dev, struct Buffer* buff, uint64_t offset, void* data, uint64_t size)
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
