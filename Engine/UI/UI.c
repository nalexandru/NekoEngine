#include <assert.h>

#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Scene/Components.h>
#include <Scene/Systems.h>
#include <System/Log.h>

#include "Internal.h"

#define UI_MOD	"UI"

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
	struct NeMatrix mvp;
};

struct NeFont UI_sysFont;
NeBufferHandle UI_vertexBuffer, UI_indexBuffer;
uint32_t UI_maxVertices = 400;
uint64_t UI_vertexBufferSize;
uint32_t UI_maxIndices = 600;
uint64_t UI_indexBufferSize;
struct NeArray UI_standaloneContexts;
bool UI_pluginLoaded = false;

static uint8_t *f_vertexPtr, *f_indexPtr;

static bool InitContext(struct NeUIContext *ctx, const void **);
static void UpdateBuffers(void **comp, void *args);
static void ResetContext(void **comp, void *args);
static void TermContext(struct NeUIContext *ctx);

NE_REGISTER_COMPONENT(NE_UI_CONTEXT, struct NeUIContext, 1, InitContext, NULL, TermContext)
NE_REGISTER_SYSTEM(UI_RESET_CONTEXT, ECSYS_GROUP_POST_RENDER, ResetContext, 0, false, 1, NE_UI_CONTEXT);
NE_REGISTER_SYSTEM(UI_UPDATE_BUFFERS, ECSYS_GROUP_MANUAL, UpdateBuffers, 0, true, 1, NE_UI_CONTEXT);
NE_REGISTER_SYSTEM(UI_DRAW_CONTEXT, ECSYS_GROUP_MANUAL, UI_p_DrawContext, 0, true, 1, NE_UI_CONTEXT);

bool
UI_InitUI(void)
{
	if (UI_pluginLoaded)
		return true;

	if (!UI_ResizeBuffers(UI_maxVertices, UI_maxIndices))
		return false;

	struct NeStream stm;
	if (!E_FileStream("/System/System.fnt", IO_READ, &stm))
		return false;

	Asset_LoadFont(&stm, &UI_sysFont);
	E_CloseStream(&stm);

	if (!Rt_InitPtrArray(&UI_standaloneContexts, 5, MH_System))
		return false;

	UI_p_InitConsoleOutput();

	return true;
}

void
UI_TermUI(void)
{
	if (UI_pluginLoaded)
		return;

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
		.vertices = (struct NeUIVertex *)(f_vertexPtr + (UI_vertexBufferSize * Re_frameId)),
		.indices = (uint16_t *)(f_indexPtr + (UI_indexBufferSize * Re_frameId)),
		.vertexCount = 0,
		.indexCount = 0
	};
	E_ExecuteSystemS(s, 0xe492af1852043c50llu /* Rt_HashLiteral(UI_UPDATE_BUFFERS) */, &updateArgs);

	struct NeUIContext *ctx;
	Rt_ArrayForEachPtr(ctx, &UI_standaloneContexts)
		UpdateBuffers((void **)&ctx, &updateArgs);
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
	TermContext(ctx);
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
	f_vertexPtr = Re_MapBuffer(UI_vertexBuffer);

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
	f_indexPtr = Re_MapBuffer(UI_indexBuffer);

	return f_vertexPtr && f_indexPtr;
}

void
UI_p_DrawContext(void **comp, void *args)
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
InitContext(struct NeUIContext *ctx, const void **args)
{
	if (UI_pluginLoaded) {
		Sys_LogEntry(UI_MOD, LOG_CRITICAL, "The UI system is disabled because a replacement plugin was loaded.");
		return false;
	}

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
UpdateBuffers(void **comp, void *a)
{
	struct NeUIContext *ctx = comp[0];
	struct NeUIUpdateBufferArgs *args = a;
	static bool resize = false;

	if (resize) {
		UI_ResizeBuffers(UI_maxVertices * 2, UI_maxIndices * 2);
		resize = false;
	}

	uint16_t vtxOffset = args->vertexCount;

	for (size_t i = 0; i < ctx->draws.count; ++i) {
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
ResetContext(void **comp, void *args)
{
	struct NeUIContext *ctx = comp[0];

	Rt_ClearArray(&ctx->vertices, false);
	Rt_ClearArray(&ctx->indices, false);
	Rt_ClearArray(&ctx->draws, false);
}

static void
TermContext(struct NeUIContext *ctx)
{
	Rt_TermArray(&ctx->vertices);
	Rt_TermArray(&ctx->indices);
	Rt_TermArray(&ctx->draws);
}

/* NekoEngine
 *
 * UI.c
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
