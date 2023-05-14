#ifndef NE_ASSET_NFONT_H
#define NE_ASSET_NFONT_H

#include <Engine/Types.h>

#define FNT_MAGIC	0xB00B5000

struct NFontHeader
{
	uint32_t magic;
	uint32_t texSize;
	uint32_t glyphCount;
};

struct NeGlyph
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

#endif /* NE_ASSET_NFONT_H */