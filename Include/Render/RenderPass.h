#ifndef _RE_RENDER_PASS_H_
#define _RE_RENDER_PASS_H_

#include <Engine/Types.h>
#include <Render/Device.h>
#include <Render/Framebuffer.h>

struct RenderPass;

enum AttachmentLoadOp
{
	ATL_LOAD = 0,
	ATL_CLEAR = 1,
	ATL_DONT_CARE = 2
};

enum AttachmentStoreOp
{
	ATS_STORE = 0,
	ATS_DONT_CARE = 1
};

enum AttachmentSampleCount
{
	ASC_1_SAMPLE		=  1,
	ASC_2_SAMPLES		=  2,
	ASC_4_SAMPLES		=  4,
	ASC_8_SAMPLES		=  8,
	ASC_16_SAMPLES		= 16
};

struct AttachmentDesc
{
	bool mayAlias;
	enum TextureFormat format;
	enum AttachmentLoadOp loadOp;
	enum AttachmentStoreOp storeOp;
	enum AttachmentSampleCount samples;
};

struct RenderPassDesc
{
	uint32_t attachmentCount;
	struct AttachmentDesc *attachments;
};

static inline struct RenderPass *Re_CreateRenderPass(const struct RenderPassDesc *desc) { return Re_deviceProcs.CreateRenderPass(Re_device, desc); }
static inline void Re_DestroyRenderPass(struct RenderPass *pass) { Re_deviceProcs.DestroyRenderPass(Re_device, pass); }

#endif /* _RE_RENDER_PASS_H_ */
