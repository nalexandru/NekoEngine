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
	uint32_t *tileSize, xTiles, yTiles;
	struct NePipeline *pipeline;
	uint64_t depthHash, sceneDataHash, visibleLightIndicesHash, passSemaphoreHash;
	uint64_t bufferSize;
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
	struct NeMatrix view;
	uint32_t depthMap;
	uint32_t threadCount;
};

static bool
_Setup(struct NeLightCulling *pass, struct NeArray *resources)
{
	pass->xTiles = (*E_screenWidth + (*E_screenWidth % *pass->tileSize)) / *pass->tileSize;
	pass->yTiles = ((*E_screenHeight + (*E_screenHeight % *pass->tileSize)) / *pass->tileSize) + 1;
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
_Execute(struct NeLightCulling *pass, const struct NeArray *resources)
{
	struct Constants constants;
	struct NeBuffer *visibleIndices;

	constants.threadCount = *pass->tileSize * *pass->tileSize;
	constants.sceneAddress = Re_GraphBuffer(pass->sceneDataHash, resources, NULL);
	constants.visibleLightIndicesAddress = Re_GraphBuffer(pass->visibleLightIndicesHash, resources, &visibleIndices);
	constants.depthMap = Re_GraphTextureLocation(pass->depthHash, resources);

	M_Copy(&constants.view, &Scn_activeCamera->viewMatrix);

	Re_BeginComputeCommandBuffer();

	Re_CmdBindPipeline(pass->pipeline);
	Re_CmdPushConstants(SS_COMPUTE, sizeof(constants), &constants);

	Re_CmdDispatch(pass->xTiles, pass->yTiles, 1);

	struct NeBufferBarrier idxBarrier =
	{
		.srcQueue = RE_QUEUE_COMPUTE,
		.dstQueue = RE_QUEUE_GRAPHICS,
		.srcStage = RE_PS_COMPUTE_SHADER,
		//.dstStage = RE_PS_INDEX_INPUT,
		.srcAccess = RE_PA_SHADER_WRITE,
		//.dstAccess = RE_PA_MEMORY_READ,
		.buffer = visibleIndices,
		.size = RE_WHOLE_SIZE
	};
	Re_Barrier(RE_PD_BY_REGION, 0, NULL, 1, &idxBarrier, 0, NULL);

	struct NeSemaphore *passSemaphore = Re_GraphData(pass->passSemaphoreHash, resources);
	Re_QueueCompute(Re_EndCommandBuffer(), passSemaphore, passSemaphore);
}

static bool
_Init(struct NeLightCulling **pass)
{
	*pass = Sys_Alloc(sizeof(struct NeLightCulling), 1, MH_Render);
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

	(*pass)->depthHash = Rt_HashString(RE_DEPTH_BUFFER);
	(*pass)->sceneDataHash = Rt_HashString(RE_SCENE_DATA);
	(*pass)->visibleLightIndicesHash = Rt_HashString(RE_VISIBLE_LIGHT_INDICES);
	(*pass)->passSemaphoreHash = Rt_HashString(RE_PASS_SEMAPHORE);

	return true;
}

static void
_Term(struct NeLightCulling *pass)
{
	Sys_Free(pass);
}
