#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>

struct NeOpaquePass;
static bool _Init(struct NeOpaquePass **pass);
static void _Term(struct NeOpaquePass *pass);
static bool _Setup(struct NeOpaquePass *pass, struct NeArray *resources);
static void _Execute(struct NeOpaquePass *pass, const struct NeArray *resources);

struct NeRenderPass RP_opaque =
{
	.Init = (NePassInitProc)_Init,
	.Term = (NePassTermProc)_Term,
	.Setup = (NePassSetupProc)_Setup,
	.Execute = (NePassExecuteProc)_Execute
};

struct NeOpaquePass
{
	struct NeFramebuffer *fb;
	uint64_t outputHash, depthHash, normalHash, sceneDataHash, instancesHash, passSemaphoreHash, visibleLightIndicesHash;
};

static bool
_Setup(struct NeOpaquePass *pass, struct NeArray *resources)
{
//	if (!Re_GraphTexture(pass->depthHash, resources))
//		return false;

	struct NeFramebufferAttachmentDesc fbAtDesc[3] =
	{
		{ .usage = 0, .format = 0 },
		{ .usage = TU_DEPTH_STENCIL_ATTACHMENT | TU_SAMPLED, .format = TF_D32_SFLOAT },
		{ .usage = TU_COLOR_ATTACHMENT | TU_INPUT_ATTACHMENT, .format = TF_R16G16B16A16_SFLOAT }
	};
	Re_SwapchainDesc(Re_swapchain, &fbAtDesc[0]);
	
	struct NeFramebufferDesc fbDesc =
	{
		.attachmentCount = 3,
		.attachments = fbAtDesc,
		.width = *E_screenWidth,
		.height = *E_screenHeight,
		.layers = 1,
		.renderPassDesc = Re_MaterialRenderPassDesc
	};
	pass->fb = Re_CreateFramebuffer(&fbDesc);
	Re_Destroy(pass->fb);

	return true;
}

