#ifndef _NE_UI_FONT_H_
#define _NE_UI_FONT_H_

#include <Engine/Types.h>

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

struct NeFont
{
	struct NeGlyph *glyphs;
	uint32_t glyphCount;
	NeHandle texture;
};

#endif /* _NE_UI_FONT_H_ */
