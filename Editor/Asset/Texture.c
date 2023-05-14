#include <png.h>
#include <stb_image.h>

#include <Editor/Asset/Asset.h>

bool
EdAsset_ConvertToPNG(const uint8_t *data, size_t size, const char *path)
{
	stbi_set_flip_vertically_on_load(0);

	int w, h, c;
	stbi_uc *imageData = stbi_load_from_memory(data, (int)size, &w, &h, &c, 4);
	if (!imageData)
		return false;

	return EdAsset_SavePNG(w, h, imageData, path);
}

bool
EdAsset_SavePNG(uint32_t w, uint32_t h, uint8_t *data, const char *path)
{
	FILE *fp = fopen(path, "wb");
	if (!fp)
		return false;

	png_bytepp rows = (png_bytepp)calloc(h, sizeof(*rows));
	for (uint32_t i = 0; i < h; ++i)
		rows[i] = data + (w * 4 * i);

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info = png_create_info_struct(png);
	png_init_io(png, fp);

	png_set_IHDR(png, info, w, h, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_ADAM7, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_set_rows(png, info, rows);

	png_set_filter(png, 0, PNG_FILTER_NONE);
	png_set_compression_level(png, 5);

	png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);
	png_destroy_write_struct(&png, &info);

	fclose(fp);
	free(rows);

	return true;
}

/* NekoEditor
 *
 * Texture.c
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
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
