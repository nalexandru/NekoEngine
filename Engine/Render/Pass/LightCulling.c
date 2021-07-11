#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>

struct LightCulling
{
	struct Pipeline *pipeline;
	BufferHandle visibleIndices;
};

static bool _Init(struct LightCulling **pass);
static void _Term(struct LightCulling *pass);
static bool _Setup(struct LightCulling *pass, struct Array *resources);
static void _Execute(struct LightCulling *pass, const struct Array *resources);

struct RenderPass RP_lightCulling =
{
	.Init = (PassInitProc)_Init,
	.Term = (PassTermProc)_Term,
	.Setup = (PassSetupProc)_Setup,
	.Execute = (PassExecuteProc)_Execute
};

static bool
_Setup(struct LightCulling *pass, struct Array *resources)
{
//	Re_CreatePassBuffer(res, "", &bci)
	return false;
}

static void
_Execute(struct LightCulling *pass, const struct Array *resources)
{
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
_Init(struct LightCulling **pass)
{
	/*if (!Re_ReserveBufferId(&pass->visibleIndices))
		return false;

	struct ComputePipelineDesc desc = {
		.shader = Re_GetShader("LightCulling"),
		.threadsPerThreadgroup = { 16, 16, 1 }
	};
	
	pass->pipeline = Re_ComputePipeline(&desc);
	return pass->pipeline != NULL;*/
	return false;
}

static void
_Term(struct LightCulling *pass)
{
	//Re_ReleaseBufferId(pass->visibleIndices);
}
