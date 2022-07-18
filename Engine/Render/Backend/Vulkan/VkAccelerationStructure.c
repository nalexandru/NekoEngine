#include "VulkanBackend.h"

struct NeAccelerationStructure *
Re_CreateAccelerationStructure(const struct NeAccelerationStructureCreateInfo *info)
{
	struct NeAccelerationStructure *as = Sys_Alloc(1, sizeof(*as), MH_RenderDriver);
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
