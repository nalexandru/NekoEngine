#include <math.h>
#include <stdio.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <Asset/NFont.h>

#include <Editor/GUI.h>
#include <Editor/Types.h>
#include <Editor/Editor.h>
#include <Editor/Asset/Import.h>

ED_ASSET_IMPORTER(Font);

#include <ft2build.h>
#include FT_FREETYPE_H

static void
WriteImg(void *context, void *data, int size)
{
	fwrite(data, size, 1, (FILE *)context);
}

static bool
Font_MatchAsset(const char *path)
{
	return strstr(path, ".ttf") || strstr(path, ".otf");
}

static bool
Font_ImportAsset(const char *path, const struct NeAssetImportOptions *options)
{
	FILE *fp = NULL;
	FT_Face sizeFace = NULL, renderFace = NULL;
	FT_Library ft = NULL;
	FT_GlyphSlot sizeGlyph, renderGlyph;
	uint32_t x = 0, y = 0;
	uint8_t *data;
	struct NFontHeader hdr;
	struct NeGlyph *glyphs;

	hdr.magic = FNT_MAGIC;
	hdr.texSize = 4096; // TODO: import options
	hdr.glyphCount = 128 - 0x20;
	const uint32_t h = (hdr.texSize / (uint32_t)ceilf(sqrtf((float)hdr.glyphCount)));

	if (FT_Init_FreeType(&ft) != FT_Err_Ok) {
		EdGUI_MessageBox("Error", "Failed to initialize FreeType", MB_Error);
		goto error;
	}

	if (FT_New_Face(ft, path, 0, &sizeFace) != FT_Err_Ok) {
		EdGUI_MessageBox("Error", "Failed to load size face", MB_Error);
		goto error;
	}

	if (FT_New_Face(ft, path, 0, &renderFace) != FT_Err_Ok) {
		EdGUI_MessageBox("Error", "Failed to load render face", MB_Error);
		goto error;
	}

	FT_Set_Pixel_Sizes(sizeFace, 0, 10);
	FT_Set_Pixel_Sizes(renderFace, 0, (h - 2));

	sizeGlyph = sizeFace->glyph;
	renderGlyph = renderFace->glyph;

	data = calloc(sizeof(*data), (size_t)hdr.texSize * hdr.texSize);
	glyphs = calloc(sizeof(*glyphs), hdr.glyphCount);

	for (int i = 0x20; i < 128; ++i) {
		uint32_t j, g = i - 0x20;

		FT_Load_Char(sizeFace, i, FT_LOAD_COMPUTE_METRICS);
		FT_Load_Char(renderFace, i, FT_LOAD_RENDER);

		if (x + renderGlyph->bitmap.width > hdr.texSize) {
			x = 0;
			y += h;
		}

		for (j = 0; j < renderGlyph->bitmap.rows; ++j)
			memmove(data + (x + hdr.texSize * (j + y)), renderGlyph->bitmap.buffer + renderGlyph->bitmap.width * j, renderGlyph->bitmap.width);

		glyphs[g].u = (float)x / (float)hdr.texSize;
		glyphs[g].v = (float)y / (float)hdr.texSize;
		glyphs[g].th = (float)renderGlyph->bitmap.rows / (float)hdr.texSize;
		glyphs[g].tw = (float)renderGlyph->bitmap.width / (float)hdr.texSize;
		glyphs[g].bearing.x = sizeGlyph->bitmap_left;
		glyphs[g].bearing.y = sizeGlyph->bitmap_top;
		glyphs[g].size.w = sizeGlyph->bitmap.width;
		glyphs[g].size.h = sizeGlyph->bitmap.rows;
		glyphs[g].adv = (uint32_t)sizeGlyph->advance.x >> 6;

		x += h;
	}

	FT_Done_Face(renderFace);
	FT_Done_Face(sizeFace);
	FT_Done_FreeType(ft);

	char dstPath[4096];
	snprintf(dstPath, sizeof(dstPath), "%s%cFonts", Ed_dataDir, ED_DIR_SEPARATOR);

	if (!Sys_DirectoryExists(dstPath))
		Sys_CreateDirectory(dstPath);

	char *name = strrchr(path, ED_DIR_SEPARATOR);
	snprintf(dstPath, sizeof(dstPath), "%s%cFonts%c%s", Ed_dataDir, ED_DIR_SEPARATOR, ED_DIR_SEPARATOR, ++name);
	char *eptr = strrchr(dstPath, '.');
	eptr[1] = 'f';
	eptr[2] = 'n';
	eptr[3] = 't';

	fp = fopen(dstPath, "wb");
	fwrite(&hdr, sizeof(hdr), 1, fp);
	fwrite(glyphs, sizeof(*glyphs), hdr.glyphCount, fp);

	stbi_write_tga_to_func(WriteImg, fp, hdr.texSize, hdr.texSize, 1, data);

	fclose(fp);

	return true;

error:
	if (sizeFace)
		FT_Done_Face(sizeFace);

	if (renderFace)
		FT_Done_Face(renderFace);

	if (ft)
		FT_Done_FreeType(ft);

	return false;
}

/* NekoEditor
 *
 * Font.c
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
