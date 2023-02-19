#include "VulkanBackend.h"

struct NeSemaphore *
Re_CreateSemaphore(void)
{
	struct NeSemaphore *s = Sys_Alloc(sizeof(*s), 1, MH_RenderDriver);
	if (!s)
		return NULL;

	VkSemaphoreTypeCreateInfo stci =
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
		.initialValue = 0,
		.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE
	};
	VkSemaphoreCreateInfo sci =
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = &stci
	};
	if (vkCreateSemaphore(Re_device->dev, &sci, Vkd_allocCb, &s->sem) != VK_SUCCESS) {
		Sys_Free(s);
		s = NULL;
	}

	return s;
}

bool
Re_WaitSemaphore(struct NeSemaphore *s, uint64_t value, uint64_t timeout)
{
	struct VkSemaphoreWaitInfo wi =
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
		.semaphoreCount = 1,
		.pSemaphores = &s->sem,
		.pValues = &value
	};
	return vkWaitSemaphores(Re_device->dev, &wi, timeout) == VK_SUCCESS;
}

bool
Re_WaitSemaphores(uint32_t count, struct NeSemaphore *s, uint64_t *values, uint64_t timeout)
{
	VkSemaphore *sem = Sys_Alloc(sizeof(*sem), count, MH_Frame);
	if (!sem)
		return false;

	struct VkSemaphoreWaitInfo wi =
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
		.semaphoreCount = count,
		.pSemaphores = sem,
		.pValues = values
	};
	return vkWaitSemaphores(Re_device->dev, &wi, timeout) == VK_SUCCESS;
}

bool
Re_SignalSemaphore(struct NeSemaphore *s, uint64_t value)
{
	struct VkSemaphoreSignalInfo si =
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
		.semaphore = s->sem,
		.value = value
	};
	return vkSignalSemaphore(Re_device->dev, &si) == VK_SUCCESS;
}

void
Re_DestroySemaphore(struct NeSemaphore *s)
{
	vkDestroySemaphore(Re_device->dev, s->sem, Vkd_allocCb);
	Sys_Free(s);
}

struct NeFence *
Re_CreateFence(bool createSignaled)
{
	struct VkFenceCreateInfo fci =
	{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = createSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0
	};
	VkFence f;
	if (vkCreateFence(Re_device->dev, &fci, Vkd_allocCb, &f) != VK_SUCCESS)
		return VK_NULL_HANDLE;
	return (struct NeFence *)f;
}

void
Re_SignalFence(struct NeRenderDevice *dev, struct NeFence *f)
{
}

bool
Re_WaitForFence(struct NeFence *f, uint64_t timeout)
{
	return vkWaitForFences(Re_device->dev, 1, (VkFence *)&f, VK_TRUE, timeout) == VK_SUCCESS;
}

void
Re_DestroyFence(struct NeFence *f)
{
	vkDestroyFence(Re_device->dev, (VkFence)f, Vkd_allocCb);
}

/* NekoEngine
 *
 * VkSynchronization.c
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
