#ifndef _RE_RENDER_PASS_H_
#define _RE_RENDER_PASS_H_

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

static inline struct RenderPassDesc *Re_CreateRenderPassDesc(const struct AttachmentDesc *attachments, uint32_t count, const struct AttachmentDesc *depthAttachment)
{ return Re_deviceProcs.CreateRenderPassDesc(Re_device, attachments, count, depthAttachment); }
static inline void Re_DestroyRenderPassDesc(struct RenderPassDesc *pass) { Re_deviceProcs.DestroyRenderPassDesc(Re_device, pass); }

#endif /* _RE_RENDER_PASS_H_ */
