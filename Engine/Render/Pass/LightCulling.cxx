#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <Engine/Config.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>

NE_RENDER_PASS(NeLightCulling,
{
	uint32_t *tileSize;
	uint32_t xTiles;
	uint32_t yTiles;
	struct NePipeline *pipeline;
	struct NeRenderPassDesc *rpd;
	uint64_t bufferSize;
});

struct Constants
{
	uint64_t sceneAddress;
	uint64_t visibleLightIndicesAddress;
	struct NeMatrix view;
	uint32_t depthMap;
	uint32_t threadCount;
};

static bool
NeLightCulling_Setup(struct NeLightCulling *pass, struct NeArray *resources)
{
	const struct NeTextureDesc *outDesc = Re_GraphTextureDesc(Rt_HashLiteral(RE_OUTPUT), resources);

	pass->xTiles = (outDesc->width + (outDesc->width % *pass->tileSize)) / *pass->tileSize;
	pass->yTiles = ((outDesc->height + (outDesc->height % *pass->tileSize)) / *pass->tileSize) + 1;
	pass->bufferSize = sizeof(int32_t) * pass->xTiles * pass->yTiles * 4096;//Scn_activeScene->lightCount;

	struct NeBufferDesc bd =
	{
		.size = pass->bufferSize,
		.usage = BU_STORAGE_BUFFER,
		.memoryType = MT_GPU_LOCAL
	};
	return Re_AddGraphBuffer(RE_VISIBLE_LIGHT_INDICES, &bd, resources);
}

static void
NeLightCulling_Execute(struct NeLightCulling *pass, const struct NeArray *resources)
{
	struct Constants constants{};
	struct NeBuffer *visibleIndices{};
	struct NeSemaphore *passSemaphore = (struct NeSemaphore *)Re_GraphData(Rt_HashLiteral(RE_PASS_SEMAPHORE), resources);

	constants.threadCount = *pass->tileSize * *pass->tileSize;
	constants.sceneAddress = Re_GraphBuffer(Rt_HashLiteral(RE_SCENE_DATA), resources, NULL);
	constants.visibleLightIndicesAddress = Re_GraphBuffer(Rt_HashLiteral(RE_VISIBLE_LIGHT_INDICES), resources, &visibleIndices);
	constants.depthMap = Re_GraphTextureLocation(Rt_HashLiteral(RE_DEPTH_BUFFER), resources);

	const struct NeCamera *cam = (const struct NeCamera *)Re_GraphData(Rt_HashLiteral(RE_CAMERA), resources);
	memcpy(&constants.view, &cam->viewMatrix, sizeof(constants.view));

	Re_BeginComputeCommandBuffer(passSemaphore);

	Re_CmdBindPipeline(pass->pipeline);
	Re_CmdPushConstants(SS_COMPUTE, sizeof(constants), &constants);

	Re_CmdDispatch(pass->xTiles, pass->yTiles, 1);

	struct NeBufferBarrier idxBarrier =
	{
		.srcStage = RE_PS_COMPUTE_SHADER,
		//.dstStage = RE_PS_INDEX_INPUT,
		.srcAccess = RE_PA_SHADER_WRITE,
		//.dstAccess = RE_PA_MEMORY_READ,
		.srcQueue = RE_QUEUE_COMPUTE,
		.dstQueue = RE_QUEUE_GRAPHICS,
		.buffer = visibleIndices,
		.size = RE_WHOLE_SIZE
	};
	Re_CmdBarrier(RE_PD_BY_REGION, 0, nullptr, 1, &idxBarrier, 0, NULL);
	Re_QueueCompute(Re_EndCommandBuffer(), passSemaphore);
}

static bool
NeLightCulling_Init(struct NeLightCulling **pass)
{
	*pass = (struct NeLightCulling *)Sys_Alloc(sizeof(struct NeLightCulling), 1, MH_Render);
	if (!*pass)
		return false;

	(*pass)->tileSize = &E_GetCVarU32(SID("Render_LightCullingTileSize"), 16)->u32;

	struct NeComputePipelineDesc desc = {
		.stageInfo = (struct NeShaderStageInfo *)&Re_GetShader("LightCulling")->stageCount,
		.threadsPerThreadgroup = { *(*pass)->tileSize, *(*pass)->tileSize, 1 },
		.pushConstantSize = sizeof(struct Constants)
	};
	
	(*pass)->pipeline = Re_ComputePipeline(&desc);
	if (!(*pass)->pipeline)
		return false;

	(*pass)->bufferSize = 1024 * 1024;

	return true;
}

static void
NeLightCulling_Term(struct NeLightCulling *pass)
{
	Sys_Free(pass);
}

/* NekoEngine
 *
 * LightCulling.cxx
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
