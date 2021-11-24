#include <stdio.h>
#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define FNT_MAGIC	0xB00B5000

struct Font
{
	uint32_t magic;
	uint32_t texSize;
	uint32_t glyphCount;
};

struct Glyph
{
	float u, v, tw, th;
	struct {
		int32_t x;
		int32_t y;
	} bearing;
	struct {
		int32_t w;
		int32_t h;
	} size;
	uint32_t adv;
};

static inline void
_downsize(const uint8_t *src, uint8_t *dst, uint32_t srcSize, uint32_t dstSize)
{
	uint32_t i = 0, j = 0, si = 0, sj = 0, scale = srcSize / dstSize;
	for (i = 0, si = 0; i < dstSize; ++i, si += scale)
		for (j = 0, sj = 0; j < dstSize; ++j, sj += scale)
			dst[i * dstSize + j] = src[si * srcSize + sj];
}

static inline void
_usage(const char *argv0)
{
	wprintf(L"usage: %hs <font file> <texture size> <output file>\n", argv0);
	exit(0);
}

static void
_write_img(void *context, void *data, int size)
{
	fwrite(data, size, 1, (FILE *)context);
}

int
main(int argc, char *argv[])
{
	FILE *fp = NULL;
	FT_Face sizeFace, renderFace;
	FT_Library ft;
	FT_GlyphSlot sizeGlyph, renderGlyph;
	uint32_t x = 0, y = 0, h = 0, minSize = 128, size = 0;
	uint8_t *data, *img;
	struct Font font;
	struct Glyph *glyphs;

	if (argc != 4)
		_usage(argv[0]);

	font.magic = FNT_MAGIC;
	font.texSize = atoi(argv[2]);
	font.glyphCount = 128 - 0x20;
	h = (font.texSize / (uint32_t)ceilf(sqrtf((float)font.glyphCount)));

	if (FT_Init_FreeType(&ft) != FT_Err_Ok) {

	}

	if (FT_New_Face(ft, argv[1], 0, &sizeFace) != FT_Err_Ok) {
		//
	}

	if (FT_New_Face(ft, argv[1], 0, &renderFace) != FT_Err_Ok) {
		//
	}

	FT_Set_Pixel_Sizes(sizeFace, 0, 10);
	FT_Set_Pixel_Sizes(renderFace, 0, (h - 2));

	sizeGlyph = sizeFace->glyph;
	renderGlyph = renderFace->glyph;

	data = calloc(sizeof(*data), (size_t)font.texSize * font.texSize);
	glyphs = calloc(sizeof(*glyphs), font.glyphCount);

	for (int i = 0x20; i < 128; ++i) {
		uint32_t j, g = i - 0x20;

		FT_Load_Char(sizeFace, i, FT_LOAD_COMPUTE_METRICS);
		FT_Load_Char(renderFace, i, FT_LOAD_RENDER);

		if (x + renderGlyph->bitmap.width > font.texSize) {
			x = 0;
			y += h;
		}

		for (j = 0; j < renderGlyph->bitmap.rows; ++j)
			memmove(data + (x + font.texSize * (j + y)), renderGlyph->bitmap.buffer + renderGlyph->bitmap.width * j, renderGlyph->bitmap.width);

		glyphs[g].u = (float)x / (float)font.texSize;
		glyphs[g].v = (float)y / (float)font.texSize;
		glyphs[g].th = (float)renderGlyph->bitmap.rows / (float)font.texSize;
		glyphs[g].tw = (float)renderGlyph->bitmap.width / (float)font.texSize;
		glyphs[g].bearing.x = sizeGlyph->bitmap_left;
		glyphs[g].bearing.y = sizeGlyph->bitmap_top;
		glyphs[g].size.w = sizeGlyph->bitmap.width;
		glyphs[g].size.h = sizeGlyph->bitmap.rows;
		glyphs[g].adv = (uint32_t)sizeGlyph->advance.x >> 6;

		x += h; //renderGlyph->bitmap.width + 2;
	}
	
	FT_Done_Face(renderFace);
	FT_Done_Face(sizeFace);
	FT_Done_FreeType(ft);

	fp = fopen(argv[3], "wb");
	fwrite(&font, sizeof(font), 1, fp);
	fwrite(glyphs, sizeof(*glyphs), font.glyphCount, fp);

	stbi_write_tga_to_func(_write_img, fp, font.texSize, font.texSize, 1, data);

//	fwrite(data, sizeof(uint8_t), (size_t)font.texSize * font.texSize, fp);

/*	img = calloc(sizeof(*img), font.texSize * font.texSize);
	size = font.texSize / 2;
	while (size >= minSize) {
		_downsize(data, img, font.texSize, size);
		fwrite(img, sizeof(uint8_t), (size_t)size * size, fp);
		size /= 2;
	}*/

	fclose(fp);

	return  0;
}
