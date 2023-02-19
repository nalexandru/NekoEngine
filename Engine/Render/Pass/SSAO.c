#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>

struct NeSSAOPass
{
	struct NePipeline *pipeline;
	NeBufferHandle visibleIndices;
};

static bool _Init(struct NeSSAOPass **pass);
static void _Term(struct NeSSAOPass *pass);
static bool _Setup(struct NeSSAOPass *pass, struct NeArray *resources);
static void _Execute(struct NeSSAOPass *pass, const struct NeArray *resources);

struct NeRenderPass RP_SSAO =
{
	.Init = (NePassInitProc)_Init,
	.Term = (NePassTermProc)_Term,
	.Setup = (NePassSetupProc)_Setup,
	.Execute = (NePassExecuteProc)_Execute
};

static bool
_Setup(struct NeSSAOPass *pass, struct NeArray *resources)
{
	const struct NeTextureDesc *outDesc = NULL;//Re_GraphTextureDesc(pass->outputHash, resources);

	struct NeTextureDesc aoDesc =
	{
		.width = outDesc->width,
		.height = outDesc->height,
		.depth = 1,
		.type = TT_2D,
		.usage = TU_COLOR_ATTACHMENT | TU_INPUT_ATTACHMENT,
		.format = TF_R8_UNORM,
		.arrayLayers = 1,
		.mipLevels = 1,
		.gpuOptimalTiling = true,
		.memoryType = MT_GPU_LOCAL
	};
	Re_AddGraphTexture(RE_AO_BUFFER, &aoDesc, 0, resources);

	//	Re_CreatePassBuffer(res, "", &bci)
	return false;
}

static void
_Execute(struct NeSSAOPass *pass, const struct NeArray *resources)
{
	Re_BeginComputeCommandBuffer();

	Re_CmdBindPipeline(pass->pipeline);
//	Re_CmdPushConstants(SS_COMPUTE, sizeof(constants), &constants);

	Re_CmdDispatch(0, 0, 0);

	Re_QueueCompute(Re_EndCommandBuffer(), NULL, NULL);

	//Re_CreateTransientBuffer(&bci, 0, 0);
	/*struct {
		uint64_t address;
	} constants;

	constants.address = Re_BufferAddress(pass->visibleIndices, 0);

	Re_BeginComputeCommandBuffer();

	Re_CmdBindPipeline(pass->pipeline);
	Re_CmdPushConstants(SS_COMPUTE, sizeof(constants), &constants);

	Re_CmdDispatch(0, 0, 0);

	Re_EndCommandBuffer();*/
}

static bool
_Init(struct NeSSAOPass **pass)
{
	/*if (!Re_ReserveBufferId(&pass->visibleIndices))
		return false;

	struct NeComputePipelineDesc desc = {
		.shader = Re_GetShader("LightCulling"),
		.threadsPerThreadgroup = { 16, 16, 1 }
	};

	pass->pipeline = Re_ComputePipeline(&desc);
	return pass->pipeline != NULL;*/
	return false;
}

static void
_Term(struct NeSSAOPass *pass)
{
	//Re_ReleaseBufferId(pass->visibleIndices);
}

/* NekoEngine
 *
 * SSAO.c
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
