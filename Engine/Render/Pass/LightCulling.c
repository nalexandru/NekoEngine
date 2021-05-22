#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>

struct LightCulling 
{
	struct Pipeline *pipeline;
};

static void
_Setup(struct LightCulling *pass)
{
}

static void
_Execute(struct LightCulling *pass, void *resources)
{
	Re_BeginComputeCommandBuffer();

	Re_CmdDispatch(0, 0, 0);

	Re_EndCommandBuffer();
}

static bool
_Init(struct LightCulling *pass)
{
	struct Shader *shader = Re_GetShader("LightCulling");

	pass->pipeline = Re_ComputePipeline(shader);
}

static void
_Term(struct LightCulling *pass)
{
}
