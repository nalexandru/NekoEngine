#include <stdlib.h>

#include <System/Log.h>
#include <System/Memory.h>
#include <Render/Render.h>
#include <Render/Components/ModelRender.h>
#include <Scene/Components.h>
#include <Engine/Resource.h>
#include <Engine/Asset.h>

#define MR_MOD	"ModelRender"

struct MorphInfo
{
	char file[4096];
};

static bool InitModelRender(struct NeModelRender *mr, const void **args);
static void TermModelRender(struct NeModelRender *mr);

NE_REGISTER_COMPONENT(NE_MODEL_RENDER, struct NeModelRender, 1, InitModelRender, NULL, TermModelRender)

void
Re_SetModel(struct NeModelRender *mr, NeHandle model)
{
	struct NeModel *new = E_ResourcePtr(model);
	struct NeModel *old = E_ResourcePtr(mr->model);

	if (old)
		TermModelRender(mr);

	if (!new)
		return;

	mr->model = model;

	memcpy(&mr->bounds, &new->bounds, sizeof(mr->bounds));

	mr->meshBounds = Sys_ReAlloc(mr->meshBounds, new->meshCount, sizeof(*mr->meshBounds), MH_Render);
	for (uint32_t i = 0; i < new->meshCount; ++i)
		memcpy(&mr->meshBounds[i], &new->meshes[i].bounds, sizeof(mr->meshBounds[i]));

	mr->materials = Sys_ReAlloc(mr->materials, new->meshCount, sizeof(*mr->materials), MH_Render);
	for (uint32_t i = 0; i < new->meshCount; ++i)
		Re_InitMaterial(new->meshes[i].materialResource, &mr->materials[i]);

	mr->vertexBuffer = new->gpu.vertexBuffer;
	mr->meshCount = new->meshCount;
}

static bool
InitModelRender(struct NeModelRender *mr, const void **args)
{
	struct NeArray morphs = { 0 };
	NeHandle model = NE_INVALID_HANDLE;
	mr->model = NE_INVALID_HANDLE;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "Model", len)) {
			model = E_LoadResource(*(++args), RES_MODEL);
		} else if (!strncmp(arg, "__ModelHandle", len)) {
			model = (NeHandle)(*(++args));
		} else if (!strncmp(arg, "Material", len)) {
		} else if (!strncmp(arg, "Morph", len)) {
			if (!morphs.data)
				Rt_InitArray(&morphs, 10, sizeof(struct MorphInfo), MH_Asset);

			struct MorphInfo mi;
			strlcpy(mi.file, (char *)(*(++args)), sizeof(mi.file));

			Rt_ArrayAdd(&morphs, &mi);
		}
	}

	// TODO: material override

	if (model != NE_INVALID_HANDLE) {
		Re_SetModel(mr, model);

		struct NeModel *mdl = E_ResourcePtr(model);

		if (morphs.data) {
			const struct MorphInfo *mi = NULL;
			Rt_ArrayForEach(mi, &morphs) {
				struct NeStream stm = { 0 };
				if (!E_FileStream(mi->file, IO_READ, &stm)) {
					Sys_LogEntry(MR_MOD, LOG_CRITICAL, "Failed to open morph file %s", mi->file);
					continue;
				}

				if (!Asset_LoadMorphPackForModel(&stm, mdl))
					Sys_LogEntry(MR_MOD, LOG_CRITICAL, "Failed to load morph %s", mi->file);
				else
					Sys_LogEntry(MR_MOD, LOG_INFORMATION, "Loaded morph %s", mi->file);

				E_CloseStream(&stm);
			}
		}
	}

	Rt_TermArray(&morphs);

	return true;
}

static void
TermModelRender(struct NeModelRender *mr)
{
	struct NeModel *m = E_ResourcePtr(mr->model);
	if (!m)
		return;

	for (uint32_t i = 0; i < m->meshCount; ++i)
		Re_TermMaterial(&mr->materials[i]);

	E_UnloadResource(mr->model);
	Sys_Free(mr->meshBounds);
	Sys_Free(mr->materials);
}

/* NekoEngine
 *
 * ModelRender.c
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
