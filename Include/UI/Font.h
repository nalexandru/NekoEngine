#ifndef _UI_FONT_H_
#define _UI_FONT_H_

#include <Engine/Types.h>

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

struct Font
{
	struct Glyph *glyphs;
	uint32_t glyphCount;
	Handle texture;
};

#endif /* _UI_FONT_H_ */

