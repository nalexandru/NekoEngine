#include "VulkanDriver.h"

struct NeSemaphore *
Vk_CreateSemaphore(struct NeRenderDevice *dev)
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
	if (vkCreateSemaphore(dev->dev, &sci, Vkd_allocCb, &s->sem) != VK_SUCCESS) {
		Sys_Free(s);
		s = NULL;
	}

	return s;
}

bool
Vk_WaitSemaphore(struct NeRenderDevice *dev, struct NeSemaphore *s, uint64_t value, uint64_t timeout)
{
	struct VkSemaphoreWaitInfo wi =
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
		.semaphoreCount = 1,
		.pSemaphores = &s->sem,
		.pValues = &value
	};
	return vkWaitSemaphores(dev->dev, &wi, timeout) == VK_SUCCESS;
}

bool
Vk_WaitSemaphores(struct NeRenderDevice *dev, uint32_t count, struct NeSemaphore *s, uint64_t *values, uint64_t timeout)
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
	return vkWaitSemaphores(dev->dev, &wi, timeout) == VK_SUCCESS;
}

bool
Vk_SignalSemaphore(struct NeRenderDevice *dev, struct NeSemaphore *s, uint64_t value)
{
	struct VkSemaphoreSignalInfo si =
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
		.semaphore = s->sem,
		.value = value
	};
	return vkSignalSemaphore(dev->dev, &si) == VK_SUCCESS;
}

void
Vk_DestroySemaphore(struct NeRenderDevice *dev, struct NeSemaphore *s)
{
	vkDestroySemaphore(dev->dev, s->sem, Vkd_allocCb);
	Sys_Free(s);
}

VkFence
Vk_CreateFence(struct NeRenderDevice *dev, bool createSignaled)
{
	struct VkFenceCreateInfo fci =
	{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = createSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0
	};
	VkFence f;
	if (vkCreateFence(dev->dev, &fci, Vkd_allocCb, &f) != VK_SUCCESS)
		return VK_NULL_HANDLE;
	return f;
}

void
Vk_SignalFence(struct NeRenderDevice *dev, VkFence f)
{
}

bool
Vk_WaitForFence(struct NeRenderDevice *dev, VkFence f, uint64_t timeout)
{
	return vkWaitForFences(dev->dev, 1, &f, VK_TRUE, timeout) == VK_SUCCESS;
}

void
Vk_DestroyFence(struct NeRenderDevice *dev, VkFence f)
{
	vkDestroyFence(dev->dev, f, Vkd_allocCb);
}
