#include "D3D12Driver.h"

D3D12_SAMPLER_DESC *
D3D12_CreateSampler(struct NeRenderDevice *dev, const struct NeSamplerDesc *desc)
{
	D3D12_SAMPLER_DESC *s = Sys_Alloc(sizeof(*s), 1, MH_RenderDriver);

  /*  D3D12_FILTER Filter;
    D3D12_TEXTURE_ADDRESS_MODE AddressU;
    D3D12_TEXTURE_ADDRESS_MODE AddressV;
    D3D12_TEXTURE_ADDRESS_MODE AddressW;
    FLOAT MipLODBias;
    UINT MaxAnisotropy;
    D3D12_COMPARISON_FUNC ComparisonFunc;
    FLOAT BorderColor[ 4 ];
    FLOAT MinLOD;
    FLOAT MaxLOD;*/

/*	VkSampler s;
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
		return VK_NULL_HANDLE;*/

//	D3D12_SetSampler(dev, 0, s);

	return s;
}

void
D3D12_DestroySampler(struct NeRenderDevice *dev, D3D12_SAMPLER_DESC *s)
{
	Sys_Free(s);
}
