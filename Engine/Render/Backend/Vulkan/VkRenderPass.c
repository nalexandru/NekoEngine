#include <assert.h>
#include <stdlib.h>

#include <System/Memory.h>

#include "VulkanBackend.h"

static inline void _SetAttachment(VkAttachmentDescription *dst, const struct NeAttachmentDesc *src);

struct NeRenderPassDesc *
Re_CreateRenderPassDesc(const struct NeAttachmentDesc *attachments, uint32_t count, const struct NeAttachmentDesc *depthAttachment,
	const struct NeAttachmentDesc *inputAttachments, uint32_t inputCount)
{
	struct NeRenderPassDesc *rp = Sys_Alloc(sizeof(*rp), 1, MH_RenderDriver);
	if (!rp)
		return NULL;

	uint32_t atCount = count;
	if (depthAttachment)
		++atCount;

	if (inputCount)
		atCount += inputCount;

	VkAttachmentDescription *atDesc = Sys_Alloc(sizeof(*atDesc), atCount, MH_Transient);
	VkAttachmentReference *atRef = Sys_Alloc(sizeof(*atRef), atCount, MH_Transient);
	VkAttachmentReference *iaRef = NULL;

	rp->clearValues = Sys_Alloc(sizeof(*rp->clearValues), atCount, MH_RenderDriver);
	assert(rp->clearValues);

	rp->inputAttachments = inputCount;

	for (uint32_t i = 0; i < count; ++i) {
		_SetAttachment(&atDesc[i], &attachments[i]);

		atRef[i].attachment = i;
		atRef[i].layout = NeToVkImageLayout(attachments[i].layout);

		if (atDesc[i].loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
			memcpy(rp->clearValues[rp->clearValueCount++].color.float32, attachments[i].clearColor, sizeof(rp->clearValues[i].color.float32));
	}

	if (depthAttachment) {
		_SetAttachment(&atDesc[count], depthAttachment);

		atRef[count].attachment = count;
		atRef[count].layout = NeToVkImageLayout(depthAttachment->layout);

		if (atDesc[count].loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
			rp->clearValues[rp->clearValueCount  ].depthStencil.depth = depthAttachment->clearDepth;
			rp->clearValues[rp->clearValueCount++].depthStencil.stencil = depthAttachment->clearStencil;
		}
	}

	if (inputCount) {
		iaRef = Sys_Alloc(sizeof(*iaRef), inputCount, MH_Transient);

		int offset = depthAttachment ? count + 1 : count;
		for (uint32_t i = 0; i < inputCount; ++i) {
			int id = offset + i;
			_SetAttachment(&atDesc[id], &inputAttachments[i]);

			iaRef[i].attachment = id;
			iaRef[i].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	}

	VkSubpassDescription spDesc[] =
	{
		{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,

			.colorAttachmentCount = count,
			.pColorAttachments = atRef,

			.pDepthStencilAttachment = depthAttachment ? &atRef[count] : NULL,

			.pInputAttachments = iaRef,
			.inputAttachmentCount = inputCount
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
		.attachmentCount = atCount,
		.pAttachments = atDesc,
		.subpassCount = sizeof(spDesc) / sizeof(spDesc[0]),
		.pSubpasses = spDesc,
		.dependencyCount = sizeof(spDep) / sizeof(spDep[0]),
		.pDependencies = spDep,
	};

	if (vkCreateRenderPass(Re_device->dev, &rpInfo, Vkd_allocCb, &rp->rp) != VK_SUCCESS) {
		Sys_Free(rp->clearValues);
		Sys_Free(rp);
		return NULL;
	}

	return rp;
}

void
Re_DestroyRenderPassDesc(struct NeRenderPassDesc *rp)
{
	vkDestroyRenderPass(Re_device->dev, rp->rp, Vkd_allocCb);

	Sys_Free(rp->clearValues);
	Sys_Free(rp);
}

static inline void
_SetAttachment(VkAttachmentDescription *dst, const struct NeAttachmentDesc *src)
{
	dst->flags = src->mayAlias ? VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT : 0;
	dst->format = NeToVkTextureFormat(src->format);
	dst->samples = src->samples;
	dst->loadOp = src->loadOp;
	dst->storeOp = src->storeOp;
	dst->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	dst->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	dst->initialLayout = NeToVkImageLayout(src->initialLayout);
	dst->finalLayout = NeToVkImageLayout(src->finalLayout);
}

/* NekoEngine
 *
 * VkRenderPass.c
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
