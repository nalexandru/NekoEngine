#include "D3D12Driver.h"

struct NeAccelerationStructure *
D3D12_CreateAccelerationStructure(struct NeRenderDevice *dev, const struct NeAccelerationStructureCreateInfo *info)
{
	struct NeAccelerationStructure *as = Sys_Alloc(1, sizeof(*as), MH_RenderDriver);
	if (!as)
		return NULL;

/*	VkAccelerationStructureCreateInfoKHR vkInfo =
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

	if (vkCreateAccelerationStructureKHR(dev->dev, &vkInfo, Vkd_allocCb, &as->as) != VK_SUCCESS) {
		Sys_Free(as);
		return NULL;
	}*/

	return as;
}

uint64_t
D3D12_AccelerationStructureHandle(struct NeRenderDevice *dev, const struct NeAccelerationStructure *as)
{
	return ID3D12Resource_GetGPUVirtualAddress(as->buffer);
}

void
D3D12_DestroyAccelerationStructure(struct NeRenderDevice *dev, struct NeAccelerationStructure *as)
{
	ID3D12Resource_Release(as->buffer);
	Sys_Free(as);
}
