#ifndef _NE_RENDER_DRIVER_RENDER_PASS_DESC_H_
#define _NE_RENDER_DRIVER_RENDER_PASS_DESC_H_

#include <Render/Types.h>
#include <Render/Driver/Device.h>

struct NeAttachmentDesc
{
	bool mayAlias;
	enum NeTextureFormat format;
	enum NeAttachmentLoadOp loadOp;
	enum NeAttachmentStoreOp storeOp;
	enum NeAttachmentSampleCount samples;
	enum NeTextureLayout layout, initialLayout, finalLayout;
	union {
		float clearColor[4];
		struct {
			float clearDepth;
			uint8_t clearStencil;
		};
	};
};

static inline struct NeRenderPassDesc *Re_CreateRenderPassDesc(const struct NeAttachmentDesc *attachments, uint32_t count, const struct NeAttachmentDesc *depthAttachment, const struct NeAttachmentDesc *inputAttachments, uint32_t inputCount)
{ return Re_deviceProcs.CreateRenderPassDesc(Re_device, attachments, count, depthAttachment, inputAttachments, inputCount); }
static inline void Re_DestroyRenderPassDesc(struct NeRenderPassDesc *pass) { Re_deviceProcs.DestroyRenderPassDesc(Re_device, pass); }

#endif /* _NE_RENDER_DRIVER_RENDER_PASS_DESC_H_ */
