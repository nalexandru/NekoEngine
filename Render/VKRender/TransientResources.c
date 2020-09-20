#include <System/Log.h>
#include <Engine/Config.h>
#include <Render/Render.h>
#include <Render/Device.h>

#include "VKRender.h"

#define VKTRMOD L"VulkanTransient"

static VkDeviceMemory _transientHeapMemory;
static VkDeviceSize *_heapSize, _heapOffset, _peakSize;

bool
VK_InitTransientHeap(void)
{
	_heapSize = &E_GetCVarU64(L"vkTransientHeapSize", 64 * 1024 * 1024)->u64;

	VkMemoryAllocateInfo ai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = *_heapSize,
		.memoryTypeIndex = Re_Device.deviceLocalMemoryType
	};
	VkResult rc = vkAllocateMemory(Re_Device.dev, &ai, VK_CPUAllocator, &_transientHeapMemory);
	if (rc != VK_SUCCESS)
		return false;

	return true;
}

VkBuffer
VK_CreateTransientBuffer(VkBufferCreateInfo *bci)
{
	VkBuffer buffer;
	VkResult rc = vkCreateBuffer(Re_Device.dev, bci, VK_CPUAllocator, &buffer);
	if (rc != VK_SUCCESS)
		return VK_NULL_HANDLE;

	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(Re_Device.dev, buffer, &memReq);

	_heapOffset = ROUND_UP(_heapOffset, memReq.alignment);
	rc = vkBindBufferMemory(Re_Device.dev, buffer, _transientHeapMemory, _heapOffset);
	if (rc != VK_SUCCESS) {
		vkDestroyBuffer(Re_Device.dev, buffer, VK_CPUAllocator);
		return VK_NULL_HANDLE;
	}

	_heapOffset += memReq.size;

	return buffer;
}

VkImage
VK_CreateTransientImage(VkImageCreateInfo *ici)
{
	VkImage image;
	VkResult rc = vkCreateImage(Re_Device.dev, ici, VK_CPUAllocator, &image);
	if (rc != VK_SUCCESS)
		return VK_NULL_HANDLE;

	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(Re_Device.dev, image, &memReq);

	_heapOffset = ROUND_UP(_heapOffset, memReq.alignment);
	rc = vkBindImageMemory(Re_Device.dev, image, _transientHeapMemory, _heapOffset);
	if (rc != VK_SUCCESS) {
		vkDestroyImage(Re_Device.dev, image, VK_CPUAllocator);
		return VK_NULL_HANDLE;
	}

	_heapOffset += memReq.size;

	return image;
}

VkAccelerationStructureKHR
VK_CreateTransientAccelerationStructure(VkAccelerationStructureCreateInfoKHR *asci)
{
	VkAccelerationStructureKHR as;
	VkResult rc = vkCreateAccelerationStructureKHR(Re_Device.dev, asci, VK_CPUAllocator, &as);
	if (rc != VK_SUCCESS)
		return VK_NULL_HANDLE;

	VkAccelerationStructureMemoryRequirementsInfoKHR mri =
	{
		.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR,
		.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR,
		.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		.accelerationStructure = as
	};
	VkMemoryRequirements2 memReq =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2
	};
	vkGetAccelerationStructureMemoryRequirementsKHR(Re_Device.dev, &mri, &memReq);

	_heapOffset = ROUND_UP(_heapOffset, memReq.memoryRequirements.alignment);
	VkBindAccelerationStructureMemoryInfoKHR bindInfo =
	{
		.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR,
		.accelerationStructure = as,
		.memory = _transientHeapMemory,
		.memoryOffset = _heapOffset
	};
	rc = vkBindAccelerationStructureMemoryKHR(Re_Device.dev, 1, &bindInfo);
	if (rc != VK_SUCCESS) {
		vkDestroyAccelerationStructureKHR(Re_Device.dev, as, VK_CPUAllocator);
		return VK_NULL_HANDLE;
	}

	_heapOffset += memReq.memoryRequirements.size;

	return as;
}

void
VK_ResetTransientHeap(void)
{
	_peakSize = max(_peakSize, _heapOffset);
	_heapOffset = 0;
}

void
VK_TermTransientHeap(void)
{
	Sys_LogEntry(VKTRMOD, LOG_DEBUG, L"Peak heap size: %llu B", _peakSize);

	vkFreeMemory(Re_Device.dev, _transientHeapMemory, VK_CPUAllocator);
}