static void
_Execute(struct NeOpaquePass *pass, const struct NeArray *resources)
{
	struct NeMaterialRenderConstants constants;

	struct NeBuffer *visibleIndices;
	constants.sceneAddress = Re_GraphBuffer(pass->sceneDataHash, resources, NULL);
	constants.visibleIndicesAddress = Re_GraphBuffer(pass->visibleLightIndicesHash, resources, &visibleIndices);
	uint64_t instanceRoot = Re_GraphBuffer(pass->instancesHash, resources, NULL);

	struct NeTexture *depthTexture = Re_GraphTexture(pass->depthHash, resources);
	struct NeTexture *normalTexture = Re_GraphTexture(pass->normalHash, resources);
	Re_SetAttachment(pass->fb, 0, Re_GraphTexture(pass->outputHash, resources));
	Re_SetAttachment(pass->fb, 1, depthTexture);
	Re_SetAttachment(pass->fb, 2, normalTexture);

	Re_BeginDrawCommandBuffer();

	struct NeBufferBarrier visIdxBarrier =
	{
		.srcQueue = RE_QUEUE_COMPUTE,
		.dstQueue = RE_QUEUE_GRAPHICS,
		.dstStage = RE_PS_INDEX_INPUT,
		.dstAccess = RE_PA_MEMORY_READ,
		.buffer = visibleIndices,
		.size = RE_WHOLE_SIZE
	};
	struct NeImageBarrier imgBarriers[] =
	{
		{ // Depth
				.srcStage = RE_PS_EARLY_FRAGMENT_TESTS | RE_PS_LATE_FRAGMENT_TESTS,
				.dstStage = RE_PS_EARLY_FRAGMENT_TESTS | RE_PS_LATE_FRAGMENT_TESTS,
				.srcAccess = RE_PA_DEPTH_STENCIL_ATTACHMENT_WRITE,
				.dstAccess = RE_PA_DEPTH_STENCIL_ATTACHMENT_READ,
				.srcQueue = RE_QUEUE_GRAPHICS,
				.dstQueue = RE_QUEUE_GRAPHICS,
				.oldLayout = TL_DEPTH_ATTACHMENT,
				.newLayout = TL_DEPTH_ATTACHMENT,
				.texture = depthTexture,
				.subresource.aspect = IA_DEPTH,
				.subresource.baseArrayLayer = 0,
				.subresource.layerCount = 1,
				.subresource.mipLevel = 0,
				.subresource.levelCount = 1
		},
		{ // Normal
				.srcStage = RE_PS_COLOR_ATTACHMENT_OUTPUT,
				.srcAccess = RE_PA_COLOR_ATTACHMENT_WRITE,
				.dstStage = RE_PS_FRAGMENT_SHADER,
				.dstAccess = RE_PA_SHADER_READ,
				.srcQueue = RE_QUEUE_GRAPHICS,
				.dstQueue = RE_QUEUE_GRAPHICS,
				.oldLayout = TL_SHADER_READ_ONLY,
				.newLayout = TL_SHADER_READ_ONLY,
				.texture = normalTexture,
				.subresource.aspect = IA_COLOR,
				.subresource.baseArrayLayer = 0,
				.subresource.layerCount = 1,
				.subresource.mipLevel = 0,
				.subresource.levelCount = 1
		}
	};
	Re_Barrier(RE_PD_BY_REGION, 0, NULL, 1, &visIdxBarrier, 2, imgBarriers);

	Re_CmdBeginRenderPass(Re_MaterialRenderPassDesc, pass->fb, RENDER_COMMANDS_INLINE);

	Re_CmdSetViewport(0.f, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, *E_screenWidth, *E_screenHeight);

	const struct NeDrawable *d = NULL;
	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
		const uint32_t instanceOffset = Scn_activeScene->collect.instanceOffset[i];
		const struct NeArray *drawables = &Scn_activeScene->collect.opaqueDrawableArrays[i];

		if (!drawables->count)
			continue;

		Rt_ArrayForEach(d, drawables) {
			Re_CmdBindPipeline(d->material->pipeline);
			Re_CmdBindVertexBuffer(d->vertexBuffer, d->vertexOffset);
			Re_CmdBindIndexBuffer(d->indexBuffer, 0, d->indexType);
			
			constants.vertexAddress = d->vertexAddress;
			constants.materialAddress = d->materialAddress;
			constants.instanceAddress = Re_OffsetAddress(instanceRoot, (((uint64_t)d->instanceId + instanceOffset) * sizeof(struct NeModelInstance)));

			Re_CmdPushConstants(SS_ALL, sizeof(constants), &constants);
			Re_CmdDrawIndexed(d->indexCount, 1, d->firstIndex, 0, 0);
		}
	}

	Re_CmdEndRenderPass();

	struct NeSemaphore *passSemaphore = Re_GraphData(pass->passSemaphoreHash, resources);
	Re_QueueGraphics(Re_EndCommandBuffer(), passSemaphore, passSemaphore);
}

static bool
_Init(struct NeOpaquePass **pass)
{
	*pass = Sys_Alloc(sizeof(struct NeOpaquePass), 1, MH_Render);
	if (!*pass)
		return false;

	(*pass)->outputHash = Rt_HashString(RE_OUTPUT);
	(*pass)->depthHash = Rt_HashString(RE_DEPTH_BUFFER);
	(*pass)->normalHash = Rt_HashString(RE_NORMAL_BUFFER);
	(*pass)->sceneDataHash = Rt_HashString(RE_SCENE_DATA);
	(*pass)->instancesHash = Rt_HashString(RE_SCENE_INSTANCES);
	(*pass)->passSemaphoreHash = Rt_HashString(RE_PASS_SEMAPHORE);
	(*pass)->visibleLightIndicesHash = Rt_HashString(RE_VISIBLE_LIGHT_INDICES);

	return true;
}

static void
_Term(struct NeOpaquePass *pass)
{
	Sys_Free(pass);
}
