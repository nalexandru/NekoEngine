#include <UI/Text.h>
#include <UI/Font.h>
#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Config.h>
#include <Engine/Resource.h>
#include <Render/Texture.h>
#include <Runtime/Runtime.h>

Array _textVertices, _textIndices;

static struct Font _sysFont;

static int _compare(const void *key, const void *elem);

void
UI_DrawText(float px, float py, float size, const wchar_t *text)
{
	struct TextVertex v;
	
	uint16_t idxStart = (uint16_t)_textIndices.count;
	v.color[0] = v.color[1] = v.color[2] = v.color[3] = 1.f;

	while (*text) {
		struct Glyph *g = NULL;
		float x, y, w, h;

	/*	g = bsearch(text, _glyphs, _glyphCount, sizoef(*_glyphs), _compre)

		if (!g)
			g = &_glyphs[*text - 0x20]; */

		if (*text > 127)
			g = &_sysFont.glyphs[95];
		else
			g = &_sysFont.glyphs[*text - 0x20];

		x = px + g->bearing.x;
		y = py;
		w = (float)g->size.w;
		h = (float)g->size.h;

		v.posUv[0] = x;
		v.posUv[1] = y;
		v.posUv[2] = g->u;
		v.posUv[3] = g->v + g->th;
		Rt_ArrayAdd(&_textVertices, &v);

		v.posUv[0] = x;
		v.posUv[1] = y + h;
		v.posUv[2] = g->u;
		v.posUv[3] = g->v;
		Rt_ArrayAdd(&_textVertices, &v);

		v.posUv[0] = x + w;
		v.posUv[1] = y + h;
		v.posUv[2] = g->u + g->tw;
		v.posUv[3] = g->v;
		Rt_ArrayAdd(&_textVertices, &v);

		v.posUv[0] = x + w;
		v.posUv[1] = y;
		v.posUv[2] = g->u + g->tw;
		v.posUv[3] = g->v + g->th;
		Rt_ArrayAdd(&_textVertices, &v);

		*(uint16_t *)Rt_ArrayAllocate(&_textIndices) = idxStart;
		*(uint16_t *)Rt_ArrayAllocate(&_textIndices) = idxStart + 1;
		*(uint16_t *)Rt_ArrayAllocate(&_textIndices) = idxStart + 2;

		*(uint16_t *)Rt_ArrayAllocate(&_textIndices) = idxStart;
		*(uint16_t *)Rt_ArrayAllocate(&_textIndices) = idxStart + 2;
		*(uint16_t *)Rt_ArrayAllocate(&_textIndices) = idxStart + 3;
	}
}

bool
UI_InitText(void)
{
	bool rc;
	struct Stream stm;

	Rt_InitArray(&_textVertices, 100, sizeof(struct TextVertex));

	E_FileStream("/System/System.fnt", IO_READ, &stm);
	rc = E_LoadFontAsset(&stm, &_sysFont);
	E_CloseStream(&stm);

	return rc;
}

void
UI_TermText(void)
{
	E_UnloadResource(_sysFont.texture);
	free(_sysFont.glyphs);

	Rt_TermArray(&_textVertices);
}

int
_compare(const void *key, const void *elem)
{
return 0;
//	return *(uint8_t *)key - 
}

