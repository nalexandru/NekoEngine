#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <System/Endian.h>
#include <Render/Render.h>

#define STBI_NO_STDIO
#define STBI_MALLOC(sz)			Sys_Alloc(1, sz, MH_Asset)
#define STBI_REALLOC(p, new)	Sys_ReAlloc(p, 1, new, MH_Asset)
#define STBI_FREE(p)			Sys_Free(p)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static int _stbiRead(void *user, char *data, int size);
static void _stbiSkip(void *user, int n);
static int _stbiEof(void *user);

static stbi_io_callbacks _cb = { _stbiRead, _stbiSkip, _stbiEof };

bool
E_LoadImageAssetComp(struct NeStream *stm, struct NeTextureCreateInfo *tci, bool flip, int rcomp)
{
	stbi_set_flip_vertically_on_load(flip);

	int x, y, comp;
	stbi_uc *image = stbi_load_from_callbacks(&_cb, stm, &x, &y, &comp, rcomp);
	if (!image)
		return false;

	tci->desc.type = TT_2D;

	switch (comp) {
	case 1: tci->desc.format = TF_R8_UNORM; break;
	case 2: tci->desc.format = TF_R8G8_UNORM; break;
	case 4: tci->desc.format = TF_R8G8B8A8_UNORM; break;
	}

	tci->desc.width = (uint16_t)x;
	tci->desc.height = (uint16_t)y;
	tci->desc.mipLevels = tci->desc.arrayLayers = 1;
	tci->data = image;
	tci->dataSize = sizeof(*image) * rcomp * x * y;

	return true;
}

bool
E_LoadHDRAsset(struct NeStream *stm, struct NeTextureCreateInfo *tci, bool flip)
{
	stbi_set_flip_vertically_on_load(flip);

	int x, y, comp;
	float *image = stbi_loadf_from_callbacks(&_cb, stm, &x, &y, &comp, 4);
	if (!image)
		return false;

	tci->desc.type = TT_2D;
	tci->desc.format = TF_R32G32B32A32_SFLOAT;
	tci->desc.width = (uint16_t)x;
	tci->desc.height = (uint16_t)y;
	tci->desc.mipLevels = tci->desc.arrayLayers = 1;
	tci->data = image;
	tci->dataSize = sizeof(*image) * 4 * x * y;

	return true;
}

int
_stbiRead(void *user, char *data, int size)
{
	return (int)E_ReadStream((struct NeStream *)user, data, size);
}

void
_stbiSkip(void *user, int n)
{
	E_StreamSeek((struct NeStream *)user, n, IO_SEEK_CUR);
}

int
_stbiEof(void *user)
{
	return E_EndOfStream((struct NeStream *)user);
}
