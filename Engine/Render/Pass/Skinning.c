#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <Scene/Components.h>
#include <Engine/Config.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>
#include <Animation/Animation.h>

struct NeSkinning
{
	NeCompTypeId typeId;
	uint64_t passSemaphoreHash;
};

static bool _Init(struct NeSkinning **pass);
static void _Term(struct NeSkinning *pass);
static bool _Setup(struct NeSkinning *pass, struct NeArray *resources);
static void _Execute(struct NeSkinning *pass, const struct NeArray *resources);

struct NeRenderPass RP_skinning =
{
	.Init = (NePassInitProc)_Init,
	.Term = (NePassTermProc)_Term,
	.Setup = (NePassSetupProc)_Setup,
	.Execute = (NePassExecuteProc)_Execute
};

static bool
_Setup(struct NeSkinning *pass, struct NeArray *resources)
{
	return E_ComponentCount(pass->typeId) > 0;
}

static void
_Execute(struct NeSkinning *pass, const struct NeArray *resources)
{
	Re_BeginDrawCommandBuffer();

	Re_CmdBindPipeline(Anim_skinningPipeline);
	E_ExecuteSystem(ANIM_COMPUTE_SKINNING, NULL);

	struct NeSemaphore *passSemaphore = Re_GraphData(pass->passSemaphoreHash, resources);
	Re_QueueGraphics(Re_EndCommandBuffer(), NULL, passSemaphore);
}

static bool
_Init(struct NeSkinning **pass)
{
	*pass = Sys_Alloc(sizeof(struct NeSkinning), 1, MH_Render);
	if (!*pass)
		return false;

	(*pass)->typeId = E_ComponentTypeId(ANIMATOR_COMP);
	(*pass)->passSemaphoreHash = Rt_HashString(RE_PASS_SEMAPHORE);

	return true;
}

static void
_Term(struct NeSkinning *pass)
{
	Sys_Free(pass);
}

/* NekoEngine
 *
 * Skinning.c
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
