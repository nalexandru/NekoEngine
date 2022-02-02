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
	struct NeTextureDesc aoDesc =
	{
		.width = *E_screenWidth,
		.height = *E_screenHeight,
		.depth = 1,
		.type = TT_2D,
		.usage = TU_COLOR_ATTACHMENT | TU_INPUT_ATTACHMENT,
		.format = TF_R8_UNORM,
		.arrayLayers = 1,
		.mipLevels = 1,
		.gpuOptimalTiling = true,
		.memoryType = MT_GPU_LOCAL
	};
	Re_AddGraphTexture("Re_aoBuffer", &aoDesc, resources);

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
