#include "VulkanDriver.h"

VkSemaphore
Vk_CreateSemaphore(struct RenderDevice *dev)
{
	return VK_NULL_HANDLE;
}

void
Vk_DestroySemaphore(struct RenderDevice *dev, VkSemaphore *s)
{

}

VkFence
Vk_CreateFence(struct RenderDevice *dev, bool createSignaled)
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
Vk_SignalFence(struct RenderDevice *dev, VkFence f)
{
}

bool
Vk_WaitForFence(struct RenderDevice *dev, VkFence f, uint64_t timeout)
{
	return vkWaitForFences(dev->dev, 1, &f, VK_TRUE, timeout) == VK_SUCCESS;
}

void
Vk_DestroyFence(struct RenderDevice *dev, VkFence f)
{
	vkDestroyFence(dev->dev, f, Vkd_allocCb);
}
