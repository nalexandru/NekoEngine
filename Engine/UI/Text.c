#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Config.h>
#include <Engine/Resource.h>
#include <Runtime/Runtime.h>

#include "Internal.h"

static int _compare(const void *key, const void *elem);

void
UI_DrawText(struct NeUIContext *ctx, const char *text, float px, float py, float size, struct NeFont *font)
{
	float sizeFactor;
	struct NeUIVertex v;
	struct NeUIDrawCmd drawCmd;

	if (!text[0])
		return;

	if (!font)
		font = &UI_sysFont;

	drawCmd.idxCount = drawCmd.vtxCount = 0;
	drawCmd.vtxOffset = (uint16_t)ctx->vertices.count;
	drawCmd.idxOffset = (uint16_t)ctx->indices.count;
	drawCmd.texture = font->texture;

	v.color[0] = v.color[1] = v.color[2] = v.color[3] = 1.f;

	sizeFactor = size / 10.f;
	py += size;

	uint16_t vtxOffset = drawCmd.vtxOffset;
	for (; *text; ++text) {
		const struct NeGlyph *g = NULL;

		//
		g = bsearch(text, font->glyphs, font->glyphCount, sizeof(*font->glyphs), _compare);

		if (!g)
			g = &font->glyphs[*text - 0x20];
		//

		if (*text > 127)
			g = &font->glyphs[95];
		else
			g = &font->glyphs[*text - 0x20];

		const float x = px + (g->bearing.x * sizeFactor);
		const float y = py - (g->bearing.y * sizeFactor);
		const float w = (float)g->size.w * sizeFactor;
		const float h = (float)g->size.h * sizeFactor;
		px += g->adv * sizeFactor;

		v.posUv[0] = x;
		v.posUv[1] = y;
		v.posUv[2] = g->u;
		v.posUv[3] = g->v;
		Rt_ArrayAdd(&ctx->vertices, &v);

		v.posUv[0] = x;
		v.posUv[1] = y + h;
		v.posUv[2] = g->u;
		v.posUv[3] = g->v + g->th;
		Rt_ArrayAdd(&ctx->vertices, &v);

		v.posUv[0] = x + w;
		v.posUv[1] = y + h;
		v.posUv[2] = g->u + g->tw;
		v.posUv[3] = g->v + g->th;
		Rt_ArrayAdd(&ctx->vertices, &v);

		v.posUv[0] = x + w;
		v.posUv[1] = y;
		v.posUv[2] = g->u + g->tw;
		v.posUv[3] = g->v;
		Rt_ArrayAdd(&ctx->vertices, &v);

		*(uint16_t *)Rt_ArrayAllocate(&ctx->indices) = vtxOffset;
		*(uint16_t *)Rt_ArrayAllocate(&ctx->indices) = vtxOffset + 1;
		*(uint16_t *)Rt_ArrayAllocate(&ctx->indices) = vtxOffset + 2;

		*(uint16_t *)Rt_ArrayAllocate(&ctx->indices) = vtxOffset;
		*(uint16_t *)Rt_ArrayAllocate(&ctx->indices) = vtxOffset + 2;
		*(uint16_t *)Rt_ArrayAllocate(&ctx->indices) = vtxOffset + 3;

		vtxOffset += 4;
		drawCmd.vtxCount += 4;
		drawCmd.idxCount += 6;
	}

	Rt_ArrayAdd(&ctx->draws, &drawCmd);
}

int
_compare(const void *key, const void *elem)
{
	return 0;
//	return *(uint8_t *)key - 
}
