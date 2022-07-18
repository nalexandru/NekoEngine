#include "VulkanBackend.h"

struct NeSampler *
Re_CreateSampler(const struct NeSamplerDesc *desc)
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
	if (vkCreateSampler(Re_device->dev, &info, Vkd_allocCb, &s) != VK_SUCCESS)
		return VK_NULL_HANDLE;

	Vk_SetSampler(Re_device, 0, s);

#ifdef _DEBUG
	if (desc->name)
		Vkd_SetObjectName(Re_device->dev, s, VK_OBJECT_TYPE_SAMPLER, desc->name);
#endif

	return (struct NeSampler *)s;
}

void
Re_DestroySampler(struct NeSampler *s)
{
	vkDestroySampler(Re_device->dev, (VkSampler)s, Vkd_allocCb);
}
