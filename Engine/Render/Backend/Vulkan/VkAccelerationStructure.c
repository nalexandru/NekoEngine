#include "VulkanBackend.h"

struct NeAccelerationStructure *
Re_CreateAccelerationStructure(const struct NeAccelerationStructureCreateInfo *info)
{
	struct NeAccelerationStructure *as = Sys_Alloc(1, sizeof(*as), MH_RenderBackend);
	if (!as)
		return NULL;

	VkAccelerationStructureCreateInfoKHR vkInfo =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		.buffer = VK_NULL_HANDLE,
		.offset = 0, // multiple of 256
		.size = 0
	};

	switch (info->desc.type) {
	case AS_TOP_LEVEL: vkInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR; break;
	case AS_BOTTOM_LEVEL: vkInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR; break;
	}

	if (vkCreateAccelerationStructureKHR(Re_device->dev, &vkInfo, Vkd_allocCb, &as->as) != VK_SUCCESS) {
		Sys_Free(as);
		return NULL;
	}

	return as;
}

uint64_t
Re_AccelerationStructureBuildSize(const struct NeAccelerationStructure *as)
{
	VkAccelerationStructureBuildSizesInfoKHR sizeInfo = { .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };
	vkGetAccelerationStructureBuildSizesKHR(Re_device->dev, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		NULL /* build info */, NULL /* maxPrimitiveCount */, &sizeInfo);
	return 0;
}

uint64_t
Re_AccelerationStructureHandle(const struct NeAccelerationStructure *as)
{
	VkAccelerationStructureDeviceAddressInfoKHR info =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
		.accelerationStructure = as->as
	};
	return vkGetAccelerationStructureDeviceAddressKHR(Re_device->dev, &info);
}

void
Re_DestroyAccelerationStructure(struct NeAccelerationStructure *as)
{
	vkDestroyAccelerationStructureKHR(Re_device->dev, as->as, Vkd_allocCb);
	Sys_Free(as);
}

/* NekoEngine
 *
 * VkAccelerationStructure.c
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
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
