#include <Engine/Config.h>
#include <Runtime/Runtime.h>
#include <System/AtomicLock.h>

#include "VulkanDriver.h"

bool
Vk_CreateDescriptorSet(struct RenderDevice *dev)
{
	VkDescriptorSetLayoutBinding bindings[] =
	{
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
			.descriptorCount = 3,
			.stageFlags = VK_SHADER_STAGE_ALL,
		},
		{
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = UINT16_MAX,
			.stageFlags = VK_SHADER_STAGE_ALL,
		},
		{
			.binding = 2,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = UINT16_MAX,
			.stageFlags = VK_SHADER_STAGE_ALL
		}
	};
	VkDescriptorBindingFlags bindingFlags[] = 
	{
		VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
		VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
		VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
	};
	VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
		.bindingCount = sizeof(bindingFlags) / sizeof(bindingFlags[0]),
		.pBindingFlags = bindingFlags
	};
	VkDescriptorSetLayoutCreateInfo dslInfo =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
		.pNext = &flagsInfo,
		.bindingCount = sizeof(bindings) / sizeof(bindings[0]),
		.pBindings = bindings
	};
	if (vkCreateDescriptorSetLayout(dev->dev, &dslInfo, Vkd_allocCb, &dev->setLayout) != VK_SUCCESS)
		return false;

	VkDescriptorPoolSize poolSize[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 3 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, UINT16_MAX },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, UINT16_MAX }
	};
	VkDescriptorPoolCreateInfo poolInfo =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
		.maxSets = 1,
		.poolSizeCount = sizeof(poolSize) / sizeof(poolSize[0]),
		.pPoolSizes = poolSize
	};
	if (vkCreateDescriptorPool(dev->dev, &poolInfo, Vkd_allocCb, &dev->descriptorPool) != VK_SUCCESS) {
		vkDestroyDescriptorSetLayout(dev->dev, dev->setLayout, Vkd_allocCb);
		return false;
	}

	VkDescriptorSetAllocateInfo allocInfo =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = dev->descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &dev->setLayout
	};
	if (vkAllocateDescriptorSets(dev->dev, &allocInfo, &dev->descriptorSet) != VK_SUCCESS) {
		vkDestroyDescriptorPool(dev->dev, dev->descriptorPool, Vkd_allocCb);
		vkDestroyDescriptorSetLayout(dev->dev, dev->setLayout, Vkd_allocCb);
		return false;
	}

	return true;
}

void
Vk_SetSampler(struct RenderDevice *dev, uint16_t location, VkSampler sampler)
{
	VkDescriptorImageInfo dii =
	{
		.sampler = sampler
	};
	VkWriteDescriptorSet wds =
	{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = dev->descriptorSet,
		.dstBinding = 0,
		.dstArrayElement = location,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
		.pImageInfo = &dii
	};
	vkUpdateDescriptorSets(dev->dev, 1, &wds, 0, NULL);
}

void
Vk_SetBuffer(struct RenderDevice *dev, uint16_t location, VkBuffer buffer)
{
	VkDescriptorBufferInfo dbi =
	{
		.buffer = buffer,
		.range = VK_WHOLE_SIZE
	};
	VkWriteDescriptorSet wds =
	{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = dev->descriptorSet,
		.dstBinding = 2,
		.dstArrayElement = location,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.pBufferInfo = &dbi
	};
	vkUpdateDescriptorSets(dev->dev, 1, &wds, 0, NULL);
}

void
Vk_SetTexture(struct RenderDevice *dev, uint16_t location, VkImageView imageView)
{
	VkDescriptorImageInfo dii =
	{
		.imageView = imageView,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};
	VkWriteDescriptorSet wds =
	{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = dev->descriptorSet,
		.dstBinding = 1,
		.dstArrayElement = location,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		.pImageInfo = &dii
	};
	vkUpdateDescriptorSets(dev->dev, 1, &wds, 0, NULL);
}

void
Vk_TermDescriptorSet(struct RenderDevice *dev)
{
	vkDestroyDescriptorPool(dev->dev, dev->descriptorPool, Vkd_allocCb);
	vkDestroyDescriptorSetLayout(dev->dev, dev->setLayout, Vkd_allocCb);
}

