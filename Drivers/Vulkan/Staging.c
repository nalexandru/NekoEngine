#include <Engine/Config.h>
#include <System/AtomicLock.h>

#include "VulkanDriver.h"

VkSemaphore Vkd_stagingSignal;

static uint64_t _size;
static VkBuffer _cpu, _gpu;
static VkDeviceMemory _cpuMem, _gpuMem;
static VkCommandBuffer _cmdBuffers[3];
static uint8_t *_memPtr;
static uint64_t _offset;
static struct NeAtomicLock _lock = { 0 };

#define ROUND_UP(v, powerOf2Alignment) (((v) + (powerOf2Alignment)-1) & ~((powerOf2Alignment)-1))

bool
Vkd_InitStagingArea(struct NeRenderDevice *dev)
{
	_size = E_GetCVarU64("VulkanDrv_NonCoherentStagingArea", 224 * 1024 * 1024)->u64;

	VkBufferCreateInfo bci =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		.size = _size
	};
	vkCreateBuffer(dev->dev, &bci, Vkd_allocCb, &_cpu);

	bci.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	vkCreateBuffer(dev->dev, &bci, Vkd_allocCb, &_gpu);

	VkMemoryRequirements mr;
	vkGetBufferMemoryRequirements(dev->dev, _cpu, &mr);

	VkMemoryDedicatedAllocateInfo dai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
		.buffer = _cpu
	};
	VkMemoryAllocateInfo ai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &dai,
		.allocationSize = mr.size,
		.memoryTypeIndex = Vkd_MemoryTypeIndex(dev, mr.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	};
	vkAllocateMemory(dev->dev, &ai, Vkd_allocCb, &_cpuMem);
	vkBindBufferMemory(dev->dev, _cpu, _cpuMem, 0);
	vkMapMemory(dev->dev, _cpuMem, 0, VK_WHOLE_SIZE, 0, (void **)&_memPtr);

	vkGetBufferMemoryRequirements(dev->dev, _gpu, &mr);
	
	VkMemoryAllocateFlagsInfo af =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
		.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
	};
	ai.pNext = &af;
	ai.memoryTypeIndex = Vkd_MemoryTypeIndex(dev, mr.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkAllocateMemory(dev->dev, &ai, Vkd_allocCb, &_gpuMem);
	vkBindBufferMemory(dev->dev, _gpu, _gpuMem, 0);

	VkSemaphoreCreateInfo sci = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	vkCreateSemaphore(dev->dev, &sci, Vkd_allocCb, &Vkd_stagingSignal);

#ifdef _DEBUG
	Vkd_SetObjectName(dev->dev, _cpu, VK_OBJECT_TYPE_BUFFER, "CPU Staging Area");
	Vkd_SetObjectName(dev->dev, _gpu, VK_OBJECT_TYPE_BUFFER, "GPU Staging Area");
	Vkd_SetObjectName(dev->dev, _cpuMem, VK_OBJECT_TYPE_DEVICE_MEMORY, "GPU Staging Area Memory");
	Vkd_SetObjectName(dev->dev, _gpuMem, VK_OBJECT_TYPE_DEVICE_MEMORY, "GPU Staging Area Memory");
	Vkd_SetObjectName(dev->dev, Vkd_stagingSignal, VK_OBJECT_TYPE_SEMAPHORE, "Staging Semaphore");
#endif

	return true;
}

void *
Vkd_AllocateStagingMemory(VkDevice dev, VkBuffer buff, VkMemoryRequirements *mr)
{
	Sys_AtomicLockWrite(&_lock);

	uint64_t start = ROUND_UP(_offset, mr->alignment);
	_offset = start + mr->size;

	vkBindBufferMemory(dev, buff, _gpuMem, start);

	Sys_AtomicUnlockWrite(&_lock);

	return _memPtr + start;
}

void
Vkd_CommitStagingArea(struct NeRenderDevice *dev, VkSemaphore wait)
{
	if (_cmdBuffers[Re_frameId])
		vkFreeCommandBuffers(dev->dev, dev->driverTransferPool, 1, &_cmdBuffers[Re_frameId]);
	_cmdBuffers[Re_frameId] = Vkd_TransferCmdBuffer(dev);

#ifdef _DEBUG
	Vkd_SetObjectName(dev->dev, _cmdBuffers[Re_frameId], VK_OBJECT_TYPE_COMMAND_BUFFER, "Staging CmdBuffer");
#endif

	VkMemoryBarrier memBarrier =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT
	};
	vkCmdPipelineBarrier(_cmdBuffers[Re_frameId], VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
							VK_DEPENDENCY_BY_REGION_BIT, 1, &memBarrier, 0, NULL, 0, NULL);

	VkBufferCopy c = { .size = _offset };
	vkCmdCopyBuffer(_cmdBuffers[Re_frameId], _cpu, _gpu, 1, &c);

	vkEndCommandBuffer(_cmdBuffers[Re_frameId]);

	VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkSubmitInfo si =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &wait,
		.pWaitDstStageMask = &waitFlags,
		.commandBufferCount = 1,
		.pCommandBuffers = &_cmdBuffers[Re_frameId],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &Vkd_stagingSignal
	};
	vkQueueSubmit(dev->transferQueue, 1, &si, VK_NULL_HANDLE);
}

void
Vkd_StagingBarrier(VkCommandBuffer cmdBuffer)
{
	VkBufferMemoryBarrier stagingBarrier =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
		.srcQueueFamilyIndex = Re_device->computeFamily,
		.dstQueueFamilyIndex = Re_device->graphicsFamily,
		.buffer = _gpu,
		.offset = 0,
		.size = _offset 
	};
	vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
		VK_DEPENDENCY_BY_REGION_BIT, 0, NULL, 1, &stagingBarrier, 0, NULL);
}

void
Vkd_TermStagingArea(struct NeRenderDevice *dev)
{
	vkDestroySemaphore(dev->dev, Vkd_stagingSignal, Vkd_allocCb);

	vkFreeCommandBuffers(dev->dev, dev->driverTransferPool, 3, _cmdBuffers);

	vkUnmapMemory(dev->dev, _cpuMem);
	vkFreeMemory(dev->dev, _gpuMem, Vkd_allocCb);
	vkFreeMemory(dev->dev, _cpuMem, Vkd_allocCb);

	vkDestroyBuffer(dev->dev, _gpu, Vkd_allocCb);
	vkDestroyBuffer(dev->dev, _cpu, Vkd_allocCb);
}
