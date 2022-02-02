#include <assert.h>

#include <Math/Math.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Scene/Components.h>
#include <Scene/Systems.h>

#include "Internal.h"

// FIXME
#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Resource.h>

struct NeUIUpdateBufferArgs
{
	struct NeUIVertex *vertices;
	uint16_t *indices;
	uint32_t vertexCount, indexCount;
};

struct NeUIConstants
{
	uint64_t vertexAddress;
	uint32_t texture;
	uint32_t __padding;
	struct mat4 mvp;
};

struct NeFont UI_sysFont;
NeBufferHandle UI_vertexBuffer, UI_indexBuffer;
uint32_t UI_maxVertices = 400;
uint64_t UI_vertexBufferSize;
uint32_t UI_maxIndices = 600;
uint64_t UI_indexBufferSize;
struct NeArray UI_standaloneContexts;

static uint8_t *_vertexPtr, *_indexPtr;

static bool _InitContext(struct NeUIContext *ctx, const void **);
static void _UpdateBuffers(void **comp, void *args);
static void _ResetContext(void **comp, void *args);
static void _TermContext(struct NeUIContext *ctx);

E_REGISTER_COMPONENT(UI_CONTEXT_COMP, struct NeUIContext, 1, _InitContext, _TermContext)
E_REGISTER_SYSTEM(UI_RESET_CONTEXT, ECSYS_GROUP_POST_RENDER, _ResetContext, 0, false, 1, UI_CONTEXT_COMP);
E_REGISTER_SYSTEM(UI_UPDATE_BUFFERS, ECSYS_GROUP_MANUAL, _UpdateBuffers, 0, true, 1, UI_CONTEXT_COMP);
E_REGISTER_SYSTEM(UI_DRAW_CONTEXT, ECSYS_GROUP_MANUAL, _UI_DrawContext, 0, true, 1, UI_CONTEXT_COMP);

bool
UI_InitUI(void)
{
	if (!UI_ResizeBuffers(UI_maxVertices, UI_maxIndices))
		return false;

	struct NeStream stm;
	if (!E_FileStream("/System/System.fnt", IO_READ, &stm))
		return false;

	E_LoadFontAsset(&stm, &UI_sysFont);
	E_CloseStream(&stm);

	return Rt_InitPtrArray(&UI_standaloneContexts, 5, MH_System);
}

void
UI_TermUI(void)
{
	Rt_TermArray(&UI_standaloneContexts);

	Re_UnmapBuffer(UI_vertexBuffer);
	Re_UnmapBuffer(UI_indexBuffer);

	E_UnloadResource(UI_sysFont.texture);
	Sys_Free(UI_sysFont.glyphs);

	Re_Destroy(UI_vertexBuffer);
	Re_Destroy(UI_indexBuffer);
}

void
UI_Update(struct NeScene *s)
{
	struct NeUIUpdateBufferArgs updateArgs =
	{
		.vertices = (struct NeUIVertex *)(_vertexPtr + (UI_vertexBufferSize * Re_frameId)),
		.indices = (uint16_t *)(_indexPtr + (UI_indexBufferSize * Re_frameId)),
		.vertexCount = 0,
		.indexCount = 0
	};
	E_ExecuteSystemS(s, UI_UPDATE_BUFFERS, &updateArgs);

	struct NeUIContext *ctx;
	Rt_ArrayForEachPtr(ctx, &UI_standaloneContexts)
		_UpdateBuffers((void **)&ctx, &updateArgs);
}

struct NeUIContext *
UI_CreateStandaloneContext(uint32_t vertexCount, uint32_t indexCount, uint32_t drawCallCount)
{
	struct NeUIContext *ctx = Sys_Alloc(sizeof(*ctx), 1, MH_System);

	if (!vertexCount) vertexCount = 64;
	if (!indexCount) indexCount = 100;
	if (!drawCallCount) drawCallCount = 10;

	if (!Rt_InitArray(&ctx->vertices, vertexCount, sizeof(struct NeUIVertex), MH_Render))
		return false;

	if (!Rt_InitArray(&ctx->indices, indexCount, sizeof(uint16_t), MH_Render))
		return false;

	if (!Rt_InitArray(&ctx->draws, drawCallCount, sizeof(struct NeUIDrawCmd), MH_Render))
		return false;

	Rt_ArrayAddPtr(&UI_standaloneContexts, ctx);

	return ctx;
}

void
UI_DestroyStandaloneContext(struct NeUIContext *ctx)
{
	Rt_ArrayRemove(&UI_standaloneContexts, Rt_PtrArrayFindId(&UI_standaloneContexts, ctx));
	_TermContext(ctx);
	Sys_Free(ctx);
}

