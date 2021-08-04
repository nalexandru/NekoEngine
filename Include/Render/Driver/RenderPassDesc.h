#ifndef _NE_RENDER_DRIVER_RENDER_PASS_DESC_H_
#define _NE_RENDER_DRIVER_RENDER_PASS_DESC_H_

#include <Render/Types.h>
#include <Render/Driver/Device.h>

struct AttachmentDesc
{
	bool mayAlias;
	enum TextureFormat format;
	enum AttachmentLoadOp loadOp;
	enum AttachmentStoreOp storeOp;
	enum AttachmentSampleCount samples;
	enum TextureLayout layout, initialLayout, finalLayout;
	union {
		float clearColor[4];
		struct {
			float clearDepth;
			uint8_t clearStencil;
		};
	};
};

static inline struct RenderPassDesc *Re_CreateRenderPassDesc(const struct AttachmentDesc *attachments, uint32_t count, const struct AttachmentDesc *depthAttachment, const struct AttachmentDesc *inputAttachments, uint32_t inputCount)
{ return Re_deviceProcs.CreateRenderPassDesc(Re_device, attachments, count, depthAttachment, inputAttachments, inputCount); }
static inline void Re_DestroyRenderPassDesc(struct RenderPassDesc *pass) { Re_deviceProcs.DestroyRenderPassDesc(Re_device, pass); }

#endif /* _NE_RENDER_DRIVER_RENDER_PASS_DESC_H_ */
