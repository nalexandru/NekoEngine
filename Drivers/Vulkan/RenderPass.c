#include <stdlib.h>

#include <System/Memory.h>

#include "VulkanDriver.h"

struct RenderPass *
Vk_CreateRenderPass(struct RenderDevice *dev, const struct RenderPassDesc *desc)
{
	struct RenderPass *rp = calloc(sizeof(*rp), 1);
	if (!rp)
		return NULL;

	VkAttachmentDescription *atDesc = Sys_Alloc(sizeof(*atDesc), desc->attachmentCount, MH_Transient);
	VkAttachmentReference *atRef = Sys_Alloc(sizeof(*atRef), desc->attachmentCount, MH_Transient);

	rp->clearValues = calloc(sizeof(*rp->clearValues), desc->attachmentCount);

	for (uint32_t i = 0; i < desc->attachmentCount; ++i) {
		atDesc[i].flags = desc->attachments[i].mayAlias ? VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT : 0;
		atDesc[i].format = NeToVkTextureFormat(desc->attachments[i].format);
		atDesc[i].samples = desc->attachments[i].samples;
		atDesc[i].loadOp = desc->attachments[i].loadOp;
		atDesc[i].storeOp = desc->attachments[i].storeOp;
		atDesc[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		atDesc[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		atDesc[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		atDesc[i].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		atRef[i].attachment = i;
		atRef[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		memcpy(rp->clearValues[i].color.float32, desc->attachments[i].clearColor, sizeof(rp->clearValues[i].color.float32));
	}

	VkSubpassDescription spDesc[] =
	{
		{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = desc->attachmentCount,
			.pColorAttachments = atRef,
		}
	};

	VkSubpassDependency spDep[] =
	{
		{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
			.dstSubpass = 0,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
		},
		{
			.srcSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.dstSubpass = VK_SUBPASS_EXTERNAL,
			.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
			.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
		}
	};

	VkRenderPassCreateInfo rpInfo =
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = desc->attachmentCount,
		.pAttachments = atDesc,
		.subpassCount = sizeof(spDesc) / sizeof(spDesc[0]),
		.pSubpasses = spDesc,
		.dependencyCount = sizeof(spDep) / sizeof(spDep[0]),
		.pDependencies = spDep,
	};

	if (vkCreateRenderPass(dev->dev, &rpInfo, Vkd_allocCb, &rp->rp) != VK_SUCCESS) {
		free(rp);
		return NULL;
	}

	return rp;
}

void
Vk_DestroyRenderPass(struct RenderDevice *dev, struct RenderPass *rp)
{
	vkDestroyRenderPass(dev->dev, rp->rp, Vkd_allocCb);

	free(rp->clearValues);
	free(rp);
}
