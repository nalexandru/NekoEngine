#include "../D3D12Driver.h"

struct NeSurface *
D3D12_CreateUWPSurface(struct NeRenderDevice *dev, void *window)
{
	struct NeSurface *s = Sys_Alloc(sizeof(*s), 1, MH_RenderDriver);
	if (!s)
		return NULL;

	s->coreWindow = window;

	return s;
}

void
D3D12_DestroyUWPSurface(struct NeRenderDevice *dev, struct NeSurface *surface)
{
	Sys_Free(surface);
}
