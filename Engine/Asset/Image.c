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

/* NekoEngine
 *
 * Image.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
