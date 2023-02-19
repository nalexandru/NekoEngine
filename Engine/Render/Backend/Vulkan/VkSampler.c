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

/* NekoEngine
 *
 * VkSampler.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
