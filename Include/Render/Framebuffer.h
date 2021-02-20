#ifndef _RE_FRAMEBUFFER_H_
#define _RE_FRAMEBUFFER_H_

#include <Engine/Types.h>
#include <Render/Device.h>
#include <Render/Texture.h>

struct FramebufferAttachmentDesc
{
	enum TextureFormat format;
	enum TextureUsage usage;
};

struct FramebufferDesc
{
	uint32_t width, height, layers;
	struct FramebufferAttachmentDesc *attachments;
	uint32_t attachmentCount;
	struct RenderPass *renderPass;
};

static inline struct Framebuffer *Re_CreateFramebuffer(const struct FramebufferDesc *desc) { return Re_deviceProcs.CreateFramebuffer(Re_device, desc); }
static inline void Re_SetAttachment(struct Framebuffer *fb, uint32_t pos, struct Texture *tex) { Re_deviceProcs.SetAttachment(fb, pos, tex); }
static inline void Re_DestroyFramebuffer(struct Framebuffer *fb) { Re_deviceProcs.DestroyFramebuffer(Re_device, fb); }

#endif /* _RE_FRAMEBUFFER_H_ */
