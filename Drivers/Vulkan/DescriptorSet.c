#include <Engine/Config.h>
#include <Runtime/Runtime.h>
#include <System/AtomicLock.h>

#include "VulkanDriver.h"

bool
Vk_CreateDescriptorSet(struct NeRenderDevice *dev)
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
	};
	VkDescriptorBindingFlags bindingFlags[] = 
	{
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
	if (vkCreateDescriptorPool(dev->dev, &poolInfo, Vkd_allocCb, &dev->descriptorPool) != VK_SUCCESS)
		goto error;

	VkDescriptorSetAllocateInfo allocInfo =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = dev->descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &dev->setLayout
	};
	if (vkAllocateDescriptorSets(dev->dev, &allocInfo, &dev->descriptorSet) != VK_SUCCESS)
		goto error;

	// Input attachment set layout & pool
	VkDescriptorSetLayoutBinding iaBindings[] =
	{
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
			.descriptorCount = 4,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
		}
	};
	VkDescriptorBindingFlags iaBindingFlags[] = { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT };
	VkDescriptorSetLayoutBindingFlagsCreateInfo iaFlagsInfo =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
		.bindingCount = sizeof(iaBindingFlags) / sizeof(iaBindingFlags[0]),
		.pBindingFlags = iaBindingFlags
	};
	VkDescriptorSetLayoutCreateInfo iaDslInfo =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = &iaFlagsInfo,
		.bindingCount = sizeof(iaBindings) / sizeof(iaBindings[0]),
		.pBindings = iaBindings
	};
	if (vkCreateDescriptorSetLayout(dev->dev, &iaDslInfo, Vkd_allocCb, &dev->iaSetLayout) != VK_SUCCESS)
		goto error;

	uint32_t inputAttachmentPoolSize = E_GetCVarU32("Vulkan_InputAttachmentPoolSize", 32)->i32;
	VkDescriptorPoolSize iaPoolSize[] =
	{
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, inputAttachmentPoolSize }
	};
	VkDescriptorPoolCreateInfo iaPoolInfo =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = inputAttachmentPoolSize,
		.poolSizeCount = sizeof(iaPoolSize) / sizeof(iaPoolSize[0]),
		.pPoolSizes = iaPoolSize
	};
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		if (vkCreateDescriptorPool(dev->dev, &iaPoolInfo, Vkd_allocCb, &dev->iaDescriptorPool[i]) != VK_SUCCESS)
			goto error;

#ifdef _DEBUG
	Vkd_SetObjectName(dev->dev, dev->descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "Global Descriptor Pool");
	Vkd_SetObjectName(dev->dev, dev->descriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET, "Global Descriptor Set");
	Vkd_SetObjectName(dev->dev, dev->setLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Global Descriptor Set Layout");
	Vkd_SetObjectName(dev->dev, dev->iaSetLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Input Attachment Descriptor Set Layout");

	char *tmp = Sys_Alloc(sizeof(*tmp), 64, MH_Transient);
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		snprintf(tmp, 64, "Input Attachment Descriptor Pool %d", i);
		Vkd_SetObjectName(dev->dev, dev->iaDescriptorPool[i], VK_OBJECT_TYPE_DESCRIPTOR_POOL, tmp);
	}
#endif

	return true;

error:
	if (dev->setLayout)
		vkDestroyDescriptorSetLayout(dev->dev, dev->setLayout, Vkd_allocCb);

	if (dev->iaSetLayout)
		vkDestroyDescriptorSetLayout(dev->dev, dev->iaSetLayout, Vkd_allocCb);

	if (dev->descriptorPool)
		vkDestroyDescriptorPool(dev->dev, dev->descriptorPool, Vkd_allocCb);

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		if (dev->iaDescriptorPool[i])
			vkDestroyDescriptorPool(dev->dev, dev->iaDescriptorPool[i], Vkd_allocCb);

	return false;
}

VkDescriptorSet
Vk_AllocateIADescriptorSet(struct NeRenderDevice *dev)
{
	VkDescriptorSet ds = VK_NULL_HANDLE;
	VkDescriptorSetAllocateInfo allocInfo =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = dev->iaDescriptorPool[Re_frameId],
		.descriptorSetCount = 1,
		.pSetLayouts = &dev->iaSetLayout
	};
	if (vkAllocateDescriptorSets(dev->dev, &allocInfo, &ds) != VK_SUCCESS)
		return VK_NULL_HANDLE;

#ifdef _DEBUG
	Vkd_SetObjectName(dev->dev, ds, VK_OBJECT_TYPE_DESCRIPTOR_SET, "Input Attachment Descriptor Set");
#endif

	return ds;
}

void
Vk_SetSampler(struct NeRenderDevice *dev, uint16_t location, VkSampler sampler)
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
Vk_SetTexture(struct NeRenderDevice *dev, uint16_t location, VkImageView imageView)
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
Vk_SetInputAttachment(struct NeRenderDevice *dev, VkDescriptorSet set, uint16_t location, VkImageView imageView)
{
	VkDescriptorImageInfo dii =
	{
		.imageView = imageView,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};
	VkWriteDescriptorSet wds =
	{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = set,
		.dstBinding = 0,
		.dstArrayElement = location,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
		.pImageInfo = &dii
	};
	vkUpdateDescriptorSets(dev->dev, 1, &wds, 0, NULL);
}

void
Vk_TermDescriptorSet(struct NeRenderDevice *dev)
{
	vkDestroyDescriptorPool(dev->dev, dev->descriptorPool, Vkd_allocCb);
	vkDestroyDescriptorSetLayout(dev->dev, dev->setLayout, Vkd_allocCb);

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i)
		vkDestroyDescriptorPool(dev->dev, dev->iaDescriptorPool[i], Vkd_allocCb);
	vkDestroyDescriptorSetLayout(dev->dev, dev->iaSetLayout, Vkd_allocCb);
}