bool
UI_ResizeBuffers(uint32_t maxVertices, uint32_t maxIndices)
{
	if (UI_vertexBuffer)
		Re_Destroy(UI_vertexBuffer);

	if (UI_indexBuffer)
		Re_Destroy(UI_indexBuffer);

	UI_maxVertices = maxVertices;
	UI_vertexBufferSize = (uint64_t)UI_maxVertices * sizeof(struct NeUIVertex);

	UI_maxIndices = maxIndices;
	UI_indexBufferSize = (uint64_t)UI_maxIndices * sizeof(uint16_t);

	struct NeBufferCreateInfo vtxInfo =
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

	struct NeBufferCreateInfo idxInfo =
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

	return _vertexPtr && _indexPtr;
}

void
_UI_DrawContext(void **comp, void *args)
{
	struct NeUIContext *ctx = comp[0];
	struct NeUIConstants *c = args;

	for (size_t i = 0; i < ctx->draws.count; ++i) {
		const struct NeUIDrawCmd dc = *(const struct NeUIDrawCmd *)Rt_ArrayGet(&ctx->draws, i);

		if (!dc.idxCount || !dc.vtxCount)
			continue;

		c->texture = E_ResHandleToGPU(dc.texture);
		Re_CmdPushConstants(SS_ALL, sizeof(*c), c);

		Re_CmdDrawIndexed(dc.idxCount, 1, dc.idxOffset, dc.vtxOffset, 0);
	}

	Rt_ClearArray(&ctx->vertices, false);
	Rt_ClearArray(&ctx->indices, false);
	Rt_ClearArray(&ctx->draws, false);
}

static bool
_InitContext(struct NeUIContext *ctx, const void **args)
{
	uint32_t vertexCount = 64, indexCount = 100, drawCallCount = 10;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "VertexCount", len))
			vertexCount = atoi((char *)*(++args));
		else if (!strncmp(arg, "IndexCount", len))
			indexCount = atoi((char *)*(++args));
		else if (!strncmp(arg, "DrawCallCount", len))
			drawCallCount = atoi((char *)*(++args));
	}

	if (!Rt_InitArray(&ctx->vertices, vertexCount, sizeof(struct NeUIVertex), MH_Render))
		return false;

	if (!Rt_InitArray(&ctx->indices, indexCount, sizeof(uint16_t), MH_Render))
		return false;

	if (!Rt_InitArray(&ctx->draws, drawCallCount, sizeof(struct NeUIDrawCmd), MH_Render))
		return false;

	return true;
}

static void
_UpdateBuffers(void **comp, void *a)
{
	size_t i = 0;
	struct NeUIContext *ctx = comp[0];
	struct NeUIUpdateBufferArgs *args = a;
	static bool resize = false;

	if (resize) {
		UI_ResizeBuffers(UI_maxVertices * 2, UI_maxIndices * 2);
		resize = false;
	}

	uint16_t vtxOffset = args->vertexCount;

	for (i = 0; i < ctx->draws.count; ++i) {
		struct NeUIDrawCmd dc = *(struct NeUIDrawCmd *)Rt_ArrayGet(&ctx->draws, i);

		if ((args->vertexCount + dc.vtxCount > UI_maxVertices) || (args->indexCount + dc.idxCount > UI_maxIndices)) {
			dc.vtxOffset = dc.vtxCount = dc.idxOffset = dc.idxCount = 0;
			resize = true;
			continue;
		} else {
			memcpy(args->vertices, Rt_ArrayGet(&ctx->vertices, dc.vtxOffset), sizeof(*args->vertices) * dc.vtxCount);
			args->vertices += dc.vtxCount;

			memcpy(args->indices, Rt_ArrayGet(&ctx->indices, dc.idxOffset), sizeof(*args->indices) * dc.idxCount);
			args->indices += dc.idxCount;

			dc.vtxOffset = vtxOffset;
			args->vertexCount += dc.vtxCount;

			dc.idxOffset = args->indexCount;
			args->indexCount += dc.idxCount;
		}

		*(struct NeUIDrawCmd *)Rt_ArrayGet(&ctx->draws, i) = dc;
	}
}

static void
_ResetContext(void **comp, void *args)
{
	struct NeUIContext *ctx = comp[0];

	Rt_ClearArray(&ctx->vertices, false);
	Rt_ClearArray(&ctx->indices, false);
	Rt_ClearArray(&ctx->draws, false);
}

static void
_TermContext(struct NeUIContext *ctx)
{
	Rt_TermArray(&ctx->vertices);
	Rt_TermArray(&ctx->indices);
	Rt_TermArray(&ctx->draws);
}
