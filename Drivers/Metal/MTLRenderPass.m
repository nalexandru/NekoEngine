#define Handle __EngineHandle

#include <Render/Device.h>
#include <Render/RenderPass.h>

#undef Handle

#include "MTLDriver.h"

struct RenderPass *
MTL_CreateRenderPass(id<MTLDevice> dev, const struct RenderPassDesc *desc)
{
	struct RenderPass *rp = calloc(1, sizeof(*rp));
	if (!rp)
		return NULL;
	
	rp->desc = [MTLRenderPassDescriptor renderPassDescriptor];
	if (!rp->desc) {
		free(rp);
		return NULL;
	}
	
	[rp->desc retain];
	
	rp->attachmentCount = desc->attachmentCount;
	rp->attachmentFormats = calloc(sizeof(*rp->attachmentFormats), desc->attachmentCount);
	
	for (uint32_t i = 0; i < desc->attachmentCount; ++i) {
		const struct AttachmentDesc *at = &desc->attachments[i];
		
		switch (at->loadOp) {
		case ATL_LOAD: rp->desc.colorAttachments[i].loadAction = MTLLoadActionLoad; break;
		case ATL_CLEAR: rp->desc.colorAttachments[i].loadAction = MTLLoadActionClear; break;
		case ATL_DONT_CARE: rp->desc.colorAttachments[i].loadAction = MTLLoadActionDontCare; break;
		}
		
		switch (at->storeOp) {
		case ATS_STORE: rp->desc.colorAttachments[i].storeAction = MTLStoreActionStore; break;
		case ATS_DONT_CARE: rp->desc.colorAttachments[i].loadAction = MTLStoreActionDontCare; break;
		}
		
		rp->desc.colorAttachments[i].clearColor = MTLClearColorMake(at->clearColor[0], at->clearColor[1], at->clearColor[2], at->clearColor[3]);
		rp->attachmentFormats[i] = NeToMTLTextureFormat(at->format);
	}
	
	return rp;
}

void
MTL_DestroyRenderPass(id<MTLDevice> dev, struct RenderPass *rp)
{
	[rp->desc autorelease];
	free(rp);
}
