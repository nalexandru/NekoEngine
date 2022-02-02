#include "../D3D12Driver.h"

struct NeSurface *
D3D12_CreateWin32Surface(struct NeRenderDevice *dev, void *window)
{
	struct NeSurface *s = Sys_Alloc(sizeof(*s), 1, MH_RenderDriver);
	if (!s)
		return NULL;

	s->hWnd = window;

	return s;
}

void
D3D12_DestroyWin32Surface(struct NeRenderDevice *dev, struct NeSurface *surface)
{
	Sys_Free(surface);
}
