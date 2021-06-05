#include "../D3D12Driver.h"

struct Surface *
D3D12_CreateWin32Surface(struct RenderDevice *dev, void *window)
{
	struct Surface *s = Sys_Alloc(sizeof(*s), 1, MH_RenderDriver);
	if (!s)
		return NULL;

	s->hWnd = window;

	return s;
}

void
D3D12_DestroyWin32Surface(struct RenderDevice *dev, struct Surface *surface)
{
	Sys_Free(surface);
}
