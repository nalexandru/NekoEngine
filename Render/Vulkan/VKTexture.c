#include <Render/Texture.h>
#include <Render/Device.h>

#include "VKRender.h"

static VkDescriptorPool _texturePool;
static VkDescriptorSet _textureSet;
static VkFormat _textureFormat[] =
{
	VK_FORMAT_R8G8B8A8_UNORM,
	VK_FORMAT_R8G8B8A8_SRGB,
	VK_FORMAT_B8G8R8A8_UNORM,
	VK_FORMAT_B8G8R8A8_SRGB,
	VK_FORMAT_B8G8R8A8_UNORM,
	VK_FORMAT_B8G8R8A8_SRGB,
	VK_FORMAT_R16G16B16A16_SFLOAT,
	VK_FORMAT_R16G16B16A16_UNORM,
	VK_FORMAT_R32G32B32A32_SFLOAT,
	VK_FORMAT_R32G32B32A32_UINT,
	VK_FORMAT_A2R10G10B10_UNORM_PACK32,
	VK_FORMAT_R32G32B32_SFLOAT,
	VK_FORMAT_R32G32B32_UINT,
	VK_FORMAT_R8G8_UNORM,
	VK_FORMAT_R16G16_SFLOAT,
	VK_FORMAT_R16G16_UNORM,
	VK_FORMAT_R32G32_SFLOAT,
	VK_FORMAT_R32G32_UINT,
	VK_FORMAT_R8_UNORM,
	VK_FORMAT_R16_SFLOAT,
	VK_FORMAT_R16_UNORM,
	VK_FORMAT_R32_SFLOAT,
	VK_FORMAT_R32_UINT,
	VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
	VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
	VK_FORMAT_BC2_UNORM_BLOCK,
	VK_FORMAT_BC2_SRGB_BLOCK,
	VK_FORMAT_BC3_UNORM_BLOCK,
	VK_FORMAT_BC3_SRGB_BLOCK,
	VK_FORMAT_BC4_UNORM_BLOCK,
	VK_FORMAT_BC4_SNORM_BLOCK,
	VK_FORMAT_BC5_UNORM_BLOCK,
	VK_FORMAT_BC5_SNORM_BLOCK,
	VK_FORMAT_BC6H_UFLOAT_BLOCK,
	VK_FORMAT_BC6H_SFLOAT_BLOCK,
	VK_FORMAT_BC7_UNORM_BLOCK,
	VK_FORMAT_BC7_SRGB_BLOCK,
	VK_FORMAT_R8_UNORM
};

bool
VK_InitTexture(const char *name, struct Texture *tex, Handle h)
{
	struct TextureRenderData *trd = (struct TextureRenderData *)&tex->renderDataStart;

/*	if (!_freeSlots)
		return false;*/

	if (tex->format >= _countof(_textureFormat))
		return false;

	trd->format = _textureFormat[tex->format];

	VkImageCreateInfo ici =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.format = trd->format,
		.mipLevels = tex->levels,
		.arrayLayers = 1,
		.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.extent = 
		{
			.width = tex->width,
			.height = tex->height,
			.depth = tex->depth
		}
	};
	VkImageViewCreateInfo ivci =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.format = ici.format,
		.image = trd->image,
		.subresourceRange =
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.levelCount = ici.mipLevels,
			.layerCount = ici.arrayLayers
		}
	};

	switch (tex->type) {
	case TT_2D:
		ici.imageType = VK_IMAGE_TYPE_2D;
		ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	break;
	case TT_3D:
		ici.imageType = VK_IMAGE_TYPE_3D;
		ivci.viewType = VK_IMAGE_VIEW_TYPE_3D;
	break;
	case TT_Cube:
		ici.imageType = VK_IMAGE_TYPE_2D;
		ici.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		ici.arrayLayers = 6;
		ivci.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		ivci.subresourceRange.layerCount = 6;
	break;
	}

	vkCreateImage(Re_Device.dev, &ici, NULL, &trd->image);

	// memory
	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(Re_Device.dev, trd->image, &memReq);

	VkMemoryAllocateInfo mai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memReq.size,
		.memoryTypeIndex = VK_MemoryTypeIndex(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};
	VkDeviceMemory mem;
	vkAllocateMemory(Re_Device.dev, &mai, NULL, &mem);

	vkBindImageMemory(Re_Device.dev, trd->image, mem, 0);

	ivci.image = trd->image;
	vkCreateImageView(Re_Device.dev, &ivci, NULL, &trd->view);


	return false;
}

bool
VK_UpdateTexture(struct Texture *tex, const void *data, uint64_t offset, uint64_t size)
{
	return false;
}

void
VK_TermTexture(struct Texture *tex)
{
	//
}
