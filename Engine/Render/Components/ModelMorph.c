#include <stdlib.h>

#include <System/Memory.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Components/ModelMorph.h>
#include <Render/Components/ModelRender.h>
#include <Engine/Resource.h>
#include <Engine/ECSystem.h>
#include <Scene/Components.h>

struct MorphInfo
{
	uint64_t name;
	float value;
};

static bool _InitModelMorph(struct NeModelMorph *mr, const void **args);
static void _TermModelMorph(struct NeModelMorph *mr);

E_REGISTER_COMPONENT(MODEL_MORPH_COMP, struct NeModelMorph, 1, _InitModelMorph, _TermModelMorph)

void
Re_SetMorph(struct NeModelMorph *mm, const char *morph, float value)
{
	uint64_t hash = Rt_HashString(morph);
	
	struct MorphInfo *mi = NULL;
	Rt_ArrayForEach(mi, &mm->activeMorphs) {
		if (mi->name == hash)
			break;
		
		mi = NULL;
	}
	
	if (mi) {
		mi->value = value;
	} else {
		struct MorphInfo m = { .name = hash, .value = value };
		Rt_ArrayAdd(&mm->activeMorphs, &m);
	}
}

static bool
_InitModelMorph(struct NeModelMorph *mm, const void **args)
{
	if (!Rt_InitArray(&mm->activeMorphs, 10, sizeof(struct MorphInfo), MH_Render))
		return false;
	
	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "Morph", len)) {
			//model = E_LoadResource(*(++args), RES_MODEL);
		} else {
			struct MorphInfo mi =
			{
				.name = Rt_HashString(arg),
				.value = (float)atof(*(++args))
			};
			Rt_ArrayAdd(&mm->activeMorphs, &mi);
		}
	}
	
	return true;
}

static void
_TermModelMorph(struct NeModelMorph *mm)
{
	Rt_TermArray(&mm->activeMorphs);
	
	if (mm->vertexBuffer)
		Re_Destroy(mm->vertexBuffer);
}

E_SYSTEM(RE_MORPH_MODELS, ECSYS_GROUP_PRE_RENDER, ECSYS_PRI_MORPH, true, void, 2, MODEL_RENDER_COMP, MODEL_MORPH_COMP)
{
	struct NeModelRender *mr = comp[0];
	struct NeModelMorph *mm = comp[1];
	
	struct NeModel *mdl = E_ResourcePtr(mr->model);

	if (!mm->vertexBuffer) {
		struct NeBufferCreateInfo bci =
		{
			.desc =
			{
				.size = mdl->cpu.vertexSize * RE_NUM_FRAMES,
				.usage = BU_VERTEX_BUFFER | BU_STORAGE_BUFFER | BU_TRANSFER_DST,
				.memoryType = MT_GPU_LOCAL
			}
		};
		if (!Re_CreateBuffer(&bci, &mm->vertexBuffer))
			return;
	}

	struct NeVertex *src = mdl->cpu.vertices;
	struct NeVertex *dst = Sys_Alloc(mdl->cpu.vertexSize, 1, MH_Transient);
	if (!dst)
		return;
	
	size_t vtxCount = mdl->cpu.vertexSize / sizeof(*src);
	
	const struct MorphInfo *mi = NULL;
	Rt_ArrayForEach(mi, &mm->activeMorphs) {
		const struct NeMorph *m = NULL;
		for (uint32_t i = 0; i < mdl->morph.count; ++i) {
			if (mdl->morph.info[i].hash != mi->name)
				continue;
			
			m = &mdl->morph.info[i];
			break;
		}
		
		if (!m)
			continue;

		const struct NeMorphDelta *deltas = mdl->morph.deltas + m->deltaOffset;
		for (uint32_t i = 0; i < m->deltaCount; ++i) {
			if (deltas[i].vertex > vtxCount)
				continue;
			
			const struct NeVertex *sVtx = &src[deltas[i].vertex];
			struct NeVertex *dVtx = &dst[deltas[i].vertex];
			
			dVtx->x = sVtx->x + mi->value * (deltas[i].x - sVtx->x);
			dVtx->y = sVtx->y + mi->value * (deltas[i].y - sVtx->y);
			dVtx->z = sVtx->z + mi->value * (deltas[i].z - sVtx->z);
			
			dVtx->nx = sVtx->nx + mi->value * (deltas[i].nx - sVtx->nx);
			dVtx->ny = sVtx->ny + mi->value * (deltas[i].ny - sVtx->ny);
			dVtx->nz = sVtx->nz + mi->value * (deltas[i].nz - sVtx->nz);
			
			dVtx->tx = sVtx->tx + mi->value * (deltas[i].tx - sVtx->tx);
			dVtx->ty = sVtx->ty + mi->value * (deltas[i].ty - sVtx->ty);
			dVtx->tz = sVtx->tz + mi->value * (deltas[i].tz - sVtx->tz);
		}
	}

	Re_BeginTransferCommandBuffer();
	Re_CmdUpdateBuffer(mm->vertexBuffer, Re_frameId * mdl->cpu.vertexSize, dst, mdl->cpu.vertexSize);
	Re_QueueTransfer(Re_EndCommandBuffer(), NULL, NULL);
	
	mr->vertexBuffer = mm->vertexBuffer;
}

/* NekoEngine
 *
 * MeshMorph.c
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
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
