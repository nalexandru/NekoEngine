#ifndef _RE_RENDER_PASS_H_
#define _RE_RENDER_PASS_H_

#include <Render/Types.h>
#include <Render/Device.h>

struct AttachmentDesc
{
	bool mayAlias;
	enum TextureFormat format;
	enum AttachmentLoadOp loadOp;
	enum AttachmentStoreOp storeOp;
	enum AttachmentSampleCount samples;
	union {
		float clearColor[4];
		struct {
			float clearDepth;
			uint8_t clearStencil;
		};
	};
};

struct RenderPassDesc
{
	uint32_t attachmentCount;
	struct AttachmentDesc *attachments;
};

static inline struct RenderPass *Re_CreateRenderPass(const struct RenderPassDesc *desc) { return Re_deviceProcs.CreateRenderPass(Re_device, desc); }
static inline void Re_DestroyRenderPass(struct RenderPass *pass) { Re_deviceProcs.DestroyRenderPass(Re_device, pass); }

#endif /* _RE_RENDER_PASS_H_ */
