#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <Engine/Config.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>

struct NeLightCulling
{
	uint32_t tileSize;
	struct NePipeline *pipeline;
	uint64_t sceneDataHash, visibleLightIndicesHash, passSemaphoreHash;
};

static bool _Init(struct NeLightCulling **pass);
static void _Term(struct NeLightCulling *pass);
static bool _Setup(struct NeLightCulling *pass, struct NeArray *resources);
static void _Execute(struct NeLightCulling *pass, const struct NeArray *resources);

struct NeRenderPass RP_lightCulling =
{
	.Init = (NePassInitProc)_Init,
	.Term = (NePassTermProc)_Term,
	.Setup = (NePassSetupProc)_Setup,
	.Execute = (NePassExecuteProc)_Execute
};

struct Constants
{
	uint64_t sceneAddress;
	uint64_t visibleLightIndicesAddress;
	struct mat4 view;
};

static bool
_Setup(struct NeLightCulling *pass, struct NeArray *resources)
{
	struct NeBufferDesc bd =
	{
		.size = 100,
		.usage = BU_STORAGE_BUFFER,
		.memoryType = MT_GPU_LOCAL
	};
	return Re_AddGraphBuffer("Re_visibleLightIndices", &bd, resources);
}

static void
_Execute(struct NeLightCulling *pass, const struct NeArray *resources)
{
	struct Constants constants;
	struct NeBuffer *visibleIndicesPtr;

	constants.sceneAddress = Re_GraphBuffer(pass->sceneDataHash, resources, NULL);
	constants.visibleLightIndicesAddress = Re_GraphBuffer(pass->visibleLightIndicesHash, resources, &visibleIndicesPtr);
	m4_copy(&constants.view, &Scn_activeCamera->viewMatrix);

	Re_BeginComputeCommandBuffer();

	Re_CmdBindPipeline(pass->pipeline);
	Re_CmdPushConstants(SS_COMPUTE, sizeof(constants), &constants);

	uint32_t x = (*E_screenWidth + (*E_screenWidth % pass->tileSize)) / pass->tileSize;
	uint32_t y = (*E_screenHeight + (*E_screenHeight % pass->tileSize)) / pass->tileSize;

/*	struct NeBufferBarrier barrier1 =
	{
		.srcQueue = RE_QUEUE_GRAPHICS,
		.dstQueue = RE_QUEUE_COMPUTE,
		.srcAccess = RE_PA_COLOR_ATTACHMENT_WRITE,
		.dstAccess = RE_PA_MEMORY_READ,
		.buffer = visibleIndicesPtr,
		.offset = 0,
		.size = 100
	};
	Re_Barrier(RE_PS_FRAGMENT_SHADER, RE_PS_COMPUTE_SHADER, RE_PD_BY_REGION, 0, NULL, 1, &barrier1, 0, NULL);*/

	Re_CmdDispatch(x, ++y, 1);

/*	struct NeBufferBarrier barrier2 =
	{
		.srcQueue = RE_QUEUE_COMPUTE,
		.dstQueue = RE_QUEUE_GRAPHICS,
		.srcAccess = RE_PA_SHADER_WRITE,
		.dstAccess = RE_PA_MEMORY_READ,
		.buffer = visibleIndicesPtr,
		.offset = 0,
		.size = 100
	};
	Re_Barrier(RE_PS_COMPUTE_SHADER, RE_PS_FRAGMENT_SHADER, RE_PD_BY_REGION, 0, NULL, 1, &barrier2, 0, NULL);*/

	struct NeSemaphore *passSemaphore = Re_GraphData(pass->passSemaphoreHash, resources);
	Re_QueueCompute(Re_EndCommandBuffer(), passSemaphore, passSemaphore);
}

static bool
_Init(struct NeLightCulling **pass)
{
	*pass = Sys_Alloc(sizeof(struct NeLightCulling), 1, MH_Render);
	if (!*pass)
		return false;

	(*pass)->tileSize = E_GetCVarU32("Render_LightCullingTileSize", 16)->u32;

	struct NeComputePipelineDesc desc = {
		.stageInfo = (struct NeShaderStageInfo *)&Re_GetShader("LightCulling")->stageCount,
		.threadsPerThreadgroup = { (*pass)->tileSize, (*pass)->tileSize, 1 },
		.pushConstantSize = sizeof(struct Constants)
	};
	
	(*pass)->pipeline = Re_ComputePipeline(&desc);
	if (!(*pass)->pipeline)
		return false;

	(*pass)->sceneDataHash = Rt_HashString("Scn_data");
	(*pass)->visibleLightIndicesHash = Rt_HashString("Re_visibleLightIndices");
	(*pass)->passSemaphoreHash = Rt_HashString("Re_passSemaphore");

	return true;
}

static void
_Term(struct NeLightCulling *pass)
{
	Sys_Free(pass);
}
