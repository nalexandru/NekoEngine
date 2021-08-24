#include <stdlib.h>

#include "D3D12Driver.h"

bool
D3D12_CreateImage(struct RenderDevice *dev, const struct TextureDesc *desc, struct Texture *tex, bool alias)
{
/*	VkImageCreateInfo imageInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.format = NeToVkTextureFormat(tci->desc.format),
		.extent =
		{
			.width = tci->desc.width,
			.height = tci->desc.height,
			.depth = tci->desc.depth
		},
		.mipLevels = tci->desc.mipLevels,
		.arrayLayers = tci->desc.arrayLayers,
		.tiling = tci->desc.gpuOptimalTiling ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR,
		.usage = tci->desc.usage,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.samples = VK_SAMPLE_COUNT_1_BIT
	};

	switch (tci->desc.type) {
	case TT_2D:
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
	break;
	case TT_3D:
		imageInfo.imageType = VK_IMAGE_TYPE_3D;
	break;
	case TT_2D_Multisample:
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
	//	imageInfo.samples = tci->desc.sam
	break;
	case TT_Cube:
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	break;
	}

	memcpy(&tex->desc, &tci->desc, sizeof(tex->desc));
	tex->layout = VK_IMAGE_LAYOUT_UNDEFINED;

	return vkCreateImage(dev->dev, &imageInfo, Vkd_allocCb, &tex->image) == VK_SUCCESS;*/
	return false;
}

bool
D3D12_CreateImageView(struct RenderDevice *dev, const struct TextureDesc *desc, struct Texture *tex)
{
/*	VkImageViewCreateInfo viewInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.format = NeToVkTextureFormat(tci->desc.format),
		.components = { 0, 0, 0, 0 },
		.subresourceRange =
		{
			.aspectMask = NeFormatAspect(tci->desc.format),
			.baseMipLevel = 0,
			.levelCount = tci->desc.mipLevels,
			.baseArrayLayer = 0,
			.layerCount = tci->desc.arrayLayers
		},
		.image = tex->image
	};

	switch (tci->desc.type) {
	case TT_2D:
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	break;
	case TT_3D:
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
	break;
	case TT_2D_Multisample:
	//	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	//	imageInfo.samples = tci->desc.sam
	break;
	case TT_Cube:
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	break;
	}

	return vkCreateImageView(dev->dev, &viewInfo, Vkd_allocCb, &tex->imageView) == VK_SUCCESS;*/
	return false;
}

struct Texture *
D3D12_CreateTexture(struct RenderDevice *dev, const struct TextureDesc *desc, uint16_t location)
{
	struct Texture *tex = Sys_Alloc(1, sizeof(*tex), MH_RenderDriver);
	if (!tex)
		return NULL;

/*	if (!D3D12_CreateImage(dev, tci, tex, false))
		goto error;

	VkMemoryRequirements req = { 0 };
	vkGetImageMemoryRequirements(dev->dev, tex->image, &req);

	VkMemoryAllocateInfo ai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = req.size,
		.memoryTypeIndex = Vkd_MemoryTypeIndex(dev, req.memoryTypeBits, NeToVkMemoryProperties(tci->desc.memoryType))
	};
	vkAllocateMemory(dev->dev, &ai, Vkd_allocCb, &tex->memory);
	vkBindImageMemory(dev->dev, tex->image, tex->memory, 0);

	if (!D3D12_CreateImageView(dev, tci, tex))
		goto error;

	D3D12_SetTexture(dev, location, tex->imageView);

	// FIXME

	if (tci->data) {
		VkBufferCreateInfo bci =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = tci->dataSize,
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
		};

		VkBuffer staging;
		vkCreateBuffer(dev->dev, &bci, Vkd_allocCb, &staging);

		VkMemoryRequirements bMemReq;
		vkGetBufferMemoryRequirements(dev->dev, staging, &bMemReq);

		VkMemoryAllocateInfo bMemAI =
		{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = bMemReq.size,
			.memoryTypeIndex = Vkd_MemoryTypeIndex(dev, req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
		};

		VkDeviceMemory stagingMem;
		vkAllocateMemory(dev->dev, &bMemAI, Vkd_allocCb, &stagingMem);

		vkBindBufferMemory(dev->dev, staging, stagingMem, 0);

		void *stagingData = NULL;
		vkMapMemory(dev->dev, stagingMem, 0, VK_WHOLE_SIZE, 0, &stagingData);

		memcpy(stagingData, tci->data, tci->dataSize);

		vkUnmapMemory(dev->dev, stagingMem);

		VkCommandBuffer cmdBuffer = Vkd_TransferCmdBuffer(dev);

		VkBufferImageCopy copy =
		{
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource =
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			.imageOffset = { 0, 0, 0 },
			.imageExtent = { tci->desc.width, tci->desc.height, tci->desc.depth }
		};

		Vkd_TransitionImageLayout(cmdBuffer, tex->image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		vkCmdCopyBufferToImage(cmdBuffer, staging, tex->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
		Vkd_TransitionImageLayout(cmdBuffer, tex->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		Vkd_ExecuteCmdBuffer(dev, cmdBuffer);

		tex->layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		vkDestroyBuffer(dev->dev, staging, Vkd_allocCb);
		vkFreeMemory(dev->dev, stagingMem, Vkd_allocCb);
	}

	// FIXME*/

	return tex;

//error:
//	Sys_Free(tex);
//	return NULL;
}

enum TextureLayout
D3D12_TextureLayout(const struct Texture *tex)
{
	return 0;// VkToNeImageLayout(tex->layout);
}

void
D3D12_DestroyTexture(struct RenderDevice *dev, struct Texture *tex)
{
/*	vkDestroyImageView(dev->dev, tex->imageView, Vkd_allocCb);
	vkDestroyImage(dev->dev, tex->image, Vkd_allocCb);
	vkFreeMemory(dev->dev, tex->memory, Vkd_allocCb);*/

	Sys_Free(tex);
}
