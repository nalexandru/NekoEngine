#include <assert.h>

#include <Math/Math.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>

#include "Internal.h"

// FIXME
#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Resource.h>

struct UpdateBufferArgs
{
	struct UIVertex *vertices;
	uint16_t *indices;
	uint32_t vertexCount, indexCount;
};

struct UIConstants
{
	uint64_t vertexAddress;
	uint32_t texture;
	uint32_t __padding;
	struct mat4 mvp;
};

struct Font UI_sysFont;
BufferHandle UI_vertexBuffer, UI_indexBuffer;
uint64_t UI_vertexBufferSize = 4000 * sizeof(struct UIVertex);
uint64_t UI_indexBufferSize = 6000 * sizeof(uint16_t);

static uint8_t *_vertexPtr, *_indexPtr;

bool
UI_InitUI(void)
{
	struct BufferCreateInfo vtxInfo =
	{
		.desc =
		{
			.size = UI_vertexBufferSize * RE_NUM_FRAMES,
			.usage = BU_STORAGE_BUFFER | BU_TRANSFER_DST,
			.memoryType = MT_CPU_COHERENT
		},
	};
	Re_CreateBuffer(&vtxInfo, &UI_vertexBuffer);
	_vertexPtr = Re_MapBuffer(UI_vertexBuffer);

	struct BufferCreateInfo idxInfo =
	{
		.desc =
		{
			.size = UI_indexBufferSize * RE_NUM_FRAMES,
			.usage = BU_INDEX_BUFFER | BU_TRANSFER_DST,
			.memoryType = MT_CPU_COHERENT
		},
	};
	Re_CreateBuffer(&idxInfo, &UI_indexBuffer);
	_indexPtr = Re_MapBuffer(UI_indexBuffer);

	struct Stream stm;
	if (!E_FileStream("/System/System.fnt", IO_READ, &stm))
		return false;

	E_LoadFontAsset(&stm, &UI_sysFont);
	E_CloseStream(&stm);

	return true;
}

void
UI_TermUI(void)
{
	Re_UnmapBuffer(UI_vertexBuffer);
	Re_UnmapBuffer(UI_indexBuffer);

	E_UnloadResource(UI_sysFont.texture);
	Sys_Free(UI_sysFont.glyphs);

	Re_Destroy(UI_vertexBuffer);
	Re_Destroy(UI_indexBuffer);
}

void
UI_Update(struct Scene *s)
{
	struct UpdateBufferArgs updateArgs =
	{
		.vertices = (struct UIVertex *)(_vertexPtr + (UI_vertexBufferSize * Re_frameId)),
		.indices = (uint16_t *)(_indexPtr + (UI_indexBufferSize * Re_frameId)),
		.vertexCount = 0,
		.indexCount = 0
	};
	E_ExecuteSystemS(s, UI_UPDATE_BUFFERS, &updateArgs);
}

bool
UI_InitContext(struct UIContext *ctx, const void **args)
{
	uint32_t vertexCount = 64, indexCount = 100, drawCallCount = 10;

	for (; *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "VertexCount", len))
			vertexCount = atoi((char *)*(++args));
		else if (!strncmp(arg, "IndexCount", len))
			indexCount = atoi((char *)*(++args));
		else if (!strncmp(arg, "DrawCallCount", len))
			drawCallCount = atoi((char *)*(++args));
	}

	if (!Rt_InitArray(&ctx->vertices, vertexCount, sizeof(struct UIVertex), MH_Render))
		return false;

	if (!Rt_InitArray(&ctx->indices, indexCount, sizeof(uint16_t), MH_Render))
		return false;

	if (!Rt_InitArray(&ctx->draws, drawCallCount, sizeof(struct UIDrawCmd), MH_Render))
		return false;

	return true;
}

void
UI_TermContext(struct UIContext *ctx)
{
	Rt_TermArray(&ctx->vertices);
	Rt_TermArray(&ctx->indices);
	Rt_TermArray(&ctx->draws);
}

void
UI_ResetContext(void **comp, void *args)
{
	struct UIContext *ctx = comp[0];

	Rt_ClearArray(&ctx->vertices, false);
	Rt_ClearArray(&ctx->indices, false);
	Rt_ClearArray(&ctx->draws, false);
}

void
UI_UpdateBuffers(void **comp, void *a)
{
	size_t i = 0;
	struct UIContext *ctx = comp[0];
	struct UpdateBufferArgs *args = a;

	for (i = 0; i < ctx->draws.count; ++i) {
		struct UIDrawCmd dc = *(struct UIDrawCmd *)Rt_ArrayGet(&ctx->draws, i);

		assert(args->vertexCount + dc.vtxCount < 1000);
		assert(args->indexCount + dc.idxCount < 4000);

		memcpy(args->vertices, Rt_ArrayGet(&ctx->vertices, dc.vtxOffset), sizeof(*args->vertices) * dc.vtxCount);
		args->vertices += dc.vtxCount;

		memcpy(args->indices, Rt_ArrayGet(&ctx->indices, dc.idxOffset), sizeof(*args->indices) * dc.idxCount);
		args->indices += dc.idxCount;

		args->vertexCount += dc.vtxCount;

		dc.idxOffset = args->indexCount;
		args->indexCount += dc.idxCount;

		*(struct UIDrawCmd *)Rt_ArrayGet(&ctx->draws, i) = dc;
	}
}

void
UI_DrawContext(void **comp, void *args)
{
	struct UIContext *ctx = comp[0];
	struct UIConstants *c = args;

	for (size_t i = 0; i < ctx->draws.count; ++i) {
		const struct UIDrawCmd dc = *(const struct UIDrawCmd *)Rt_ArrayGet(&ctx->draws, i);

		c->texture = E_ResHandleToGPU(dc.texture);
		Re_CmdPushConstants(SS_ALL, sizeof(*c), c);

		Re_CmdDrawIndexed(dc.idxCount, 1, dc.idxOffset, 0, 0);
	}

	Rt_ClearArray(&ctx->vertices, false);
	Rt_ClearArray(&ctx->indices, false);
	Rt_ClearArray(&ctx->draws, false);
}
