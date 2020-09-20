#include <UI/Text.h>
#include <UI/Font.h>
#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Config.h>
#include <Engine/Resource.h>
#include <UI/UI.h>
#include <Render/Texture.h>
#include <Runtime/Runtime.h>

static struct Font _sysFont;

static int _compare(const void *key, const void *elem);

void
UI_DrawText(struct UIContext *ctx, const wchar_t *text, float px, float py, float size, struct Font *font)
{
	uint16_t vtxOffset;
	struct UIVertex v;
	struct UIDrawCall drawCall;
	
	if (!font)
		font = &_sysFont;

	drawCall.idxCount = drawCall.vtxCount = 0;
	drawCall.vtxOffset = (uint16_t)ctx->vertices.count;
	drawCall.idxOffset = (uint16_t)ctx->indices.count;
	drawCall.texture = font->texture;

	v.color[0] = v.color[1] = v.color[2] = v.color[3] = 1.f;

	vtxOffset = drawCall.vtxOffset;
	while (*text) {
		struct Glyph *g = NULL;
		float x, y, w, h;

	/*	g = bsearch(text, _glyphs, _glyphCount, sizoef(*_glyphs), _compre)

		if (!g)
			g = &_glyphs[*text - 0x20]; */

		if (*text > 127)
			g = &font->glyphs[95];
		else
			g = &font->glyphs[*text - 0x20];

		x = px + g->bearing.x;
		y = py;
		w = (float)g->size.w;
		h = (float)g->size.h;

		v.posUv[0] = x;
		v.posUv[1] = y;
		v.posUv[2] = g->u;
		v.posUv[3] = g->v + g->th;
		Rt_ArrayAdd(&ctx->vertices, &v);

		v.posUv[0] = x;
		v.posUv[1] = y + h;
		v.posUv[2] = g->u;
		v.posUv[3] = g->v;
		Rt_ArrayAdd(&ctx->vertices, &v);

		v.posUv[0] = x + w;
		v.posUv[1] = y + h;
		v.posUv[2] = g->u + g->tw;
		v.posUv[3] = g->v;
		Rt_ArrayAdd(&ctx->vertices, &v);

		v.posUv[0] = x + w;
		v.posUv[1] = y;
		v.posUv[2] = g->u + g->tw;
		v.posUv[3] = g->v + g->th;
		Rt_ArrayAdd(&ctx->vertices, &v);

		*(uint16_t *)Rt_ArrayAllocate(&ctx->indices) = vtxOffset;
		*(uint16_t *)Rt_ArrayAllocate(&ctx->indices) = vtxOffset + 1;
		*(uint16_t *)Rt_ArrayAllocate(&ctx->indices) = vtxOffset + 2;

		*(uint16_t *)Rt_ArrayAllocate(&ctx->indices) = vtxOffset;
		*(uint16_t *)Rt_ArrayAllocate(&ctx->indices) = vtxOffset + 2;
		*(uint16_t *)Rt_ArrayAllocate(&ctx->indices) = vtxOffset + 3;

		vtxOffset += 4;
		drawCall.vtxCount += 4;
		drawCall.idxCount += 6;
	}

	Rt_ArrayAdd(&ctx->draws, &drawCall);
}

bool
UI_InitText(void)
{
	bool rc;
	struct Stream stm;

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
}

int
_compare(const void *key, const void *elem)
{
return 0;
//	return *(uint8_t *)key - 
}

