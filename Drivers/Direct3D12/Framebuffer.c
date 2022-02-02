#include <stdlib.h>

#include <System/Memory.h>

#include "D3D12Driver.h"

struct NeFramebuffer *
D3D12_CreateFramebuffer(struct NeRenderDevice *dev, const struct NeFramebufferDesc *desc)
{
	enum NeMemoryHeap heap = MH_Frame;

	struct NeFramebuffer *fb = Sys_Alloc(1, sizeof(*fb), heap);
	if (!fb)
		return NULL;

/*	fb->attachments = Sys_Alloc(desc->attachmentCount, sizeof(*fb->attachments), heap);
	if (!fb->attachments) {
		Sys_Free(fb);
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
		.renderPass = desc->renderPassDesc->rp,
		.attachmentCount = desc->attachmentCount,
		.width = desc->width,
		.height = desc->height,
		.layers = desc->layers
	};

	if (vkCreateFramebuffer(dev->dev, &fbInfo, Vkd_allocCb, &fb->fb) != VK_SUCCESS) {
		Sys_Free(fb->attachments);
		Sys_Free(fb);
		return NULL;
	}

	fb->attachmentCount = desc->attachmentCount;
	fb->width = desc->width;
	fb->height = desc->height;
	fb->layers = desc->layers;*/

	return fb;
}

void
D3D12_SetAttachment(struct NeFramebuffer *fb, uint32_t pos, struct NeTexture *tex)
{
//	fb->attachments[pos] = tex->imageView;
}

void
D3D12_DestroyFramebuffer(struct NeRenderDevice *dev, struct NeFramebuffer *fb)
{
//	vkDestroyFramebuffer(dev->dev, fb->fb, Vkd_allocCb);

//	Sys_Free(fb->attachments);
	Sys_Free(fb);
}
