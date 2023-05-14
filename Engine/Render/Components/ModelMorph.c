#include <stdlib.h>

#include <System/Memory.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Components/ModelMorph.h>
#include <Render/Components/ModelRender.h>
#include <Engine/Resource.h>
#include <Engine/ECSystem.h>
#include <Scene/Components.h>
#include <Engine/Entity.h>
#include <Engine/Asset.h>
#include <System/Log.h>

#define MMORPH_MOD	"ModelMorph"

struct MorphInfo
{
	uint64_t name;
	float value;
};

static bool InitModelMorph(struct NeModelMorph *mm, const void **args);
static void TermModelMorph(struct NeModelMorph *mm);

NE_REGISTER_COMPONENT(NE_MODEL_MORPH, struct NeModelMorph, 1, InitModelMorph, NULL, TermModelMorph)

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
InitModelMorph(struct NeModelMorph *mm, const void **args)
{
	if (!Rt_InitArray(&mm->activeMorphs, 10, sizeof(struct MorphInfo), MH_Render))
		return false;

	if (!Rt_InitArray(&mm->morphPacks, 1, sizeof(NeHandle), MH_Render))
		return false;

	struct NeModelRender *mr = E_GetComponent(mm->_owner, NE_MODEL_RENDER_ID);
	struct NeModel *mdl = E_ResourcePtr(mr->model);

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "MorphPack", len)) {
			NeHandle res = E_LoadResource(*(++args), RES_MORPH_PACK);
			if (res != NE_INVALID_HANDLE)
				Rt_ArrayAdd(&mm->morphPacks, &res);
		} else {
			struct MorphInfo mi =
			{
				.name = Rt_HashString(arg),
				.value = (float)atof(*(++args))
			};
			Rt_ArrayAdd(&mm->activeMorphs, &mi);
		}
	}

	struct NeBufferCreateInfo bci =	{ .desc = { .size = mdl->cpu.vertexSize * RE_NUM_FRAMES } };
	if (mdl->cpu.vertexSize > UINT16_MAX) {
		bci.desc.usage = BU_TRANSFER_DST | BU_TRANSFER_SRC,
		bci.desc.memoryType = MT_CPU_COHERENT;
	} else {
		bci.desc.usage = BU_VERTEX_BUFFER | BU_STORAGE_BUFFER | BU_TRANSFER_DST,
		bci.desc.memoryType = MT_GPU_LOCAL;
	}

	if (!Re_CreateBuffer(&bci, &mm->buffer))
		return false;

	if (bci.desc.memoryType == MT_CPU_COHERENT)
		mm->ptr = Re_MapBuffer(mm->buffer);

	return true;
}

static void
TermModelMorph(struct NeModelMorph *mm)
{
	Rt_TermArray(&mm->activeMorphs);

	NeHandle *mp;
	Rt_ArrayForEach(mp, &mm->morphPacks)
		E_UnloadResource(*mp);

	if (mm->ptr)
		Re_UnmapBuffer(mm->buffer);

	if (mm->vertexBufferId)
		Re_ReleaseBufferId(mm->vertexBufferId);

	if (mm->buffer)
		Re_Destroy(mm->buffer);
}

NE_SYSTEM(RE_MORPH_MODELS, ECSYS_GROUP_PRE_RENDER, ECSYS_PRI_MORPH, true, void, 2, NE_MODEL_RENDER, NE_MODEL_MORPH)
{
	struct NeModelRender *mr = comp[0];
	struct NeModelMorph *mm = comp[1];

	struct NeModel *mdl = E_ResourcePtr(mr->model);

	struct NeVertex *src = mdl->cpu.vertices;
	size_t vtxCount = mdl->cpu.vertexSize / sizeof(*src);

	struct NeVertex *dst = mdl->cpu.vertexSize > UINT16_MAX ?
		mm->ptr + Re_frameId * vtxCount
	:
		Sys_Alloc(mdl->cpu.vertexSize, 1, MH_Transient);

	if (!dst)
		return;

	memcpy(dst, src, mdl->cpu.vertexSize);

	const struct MorphInfo *mi = NULL;
	Rt_ArrayForEach(mi, &mm->activeMorphs) {
		const struct NeMorph *m = NULL;
		const struct NeMorphDelta *deltas = NULL;

		NeHandle *res;
		Rt_ArrayForEach(res, &mm->morphPacks) {
			const struct NeMorphPack *mp = E_ResourcePtr(*res);

			for (uint32_t i = 0; i < mp->count; ++i) {
				if (mp->morphs[i].hash != mi->name)
					continue;

				m = &mp->morphs[i];
				deltas = mp->deltas + m->deltaOffset;
				break;
			}

			if (m)
				break;
		}

		if (!m) {
			for (uint32_t i = 0; i < mdl->morph.count; ++i) {
				if (mdl->morph.info[i].hash != mi->name)
					continue;

				m = &mdl->morph.info[i];
				deltas = mdl->morph.deltas + m->deltaOffset;
				break;
			}

			if (!m)
				continue;
		}

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

	Re_BeginTransferCommandBuffer(NULL);

	if (mdl->cpu.vertexSize > UINT16_MAX) {
		Re_ReserveBufferId(&mm->vertexBufferId);
		Re_Destroy(mm->vertexBufferId);

		struct NeBufferDesc desc =
		{
			.size = mdl->cpu.vertexSize,
			.usage = BU_TRANSFER_DST | BU_VERTEX_BUFFER | BU_STORAGE_BUFFER,
			.memoryType = MT_GPU_LOCAL,
			.name = "Morph Vertex Buffer"
		};
		Re_CreateTransientBuffer(&desc, mm->vertexBufferId);

		Re_CmdCopyBuffer(mm->buffer, Re_frameId * mdl->cpu.vertexSize, mm->vertexBufferId, 0, mdl->cpu.vertexSize);
		mr->vertexBuffer = mm->vertexBufferId;
	} else {
		Re_CmdUpdateBuffer(mm->buffer, Re_frameId * mdl->cpu.vertexSize, dst, mdl->cpu.vertexSize);
		mr->vertexBuffer = mm->buffer;
	}

	Re_QueueTransfer(Re_EndCommandBuffer(), NULL);
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
