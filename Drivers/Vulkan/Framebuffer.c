#include <stdlib.h>

#include <System/Memory.h>

#include "VulkanDriver.h"

struct Framebuffer *
Vk_CreateFramebuffer(struct RenderDevice *dev, const struct FramebufferDesc *desc)
{
	struct Framebuffer *fb = calloc(1, sizeof(*fb));
	if (!fb)
		return NULL;

	fb->attachments = calloc(desc->attachmentCount, sizeof(*fb->attachments));
	if (!fb->attachments) {
		free(fb);
		return NULL;
	}

	VkFramebufferAttachmentImageInfo *imgInfo = Sys_Alloc(sizeof(*imgInfo), desc->attachmentCount, MH_Transient);
	VkFormat *formats = Sys_Alloc(sizeof(*formats), desc->attachmentCount, MH_Transient);

	for (uint32_t i = 0; i < desc->attachmentCount; ++i) {
		imgInfo[i].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO,
		imgInfo[i].usage = desc->attachments[i].usage;
		imgInfo[i].width = desc->width;
		imgInfo[i].height = desc->height;
		imgInfo[i].layerCount = desc->layers;
		imgInfo[i].viewFormatCount = 1;
		imgInfo[i].pViewFormats = &formats[i];

		formats[i] = NeToVkTextureFormat(desc->attachments[i].format);
	}

	VkFramebufferAttachmentsCreateInfo atInfo =
	{
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO,
		.attachmentImageInfoCount = desc->attachmentCount,
		.pAttachmentImageInfos = imgInfo
	};

	VkFramebufferCreateInfo fbInfo = 
	{
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.pNext = &atInfo,
		.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT,
		.renderPass = desc->renderPass->rp,
		.attachmentCount = desc->attachmentCount,
		.width = desc->width,
		.height = desc->height,
		.layers = desc->layers
	};

	if (vkCreateFramebuffer(dev->dev, &fbInfo, Vkd_allocCb, &fb->fb) != VK_SUCCESS) {
		free(fb->attachments);
		free(fb);
		return NULL;
	}

	fb->attachmentCount = desc->attachmentCount;
	fb->width = desc->width;
	fb->height = desc->height;
	fb->layers = desc->layers;

	return fb;
}

void
Vk_SetAttachment(struct Framebuffer *fb, uint32_t pos, struct Texture *tex)
{
	fb->attachments[pos] = tex->imageView;
}

void
Vk_DestroyFramebuffer(struct RenderDevice *dev, struct Framebuffer *fb)
{
	vkDestroyFramebuffer(dev->dev, fb->fb, Vkd_allocCb);

	free(fb->attachments);
	free(fb);
}

