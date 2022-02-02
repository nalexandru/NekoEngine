#include "OpenGLDriver.h"

struct NeSampler *
GL_CreateSampler(struct NeRenderDevice *dev, const struct NeSamplerDesc *desc)
{
	struct NeSampler *s = Sys_Alloc(1, sizeof(*s), MH_RenderDriver);
	if (!s)
		return NULL;

	//GL_SetSampler(dev, 0, s);

	return s;
}

void
GL_DestroySampler(struct NeRenderDevice *dev, struct NeSampler *s)
{
	Sys_Free(s);
}
