#include "NullGraphicsDriver.h"

struct NeSampler *
NG_CreateSampler(struct NeRenderDevice *dev, const struct NeSamplerDesc *desc)
{
	struct NeSampler *s = Sys_Alloc(1, sizeof(*s), MH_RenderDriver);
	if (!s)
		return NULL;

	return s;
}

void
NG_DestroySampler(struct NeRenderDevice *dev, struct NeSampler *s)
{
	Sys_Free(s);
}
