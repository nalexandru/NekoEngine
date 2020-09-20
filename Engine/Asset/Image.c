#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <System/Endian.h>
#include <Render/Texture.h>

#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static int _stbiRead(void *user, char *data, int size);
static void _stbiSkip(void *user, int n);
static int _stbiEof(void *user);

static stbi_io_callbacks _cb = { _stbiRead, _stbiSkip, _stbiEof };

bool
E_LoadImageAsset(struct Stream *stm, struct Texture *tex)
{
	int x, y, comp;
	stbi_uc *image = stbi_load_from_callbacks(&_cb, stm, &x, &y, &comp, 4);

	if (!image)
		return false;

	tex->width = (uint16_t)x;
	tex->height = (uint16_t)y;
	tex->levels = 1;
	tex->data = image;
	tex->dataSize = sizeof(stbi_uc) * comp * x * y;
	tex->format = TF_R8G8B8A8_UNORM;

	return true;
}

int
_stbiRead(void *user, char *data, int size)
{
	return (int)E_ReadStream((struct Stream *)user, data, size);
}

void
_stbiSkip(void *user, int n)
{
	E_StreamSeek((struct Stream *)user, n, IO_SEEK_CUR);
}

int
_stbiEof(void *user)
{
	return E_EndOfStream((struct Stream *)user);
}