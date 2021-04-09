#include "VulkanDriver.h"

VkSampler
Vk_CreateSampler(struct RenderDevice *dev, const struct SamplerDesc *desc)
{
	VkSampler s;
	VkSamplerCreateInfo info =
	{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = desc->magFilter,
		.minFilter = desc->minFilter,
		.mipmapMode = desc->mipmapMode,
		.addressModeU = desc->addressModeU,
		.addressModeV = desc->addressModeV,
		.addressModeW = desc->addressModeW,
		.mipLodBias = desc->lodBias,
		.anisotropyEnable = desc->enableAnisotropy,
		.maxAnisotropy = desc->maxAnisotropy,
		.compareEnable = desc->enableCompare,
		.compareOp = (VkCompareOp)desc->compareOp,
		.minLod = desc->minLod,
		.maxLod = desc->maxLod,
		.borderColor = (VkBorderColor)desc->borderColor,
		.unnormalizedCoordinates = desc->unnormalizedCoordinates
	};
	if (vkCreateSampler(dev->dev, &info, Vkd_allocCb, &s) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	Vk_SetSampler(dev, 0, s);

	return s;
}

void
Vk_DestroySampler(struct RenderDevice *dev, VkSampler s)
{
	vkDestroySampler(dev->dev, s, Vkd_allocCb);
}
