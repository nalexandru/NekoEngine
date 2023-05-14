#include <Engine/Job.h>
#include <Engine/Config.h>
#include <Engine/Component.h>
#include <Scene/Scene.h>
#include <Runtime/Runtime.h>
#include <Render/Graph/Graph.h>
#include <Render/Graph/Pass.h>
#include <Render/DestroyResource.h>
#include <Render/TransientResources.h>

#include "../Internal.h"

#define GRAPH_MOD	"RenderGraph"

enum NeGraphResourceType
{
	PRT_TEXTURE,
	PRT_BUFFER,
	PRT_DATA
};

struct NeGraphResource
{
	uint64_t hash;
	struct {
		enum NeGraphResourceType type;
		union {
			struct {
				struct NeTextureDesc texture;
				uint16_t location;
			};
			struct NeBufferDesc buffer;
		};
	} info;
	union {
		struct NeTexture *texture;
		struct {
			uint64_t bufferAddress;
			NeBufferHandle buffer;
			struct NeBuffer *bufferPtr;
		};
		void *hostData;
	} handle;
};

struct NePassData
{
	void *data;
	struct NeRenderPass procs;
};

struct NeRenderPassInfo
{
	uint64_t hash;
	const struct NeRenderPass *pass;
};

static struct NeArray f_renderPasses;

static struct NeGraphResource *GetResource(uint64_t hash, const struct NeArray *resources);

void
Re_RegisterPass(const char *name, const struct NeRenderPass *pass)
{
	if (!f_renderPasses.data)
		Rt_InitArray(&f_renderPasses, 10, sizeof(struct NeRenderPassInfo), MH_Render);

	struct NeRenderPassInfo info = { .hash = Rt_HashString(name), .pass = pass };
	Rt_ArrayAdd(&f_renderPasses, &info);
}

bool
Re_AddGraphTexture(const char *name, const struct NeTextureDesc *desc, uint16_t location, struct NeArray *resources)
{
	struct NeGraphResource res = { .hash = Rt_HashString(name), .info = { .type = PRT_TEXTURE, .texture = *desc, .location = location } };
	if (GetResource(res.hash, resources))
		return false;

	return Rt_ArrayAdd(resources, &res);
}

bool
Re_AddGraphBuffer(const char *name, const struct NeBufferDesc *desc, struct NeArray *resources)
{
	struct NeGraphResource res = { .hash = Rt_HashString(name), .info = { .type = PRT_BUFFER, .buffer = *desc } };
	if (GetResource(res.hash, resources))
		return false;

	return Rt_ArrayAdd(resources, &res);
}

bool
Re_AddGraphData(const char *name, void *ptr, struct NeArray *resources)
{
	struct NeGraphResource res = { .hash = Rt_HashString(name), .info = { .type = PRT_DATA } };
	if (GetResource(res.hash, resources))
		return false;

	return Rt_ArrayAdd(resources, &res);
}

struct NeTexture *
Re_GraphTexture(uint64_t hash, const struct NeArray *resources, uint32_t *location, struct NeTextureDesc **desc)
{
	struct NeGraphResource *res = GetResource(hash, resources);
	if (!res || res->info.type != PRT_TEXTURE)
		return NULL;

	if (location)
		*location = res->info.location;

	if (desc)
		*desc = &res->info.texture;

	return res->handle.texture;
}

uint16_t
Re_GraphTextureLocation(uint64_t hash, const struct NeArray *resources)
{
	struct NeGraphResource *res = GetResource(hash, resources);
	if (!res || res->info.type != PRT_TEXTURE)
		return 0;
	return res->info.location;
}

struct NeTexture *
Re_GraphTexturePtr(uint64_t hash, const struct NeArray *resources)
{
	struct NeGraphResource *res = GetResource(hash, resources);
	if (!res || res->info.type != PRT_TEXTURE)
		return NULL;
	return res->handle.texture;
}

const struct NeTextureDesc *
Re_GraphTextureDesc(uint64_t hash, const struct NeArray *resources)
{
	struct NeGraphResource *res = GetResource(hash, resources);
	if (!res)
		return NULL;
	return res->info.type == PRT_TEXTURE ? &res->info.texture : 0;
}

uint64_t
Re_GraphBuffer(uint64_t hash, const struct NeArray *resources, struct NeBuffer **buff)
{
	struct NeGraphResource *res = GetResource(hash, resources);
	if (!res)
		return 0;

	if (buff)
		*buff = res->handle.bufferPtr;

	return res->info.type == PRT_BUFFER ? res->handle.bufferAddress : 0;
}

void *
Re_GraphData(uint64_t hash, const struct NeArray *resources)
{
	struct NeGraphResource *res = GetResource(hash, resources);
	if (!res)
		return 0;
	return res->info.type == PRT_DATA ? res->handle.hostData : NULL;
}

struct NeRenderGraph *
Re_CreateGraph(void)
{
	struct NeRenderGraph *g = (struct NeRenderGraph *)Sys_Alloc(sizeof(*g), 1, MH_Render);
	if (!g)
		return 0;

	Rt_InitArray(&g->allPasses, 10, sizeof(struct NePassData), MH_Render);
	Rt_InitArray(&g->resources, 10, sizeof(struct NeGraphResource), MH_Render);

	g->semaphore = Re_CreateSemaphore();

	return g;
}

