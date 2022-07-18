#include "MTLBackend.h"

struct NeRenderPassDesc *
Re_CreateRenderPassDesc(const struct NeAttachmentDesc *attachments, uint32_t count, const struct NeAttachmentDesc *depthAttachment,
	const struct NeAttachmentDesc *inputAttachments, uint32_t inputCount)
{
	struct NeRenderPassDesc *rp = Sys_Alloc(sizeof(*rp), 1, MH_RenderDriver);
	if (!rp)
		return NULL;
	
	rp->desc = [MTLRenderPassDescriptor renderPassDescriptor];
	if (!rp->desc) {
		Sys_Free(rp);
		return NULL;
	}
	
	[rp->desc retain];
	
	rp->colorAttachments = count;
	rp->inputAttachments = inputCount;
	rp->attachmentFormats = Sys_Alloc(sizeof(*rp->attachmentFormats), count + inputCount, MH_RenderDriver);
	
	for (uint32_t i = 0; i < count; ++i) {
		const struct NeAttachmentDesc *at = &attachments[i];
		
		switch (at->loadOp) {
		case ATL_LOAD: rp->desc.colorAttachments[i].loadAction = MTLLoadActionLoad; break;
		case ATL_CLEAR: rp->desc.colorAttachments[i].loadAction = MTLLoadActionClear; break;
		case ATL_DONT_CARE: rp->desc.colorAttachments[i].loadAction = MTLLoadActionDontCare; break;
		}
		
		switch (at->storeOp) {
		case ATS_STORE: rp->desc.colorAttachments[i].storeAction = MTLStoreActionStore; break;
		case ATS_DONT_CARE: rp->desc.colorAttachments[i].storeAction = MTLStoreActionDontCare; break;
		}
		
		rp->desc.colorAttachments[i].clearColor = MTLClearColorMake(at->clearColor[0], at->clearColor[1], at->clearColor[2], at->clearColor[3]);
		rp->attachmentFormats[i] = NeToMTLTextureFormat(at->format);
	}
	
	if (depthAttachment) {
		switch (depthAttachment->loadOp) {
		case ATL_LOAD: rp->desc.depthAttachment.loadAction = MTLLoadActionLoad; break;
		case ATL_CLEAR: rp->desc.depthAttachment.loadAction = MTLLoadActionClear; break;
		case ATL_DONT_CARE: rp->desc.depthAttachment.loadAction = MTLLoadActionDontCare; break;
		}
		
		switch (depthAttachment->storeOp) {
		case ATS_STORE: rp->desc.depthAttachment.storeAction = MTLStoreActionStore; break;
		case ATS_DONT_CARE: rp->desc.depthAttachment.storeAction = MTLStoreActionDontCare; break;
		}
		
		rp->desc.depthAttachment.clearDepth = depthAttachment->clearDepth;
		rp->depthFormat = NeToMTLTextureFormat(depthAttachment->format);
	}

	for (uint32_t i = 0; i < inputCount; ++i) {
		int idx = i + count;

		const struct NeAttachmentDesc *at = &inputAttachments[i];

		switch (at->loadOp) {
		case ATL_LOAD: rp->desc.colorAttachments[idx].loadAction = MTLLoadActionLoad; break;
		case ATL_CLEAR: rp->desc.colorAttachments[idx].loadAction = MTLLoadActionClear; break;
		case ATL_DONT_CARE: rp->desc.colorAttachments[idx].loadAction = MTLLoadActionDontCare; break;
		}

		switch (at->storeOp) {
		case ATS_STORE: rp->desc.colorAttachments[idx].storeAction = MTLStoreActionStore; break;
		case ATS_DONT_CARE: rp->desc.colorAttachments[idx].storeAction = MTLStoreActionDontCare; break;
		}

		rp->desc.colorAttachments[idx].clearColor = MTLClearColorMake(at->clearColor[0], at->clearColor[1], at->clearColor[2], at->clearColor[3]);
		rp->attachmentFormats[idx] = NeToMTLTextureFormat(at->format);
	}
	
	return rp;
}

void
Re_DestroyRenderPassDesc(struct NeRenderPassDesc *rp)
{
	[rp->desc release];
	Sys_Free(rp->attachmentFormats);
	Sys_Free(rp);
}
