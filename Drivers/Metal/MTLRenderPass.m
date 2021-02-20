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
		
		rp->desc.colorAttachments[i].clearColor = MTLClearColorMake(1.f, 0.f, 0.f, 1.f);
	}
	
	return rp;
}

void
MTL_DestroyRenderPass(id<MTLDevice> dev, struct RenderPass *rp)
{
	[rp->desc release];
	free(rp);
}
