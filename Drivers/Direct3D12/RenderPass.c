#include <assert.h>
#include <stdlib.h>

#include <Math/Math.h>
#include <System/Memory.h>

#include "D3D12Driver.h"

//static inline void _SetAttachment(VkAttachmentDescription *dst, const struct NeAttachmentDesc *src);

struct NeRenderPassDesc *
D3D12_CreateRenderPassDesc(struct NeRenderDevice *dev, const struct NeAttachmentDesc *attachments, uint32_t count, const struct NeAttachmentDesc *depthAttachment,
							const struct NeAttachmentDesc *inputAttachments, uint32_t inputCount)
{
	struct NeRenderPassDesc *rp = Sys_Alloc(sizeof(*rp), 1, MH_RenderDriver);
	if (!rp)
		return NULL;

	rp->attachmentCount = count + inputCount;

	for (uint32_t i = 0; i < count; ++i) {
		const struct NeAttachmentDesc *at = &attachments[i];

		rp->rtvFormats[i] = NeToDXGITextureFormat(at->format);
		rp->loadOp[i] = at->loadOp;
		// at->storeOp
		v4(&rp->clearValues[i], at->clearColor[0], at->clearColor[1], at->clearColor[2], at->clearColor[3]);
		rp->rtvInitialState[i] = NeTextureLayoutToD3D12ResourceState(at->initialLayout);
		rp->rtvState[i] = NeTextureLayoutToD3D12ResourceState(at->layout);
		rp->rtvFinalState[i] = NeTextureLayoutToD3D12ResourceState(at->finalLayout);
	}

	for (uint32_t i = 0; i < inputCount; ++i) {
		const int idx = i + count;
		const struct NeAttachmentDesc *at = &inputAttachments[i];

		rp->rtvFormats[idx] = NeToDXGITextureFormat(at->format);
		rp->loadOp[idx] = at->loadOp;
		// at->storeOp
		v4(&rp->clearValues[idx], at->clearColor[0], at->clearColor[1], at->clearColor[2], at->clearColor[3]);
		rp->rtvInitialState[idx] = NeTextureLayoutToD3D12ResourceState(at->initialLayout);
		rp->rtvState[idx] = NeTextureLayoutToD3D12ResourceState(at->layout);
		rp->rtvFinalState[idx] = NeTextureLayoutToD3D12ResourceState(at->finalLayout);
	}

	if (depthAttachment) {
		rp->depthFormat = NeToDXGITextureFormat(depthAttachment->format);
		rp->depthLoadOp = depthAttachment->loadOp;
		rp->depthClearValue = depthAttachment->clearDepth;
		rp->depthInitialState = NeTextureLayoutToD3D12ResourceState(depthAttachment->initialLayout);
		rp->depthState = NeTextureLayoutToD3D12ResourceState(depthAttachment->layout);
		rp->depthFinalState = NeTextureLayoutToD3D12ResourceState(depthAttachment->finalLayout);
	} else {
		rp->depthFormat = DXGI_FORMAT_UNKNOWN;
	}

	return rp;
}

void
D3D12_DestroyRenderPassDesc(struct NeRenderDevice *dev, struct NeRenderPassDesc *rp)
{
//	vkDestroyRenderPass(dev->dev, rp->rp, Vkd_allocCb);

//	Sys_Free(rp->clearValues);
	Sys_Free(rp);
}

/*static inline void
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
}*/
