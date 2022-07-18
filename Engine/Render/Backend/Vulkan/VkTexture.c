#include <stdlib.h>

#include "VulkanBackend.h"

bool
Vk_CreateImage(const struct NeTextureDesc *desc, struct NeTexture *tex, bool alias)
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

#ifdef _DEBUG
	if (desc->name)
		Vkd_SetObjectName(Re_device->dev, tex->image, VK_OBJECT_TYPE_IMAGE, desc->name);
#endif

	return vkCreateImage(Re_device->dev, &imageInfo, tex->transient ? Vkd_transientAllocCb : Vkd_allocCb, &tex->image) == VK_SUCCESS;
}

bool
Vk_CreateImageView(const struct NeTextureDesc *desc, struct NeTexture *tex)
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

#ifdef _DEBUG
	if (desc->name)
		Vkd_SetObjectName(Re_device->dev, tex->imageView, VK_OBJECT_TYPE_IMAGE_VIEW, desc->name);
#endif

	return vkCreateImageView(Re_device->dev, &viewInfo, tex->transient ? Vkd_transientAllocCb : Vkd_allocCb, &tex->imageView) == VK_SUCCESS;
}

struct NeTexture *
Re_BkCreateTexture(const struct NeTextureDesc *desc, uint16_t location)
{
	struct NeTexture *tex = Sys_Alloc(1, sizeof(*tex), MH_RenderDriver);
	if (!tex)
		return NULL;

	tex->transient = false;

	if (!Vk_CreateImage(desc, tex, false))
		goto error;

	VkMemoryRequirements req = { 0 };
	vkGetImageMemoryRequirements(Re_device->dev, tex->image, &req);

	VkMemoryDedicatedAllocateInfo dai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
		.image = tex->image
	};

	VkMemoryAllocateInfo ai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &dai,
		.allocationSize = req.size,
		.memoryTypeIndex = Vkd_MemoryTypeIndex(Re_device, req.memoryTypeBits, NeToVkMemoryProperties(desc->memoryType))
	};
	vkAllocateMemory(Re_device->dev, &ai, Vkd_allocCb, &tex->memory);
	vkBindImageMemory(Re_device->dev, tex->image, tex->memory, 0);

	if (!Vk_CreateImageView(desc, tex))
		goto error;

	Vk_SetTexture(location, tex->imageView);

	VkCommandBuffer cb = Vkd_TransferCmdBuffer(Re_device);
	Vkd_TransitionImageLayout(cb, tex->image, VK_IMAGE_LAYOUT_UNDEFINED, tex->layout);
	Vkd_ExecuteCmdBuffer(Re_device, cb);

#ifdef _DEBUG
	if (desc->name && tex->memory) {
		size_t tmpLen = strlen(desc->name) + 8;
		char *tmp = Sys_Alloc(sizeof(*tmp), tmpLen, MH_Transient);
		snprintf(tmp, tmpLen, "%s memory", desc->name);

		Vkd_SetObjectName(Re_device->dev, tex->memory, VK_OBJECT_TYPE_DEVICE_MEMORY, tmp);
	}
#endif

	return tex;

error:
	Sys_Free(tex);
	return NULL;
}

enum NeTextureLayout
Re_BkTextureLayout(const struct NeTexture *tex)
{
	return VkToNeImageLayout(tex->layout);
}

void
Re_BkDestroyTexture(struct NeTexture *tex)
{
	if (!tex->transient) {
		vkDestroyImageView(Re_device->dev, tex->imageView, Vkd_allocCb);
		vkDestroyImage(Re_device->dev, tex->image, Vkd_allocCb);
		vkFreeMemory(Re_device->dev, tex->memory, Vkd_allocCb);
	} else {
		vkDestroyImageView(Re_device->dev, tex->imageView, Vkd_transientAllocCb);
		vkDestroyImage(Re_device->dev, tex->image, Vkd_transientAllocCb);
	}

	Sys_Free(tex);
}
