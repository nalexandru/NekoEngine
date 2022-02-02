#include <assert.h>
#include <stdlib.h>

#include <System/Memory.h>

#include "NullGraphicsDriver.h"

struct NeRenderPassDesc *
NG_CreateRenderPassDesc(struct NeRenderDevice *dev, const struct NeAttachmentDesc *attachments, uint32_t count, const struct NeAttachmentDesc *depthAttachment,
	const struct NeAttachmentDesc *inputAttachments, uint32_t inputCount)
{
	struct NeRenderPassDesc *rp = Sys_Alloc(sizeof(*rp), 1, MH_RenderDriver);
	if (!rp)
		return NULL;

	return rp;
}

void
NG_DestroyRenderPassDesc(struct NeRenderDevice *dev, struct NeRenderPassDesc *rp)
{
	Sys_Free(rp);
}
