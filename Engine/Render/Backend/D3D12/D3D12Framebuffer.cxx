#include <stdlib.h>

#include <System/Memory.h>

#include "D3D12Backend.h"

struct NeFramebuffer *
Re_CreateFramebuffer(const struct NeFramebufferDesc *desc)
{
	enum NeMemoryHeap heap = MH_Frame;

	NeFramebuffer *fb = (NeFramebuffer *)Sys_Alloc(1, sizeof(*fb), heap);
	if (!fb)
		return nullptr;

	/*fb->attachments = Sys_Alloc(desc->attachmentCount, sizeof(*fb->attachments), heap);
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

	if (vkCreateFramebuffer(Re_device->dev, &fbInfo, Vkd_allocCb, &fb->fb) != VK_SUCCESS) {
		Sys_Free(fb->attachments);
		Sys_Free(fb);
		return NULL;
	}

	fb->attachmentCount = desc->attachmentCount;
	fb->width = desc->width;
	fb->height = desc->height;
	fb->layers = desc->layers;

#ifdef _DEBUG
	if (desc->name)
		Vkd_SetObjectName(Re_device->dev, fb->fb, VK_OBJECT_TYPE_FRAMEBUFFER, desc->name);
#endif*/

	return fb;
}

void
Re_SetAttachment(struct NeFramebuffer *fb, uint32_t pos, struct NeTexture *tex)
{
	//fb->attachments[pos] = tex->imageView;
}

void
Re_DestroyFramebuffer(struct NeFramebuffer *fb)
{
	/*vkDestroyFramebuffer(Re_device->dev, fb->fb, Vkd_allocCb);

	Sys_Free(fb->attachments);*/
	Sys_Free(fb);
}

/* NekoEngine
 *
 * D3D12Framebuffer.cxx
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
