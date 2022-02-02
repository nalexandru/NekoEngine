#include <stdlib.h>

#include "NullGraphicsDriver.h"

struct NeTexture *
NG_CreateTexture(struct NeRenderDevice *dev, const struct NeTextureDesc *desc, uint16_t location)
{
	struct NeTexture *tex = Sys_Alloc(1, sizeof(*tex), MH_RenderDriver);
	if (!tex)
		return NULL;
	return tex;
}

enum NeTextureLayout
NG_TextureLayout(const struct NeTexture *tex)
{
	return TL_UNKNOWN;
}

void
NG_DestroyTexture(struct NeRenderDevice *dev, struct NeTexture *tex)
{
	Sys_Free(tex);
}
