#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Resource.h>
#include <Render/Texture.h>
#include <Runtime/Runtime.h>

bool
Re_CreateTexture(const char *name, const struct TextureCreateInfo *ci, struct Texture *tex, Handle h)
{
	bool rc = true;

	tex->width = ci->width;
	tex->height = ci->height;
	tex->depth = ci->depth;
	tex->format = ci->format;
	tex->type = ci->type;
	tex->levels = 1;
	tex->data = ci->data;
	tex->dataSize = ci->dataSize;

	rc = Re_InitTexture(name, tex, h);

	if (!ci->keepData) {
		free(tex->data);
		tex->data = NULL;
		tex->dataSize = 0;
	}

	return rc;
}

bool
Re_LoadTexture(struct ResourceLoadInfo *li, const char *args, struct Texture *tex, Handle h)
{
	bool rc = true;

	if (strstr(li->path, ".dds")) {
		//if (!E_LoadTGAAsset(data, size, tex))
		//	goto error;
	} else if (strstr(li->path, ".tga")) {
		if (!E_LoadTGAAsset(&li->stm, tex))
			rc = false;
	} else {
		if (!E_LoadImageAsset(&li->stm, tex))
			rc = false;
	}

	if (rc && !Re_InitTexture(li->path, tex, h))
		rc = false;

	free(tex->data);
	tex->data = NULL;
	tex->dataSize = 0;

	return rc;
}

void
Re_UnloadTexture(struct Texture *tex, Handle h)
{
	Re_TermTexture(tex);

	free(tex->data);
}
