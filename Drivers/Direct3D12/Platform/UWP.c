#include "../D3D12Driver.h"

struct Surface *
D3D12_CreateUWPSurface(struct RenderDevice *dev, void *window)
{
	struct Surface *s = Sys_Alloc(sizeof(*s), 1, MH_RenderDriver);
	if (!s)
		return NULL;

	s->coreWindow = window;

	return s;
}

void
D3D12_DestroyUWPSurface(struct RenderDevice *dev, struct Surface *surface)
{
	Sys_Free(surface);
}
