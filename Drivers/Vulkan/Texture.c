#include <stdlib.h>

#include "VulkanDriver.h"

bool
Vk_CreateImage(struct RenderDevice *dev, const struct TextureDesc *desc, struct Texture *tex, bool alias)
{
	VkImageCreateInfo imageInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.format = NeToVkTextureFormat(desc->format),
		.extent =
		{
			.width = desc->width,
			.height = desc->height,
			.depth = desc->depth
		},
		.mipLevels = desc->mipLevels,
		.arrayLayers = desc->arrayLayers,
		.tiling = desc->gpuOptimalTiling ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR,
		.usage = desc->usage,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.samples = VK_SAMPLE_COUNT_1_BIT
	};

	switch (desc->type) {
	case TT_2D:
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
	break;
	case TT_3D:
		imageInfo.imageType = VK_IMAGE_TYPE_3D;
	break;
	case TT_2D_Multisample:
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
	//	imageInfo.samples = desc->sam
	break;
	case TT_Cube:
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	break;
	}

	tex->layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	return vkCreateImage(dev->dev, &imageInfo, Vkd_allocCb, &tex->image) == VK_SUCCESS;
}

bool
Vk_CreateImageView(struct RenderDevice *dev, const struct TextureDesc *desc, struct Texture *tex)
{
	VkImageViewCreateInfo viewInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.format = NeToVkTextureFormat(desc->format),
		.components = { 0, 0, 0, 0 },
		.subresourceRange =
		{
			.aspectMask = NeFormatAspect(desc->format),
			.baseMipLevel = 0,
			.levelCount = desc->mipLevels,
			.baseArrayLayer = 0,
			.layerCount = desc->arrayLayers
		},
		.image = tex->image
	};

	switch (desc->type) {
	case TT_2D:
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	break;
	case TT_3D:
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
	break;
	case TT_2D_Multisample:
	//	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	//	imageInfo.samples = desc->sam
	break;
	case TT_Cube:
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	break;
	}

	return vkCreateImageView(dev->dev, &viewInfo, Vkd_allocCb, &tex->imageView) == VK_SUCCESS;
}

struct Texture *
Vk_CreateTexture(struct RenderDevice *dev, const struct TextureDesc *desc, uint16_t location)
{
	struct Texture *tex = Sys_Alloc(1, sizeof(*tex), MH_RenderDriver);
	if (!tex)
		return NULL;

	if (!Vk_CreateImage(dev, desc, tex, false))
		goto error;

	VkMemoryRequirements req = { 0 };
	vkGetImageMemoryRequirements(dev->dev, tex->image, &req);

	VkMemoryAllocateInfo ai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = req.size,
		.memoryTypeIndex = Vkd_MemoryTypeIndex(dev, req.memoryTypeBits, NeToVkMemoryProperties(desc->memoryType))
	};
	vkAllocateMemory(dev->dev, &ai, Vkd_allocCb, &tex->memory);
	vkBindImageMemory(dev->dev, tex->image, tex->memory, 0);

	if (!Vk_CreateImageView(dev, desc, tex))
		goto error;

	Vk_SetTexture(dev, location, tex->imageView);

	VkCommandBuffer cb = Vkd_TransferCmdBuffer(dev);
	Vkd_TransitionImageLayout(cb, tex->image, VK_IMAGE_LAYOUT_UNDEFINED, tex->layout);
	Vkd_ExecuteCmdBuffer(dev, cb);

	return tex;

error:
	Sys_Free(tex);
	return NULL;
}

enum TextureLayout
Vk_TextureLayout(const struct Texture *tex)
{
	return VkToNeImageLayout(tex->layout);
}

void
Vk_DestroyTexture(struct RenderDevice *dev, struct Texture *tex)
{
	vkDestroyImageView(dev->dev, tex->imageView, Vkd_allocCb);
	vkDestroyImage(dev->dev, tex->image, Vkd_allocCb);
	vkFreeMemory(dev->dev, tex->memory, Vkd_allocCb);

	Sys_Free(tex);
}