struct NeRenderGraph *
Re_CreateDefaultGraph(void)
{
	struct NeRenderGraph *g = Re_CreateGraph();

	Re_AddPass(g, Rt_HashLiteral("NeSkinning"));
	Re_AddPass(g, Rt_HashLiteral("NeDepthPrePass"));

	if (E_GetCVarBln("Render.SSAO_Enable", true)->bln)
		Re_AddPass(g, Rt_HashLiteral("NeSSAO"));

	Re_AddPass(g, Rt_HashLiteral("NeLightCulling"));
	Re_AddPass(g, Rt_HashLiteral("NeOpaquePass"));
	Re_AddPass(g, Rt_HashLiteral("NeSkyPass"));
	Re_AddPass(g, Rt_HashLiteral("NeTransparentPass"));

	if (E_GetCVarBln("Render.Debug_DrawObjectBounds", false)->bln)
		Re_AddPass(g, Rt_HashLiteral("NeDebugBoundsPass"));

	if (E_GetCVarBln("Render.Debug_DrawLightBounds", false)->bln)
		Re_AddPass(g, Rt_HashLiteral("NeLightBoundsPass"));

	Re_AddPass(g, Rt_HashString(E_GetCVarStr("Render_UIPass", "NeUIPass")->str));

	return g;
}

bool
Re_AddPass(struct NeRenderGraph *g, uint64_t name)
{
	struct NeRenderPassInfo *info = (struct NeRenderPassInfo *)Rt_ArrayFind(&f_renderPasses, &name, Rt_U64CmpFunc);
	if (!info)
		return false;

	struct NePassData pd = { .procs = *info->pass };
	if (!info->pass->Init(&pd.data))
		return false;

	Rt_ArrayAdd(&g->allPasses, &pd);

	return true;
}

void
Re_BuildGraph(struct NeRenderGraph *g, const struct NeTextureDesc *outputDesc, struct NeTexture *output)
{
	struct NeScene *s = Scn_activeScene;

	Rt_InitArray(&g->execPasses, g->allPasses.count, g->allPasses.elemSize, MH_Transient);
	Rt_ClearArray(&g->resources, false);

	struct NeGraphResource res =
	{
		.hash = Rt_HashLiteral(RE_OUTPUT),
		.info = { .type = PRT_TEXTURE },
		.handle = { .texture = output }
	};
	memcpy(&res.info.texture, outputDesc, sizeof(res.info.texture));
	Rt_ArrayAdd(&g->resources, &res);

	uint64_t sceneAddr, instAddr;
	Scn_DataAddress(s, &sceneAddr, &instAddr);

	res.hash = Rt_HashLiteral(RE_CAMERA);
	res.info.type = PRT_DATA;
	res.handle.hostData = E_ComponentPtrS(s, s->camera);
	Rt_ArrayAdd(&g->resources, &res);

	res.hash = Rt_HashLiteral(RE_SCENE_DATA);
	res.info.type = PRT_BUFFER,
	res.handle.bufferAddress = sceneAddr;
	res.handle.buffer = s->sceneData;
	Rt_ArrayAdd(&g->resources, &res);

	res.hash = Rt_HashLiteral(RE_SCENE_INSTANCES);
	res.info.type = PRT_BUFFER,
	res.handle.bufferAddress = instAddr;
	res.handle.buffer = s->sceneData;
	Rt_ArrayAdd(&g->resources, &res);

	res.hash = Rt_HashLiteral(RE_PASS_SEMAPHORE);
	res.info.type = PRT_DATA;
	res.handle.hostData = g->semaphore;
	Rt_ArrayAdd(&g->resources, &res);

	struct NePassData *pd; 
	Rt_ArrayForEach(pd, &g->allPasses, struct NePassData *)
		if (pd->procs.Setup(pd->data, &g->resources))
			Rt_ArrayAdd(&g->execPasses, pd);

	struct NeGraphResource *gr;
	Rt_ArrayForEach(gr, &g->resources, struct NeGraphResource *) {
		if (gr->handle.hostData)
			continue;

		if (gr->info.type == PRT_TEXTURE) {
			gr->handle.texture = Re_CreateTransientTexture(&gr->info.texture, gr->info.location);
			Re_Destroy(gr->handle.texture);
		} else if (gr->info.type == PRT_BUFFER) {
			if (!gr->handle.buffer)
				Re_ReserveBufferId(&gr->handle.buffer);

			gr->handle.bufferPtr = Re_CreateTransientBuffer(&gr->info.buffer, gr->handle.buffer);
			gr->handle.bufferAddress = Re_BufferAddress(gr->handle.buffer, 0);
			Re_Destroy(gr->handle.buffer);
		}
	}
}

void
Re_ExecuteGraph(struct NeRenderGraph *g)
{
	struct NePassData *pd;
	Rt_ArrayForEach(pd, &g->execPasses, struct NePassData *)
		pd->procs.Execute(pd->data, &g->resources);
}

void
Re_DestroyGraph(struct NeRenderGraph *g)
{
	if (!g)
		return;

	Rt_TermArray(&g->resources);

	struct NePassData *pd; 
	Rt_ArrayForEach(pd, &g->allPasses, struct NePassData *)
		pd->procs.Term(pd->data);
	Rt_TermArray(&g->allPasses);

	Re_DestroySemaphore(g->semaphore);

	Sys_Free(g);
}

void
ReP_DestroyRenderPasses(void)
{
	Rt_TermArray(&f_renderPasses);
}

static struct NeGraphResource *
GetResource(uint64_t hash, const struct NeArray *resources)
{
	struct NeGraphResource *gr;
	Rt_ArrayForEach(gr, resources, struct NeGraphResource *)
		if (gr->hash == hash)
			return gr;
	return NULL;
}

/* NekoEngine
 *
 * Graph.c
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
