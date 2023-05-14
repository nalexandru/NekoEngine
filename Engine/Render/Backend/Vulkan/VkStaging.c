#include <Engine/Config.h>
#include <System/AtomicLock.h>

#include "VulkanBackend.h"

struct NeSemaphore Vkd_stagingSignal;

static uint64_t f_size;
static VkBuffer f_cpu, f_gpu;
static VkDeviceMemory f_cpuMem, f_gpuMem;
static VkCommandBuffer f_cmdBuffers[3];
static uint8_t *f_memPtr;
static uint64_t f_offset;
static struct NeAtomicLock f_lock = {0 };

bool
VkBk_InitStagingArea(struct NeRenderDevice *dev)
{
	f_size = E_GetCVarU64("Vulkan_NonCoherentStagingArea", 224 * 1024 * 1024)->u64;

	VkBufferCreateInfo bci =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		.size = f_size
	};
	vkCreateBuffer(dev->dev, &bci, Vkd_allocCb, &f_cpu);

	bci.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	vkCreateBuffer(dev->dev, &bci, Vkd_allocCb, &f_gpu);

	VkMemoryRequirements mr;
	vkGetBufferMemoryRequirements(dev->dev, f_cpu, &mr);

	const VkMemoryDedicatedAllocateInfo dai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO,
		.buffer = f_cpu
	};
	VkMemoryAllocateInfo ai =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = &dai,
		.allocationSize = mr.size,
		.memoryTypeIndex = Vkd_MemoryTypeIndex(dev, mr.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
	};
	vkAllocateMemory(dev->dev, &ai, Vkd_allocCb, &f_cpuMem);
	vkBindBufferMemory(dev->dev, f_cpu, f_cpuMem, 0);
	vkMapMemory(dev->dev, f_cpuMem, 0, VK_WHOLE_SIZE, 0, (void **)&f_memPtr);

	vkGetBufferMemoryRequirements(dev->dev, f_gpu, &mr);
	
	const VkMemoryAllocateFlagsInfo af =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
		.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
	};
	ai.pNext = &af;
	ai.memoryTypeIndex = Vkd_MemoryTypeIndex(dev, mr.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkAllocateMemory(dev->dev, &ai, Vkd_allocCb, &f_gpuMem);
	vkBindBufferMemory(dev->dev, f_gpu, f_gpuMem, 0);

	const VkSemaphoreTypeCreateInfo typeInfo =
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
		.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
		.initialValue = 0
	};
	const VkSemaphoreCreateInfo semInfo =
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = &typeInfo
	};
	vkCreateSemaphore(dev->dev, &semInfo, Vkd_allocCb, &Vkd_stagingSignal.sem);

#ifdef _DEBUG
	VkBk_SetObjectName(dev->dev, f_cpu, VK_OBJECT_TYPE_BUFFER, "CPU Staging Area");
	VkBk_SetObjectName(dev->dev, f_gpu, VK_OBJECT_TYPE_BUFFER, "GPU Staging Area");
	VkBk_SetObjectName(dev->dev, f_cpuMem, VK_OBJECT_TYPE_DEVICE_MEMORY, "CPU Staging Area Memory");
	VkBk_SetObjectName(dev->dev, f_gpuMem, VK_OBJECT_TYPE_DEVICE_MEMORY, "GPU Staging Area Memory");
	VkBk_SetObjectName(dev->dev, Vkd_stagingSignal.sem, VK_OBJECT_TYPE_SEMAPHORE, "Staging Semaphore");
#endif

	return true;
}

void *
VkBk_AllocateStagingMemory(VkDevice dev, VkBuffer buff, VkMemoryRequirements *mr)
{
	Sys_AtomicLockWrite(&f_lock);

	uint64_t start = NE_ROUND_UP(f_offset, mr->alignment);
	f_offset = start + mr->size;

	vkBindBufferMemory(dev, buff, f_gpuMem, start);

	Sys_AtomicUnlockWrite(&f_lock);

	return f_memPtr + start;
}

void
VkBk_CommitStagingArea(struct NeRenderDevice *dev, VkSemaphore wait)
{
	if (f_cmdBuffers[Re_frameId])
		vkFreeCommandBuffers(dev->dev, dev->driverTransferPool, 1, &f_cmdBuffers[Re_frameId]);
	f_cmdBuffers[Re_frameId] = Vkd_TransferCmdBuffer(dev);

#ifdef _DEBUG
	VkBk_SetObjectName(dev->dev, f_cmdBuffers[Re_frameId], VK_OBJECT_TYPE_COMMAND_BUFFER, "Staging CmdBuffer");
#endif

	const VkMemoryBarrier memBarrier =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT
	};
	vkCmdPipelineBarrier(f_cmdBuffers[Re_frameId], VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
							VK_DEPENDENCY_BY_REGION_BIT, 1, &memBarrier, 0, NULL, 0, NULL);

	const VkBufferCopy c = { .size = f_offset };
	vkCmdCopyBuffer(f_cmdBuffers[Re_frameId], f_cpu, f_gpu, 1, &c);

	vkEndCommandBuffer(f_cmdBuffers[Re_frameId]);

	const VkCommandBufferSubmitInfoKHR cbsi =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR,
		.commandBuffer = f_cmdBuffers[Re_frameId]
	};

	const VkSemaphoreSubmitInfoKHR wssi =
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR,
		.semaphore = wait,
		.stageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR
	};

	const VkSemaphoreSubmitInfoKHR sssi =
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR,
		.semaphore = Vkd_stagingSignal.sem,
		.value = ++Vkd_stagingSignal.value
	};

	const VkSubmitInfo2KHR si =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR,
		.pCommandBufferInfos = &cbsi,
		.commandBufferInfoCount = 1,
		.pWaitSemaphoreInfos = &wssi,
		.waitSemaphoreInfoCount = 1,
		.pSignalSemaphoreInfos = &sssi,
		.signalSemaphoreInfoCount = 1
	};
	Vkd_QueueSubmit(&dev->transfer, 1, &si, VK_NULL_HANDLE);
}

void
VkBk_StagingBarrier(VkCommandBuffer cmdBuffer)
{
	const VkBufferMemoryBarrier stagingBarrier =
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
		.srcQueueFamilyIndex = Re_device->compute.family,
		.dstQueueFamilyIndex = Re_device->graphics.family,
		.buffer = f_gpu,
		.offset = 0,
		.size = f_offset
	};
	vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
		VK_DEPENDENCY_BY_REGION_BIT, 0, NULL, 1, &stagingBarrier, 0, NULL);
}

void
VkBk_TermStagingArea(struct NeRenderDevice *dev)
{
	vkDestroySemaphore(dev->dev, Vkd_stagingSignal.sem, Vkd_allocCb);

	vkFreeCommandBuffers(dev->dev, dev->driverTransferPool, 3, f_cmdBuffers);

	vkUnmapMemory(dev->dev, f_cpuMem);
	vkFreeMemory(dev->dev, f_gpuMem, Vkd_allocCb);
	vkFreeMemory(dev->dev, f_cpuMem, Vkd_allocCb);

	vkDestroyBuffer(dev->dev, f_gpu, Vkd_allocCb);
	vkDestroyBuffer(dev->dev, f_cpu, Vkd_allocCb);
}

/* NekoEngine
 *
 * VkStaging.c
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
