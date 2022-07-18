#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <Engine/Engine.h>
#include <Engine/Resource.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/RayTracing.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>

struct NeAccelerationStructureBuildPass;
static bool _Init(struct NeAccelerationStructureBuildPass **pass);
static void _Term(struct NeAccelerationStructureBuildPass *pass);
static bool _Setup(struct NeAccelerationStructureBuildPass *pass, struct NeArray *resources);
static void _Execute(struct NeAccelerationStructureBuildPass *pass, const struct NeArray *resources);

struct NeRenderPass RP_accelerationStructureBuild =
{
	.Init = (NePassInitProc)_Init,
	.Term = (NePassTermProc)_Term,
	.Setup = (NePassSetupProc)_Setup,
	.Execute = (NePassExecuteProc)_Execute
};

struct NeAccelerationStructureBuildPass
{
	uint64_t passSemaphoreHash;
};

static bool
_Setup(struct NeAccelerationStructureBuildPass *pass, struct NeArray *resources)
{
	return true;
}

static void
_Execute(struct NeAccelerationStructureBuildPass *pass, const struct NeArray *resources)
{
	Re_BeginComputeCommandBuffer();

	struct NeAccelerationStructureGeometryDesc asg =
	{
		.type = ASG_INSTANCES,
		.instances.address = 0 /* instance buffer address */
	};
	struct NeAccelerationStructureBuildInfo asbi =
	{
		.type = AS_TOP_LEVEL,
		.flags = ASF_FAST_BUILD,
		.mode = ASB_BUILD,
		.dst = NULL, /* resource */
		.geometryCount = 0, /* instance count */
		.geometries = &asg,
		.scratchAddress = 0 /* scratch buffer */
	};

	Re_CmdBuildAccelerationStructures(1, &asbi, NULL);

	struct NeSemaphore *passSemaphore = Re_GraphData(pass->passSemaphoreHash, resources);
	Re_QueueCompute(Re_EndCommandBuffer(), NULL, passSemaphore);
}

static bool
_Init(struct NeAccelerationStructureBuildPass **pass)
{
	*pass = Sys_Alloc(sizeof(struct NeAccelerationStructureBuildPass), 1, MH_Render);
	if (!*pass)
		return false;

	(*pass)->passSemaphoreHash = Rt_HashString(RE_PASS_SEMAPHORE);

	return true;
}

static void
_Term(struct NeAccelerationStructureBuildPass *pass)
{
	Sys_Free(pass);
}
