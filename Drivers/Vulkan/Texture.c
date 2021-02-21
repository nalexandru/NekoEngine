#include <stdlib.h>

#include "VulkanDriver.h"

struct Texture *
Vk_CreateTexture(struct RenderDevice *dev, struct TextureCreateInfo *tci)
{
	struct Texture *tex = malloc(sizeof(*tex));
	if (!tex)
		return NULL;

	VkImageCreateInfo imageInfo =
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
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};
	VkImageViewCreateInfo viewInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.format = NeToVkTextureFormat(tci->desc.format),
		.components = { 0, 0, 0, 0 },
		.subresourceRange =
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = tci->desc.mipLevels,
			.baseArrayLayer = 0,
			.layerCount = tci->desc.arrayLayers
		}
	};
	
	switch (tci->desc.type) {
	case TT_2D:
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	break;
	case TT_3D:
		imageInfo.imageType = VK_IMAGE_TYPE_3D;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
	break;
	case TT_2D_Multisample:
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
	//	imageInfo.samples = tci->desc.sam
	break;
	case TT_Cube:
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		viewInfo.sType = VK_IMAGE_VIEW_TYPE_CUBE;
	break;
	}

	vkCreateImage(dev->dev, &imageInfo, Vkd_allocCb, &tex->image);

	viewInfo.image = tex->image;
	vkCreateImageView(dev->dev, &viewInfo, Vkd_allocCb, &tex->imageView);

	memcpy(&tex->desc, &tci->desc, sizeof(tex->desc));

	return tex;
}

const struct TextureDesc *
Vk_TextureDesc(const struct Texture *tex)
{
	return &tex->desc;
}

void
Vk_DestroyTexture(struct RenderDevice *dev, struct Texture *tex)
{
	vkDestroyImageView(dev->dev, tex->imageView, Vkd_allocCb);
	vkDestroyImage(dev->dev, tex->image, Vkd_allocCb);
	free(tex);
}

