#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>

struct ForwardPass;
static bool _Init(struct ForwardPass **pass);
static void _Term(struct ForwardPass *pass);
static bool _Setup(struct ForwardPass *pass, struct Array *resources);
static void _Execute(struct ForwardPass *pass, const struct Array *resources);

struct RenderPass RP_forward =
{
	.Init = (PassInitProc)_Init,
	.Term = (PassTermProc)_Term,
	.Setup = (PassSetupProc)_Setup,
	.Execute = (PassExecuteProc)_Execute
};

struct ForwardPass
{
	struct Framebuffer *fb;
	uint64_t outputHash, depthHash, normalHash, sceneDataHash, instancesHash;
};

static bool
_Setup(struct ForwardPass *pass, struct Array *resources)
{
//	if (!Re_GraphTexture(pass->depthHash, resources))
//		return false;

	struct FramebufferAttachmentDesc fbAtDesc[3] =
	{
		{ .usage = 0, .format = 0 },
		{ .usage = TU_DEPTH_STENCIL_ATTACHMENT, .format = TF_D32_SFLOAT },
		{ .usage = TU_COLOR_ATTACHMENT | TU_INPUT_ATTACHMENT, .format = TF_R16G16B16A16_SFLOAT }
	};
	Re_SwapchainDesc(Re_swapchain, &fbAtDesc[0]);
	
	struct FramebufferDesc fbDesc =
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
_Execute(struct ForwardPass *pass, const struct Array *resources)
{
	struct MaterialRenderConstants constants;

	constants.sceneAddress = Re_GraphBuffer(pass->sceneDataHash, resources);
	uint64_t instanceRoot = Re_GraphBuffer(pass->instancesHash, resources);

	Re_SetAttachment(pass->fb, 0, Re_GraphTexture(pass->outputHash, resources));
	Re_SetAttachment(pass->fb, 1, Re_GraphTexture(pass->depthHash, resources));
	Re_SetAttachment(pass->fb, 2, Re_GraphTexture(pass->normalHash, resources));

	Re_BeginDrawCommandBuffer();
	Re_CmdBeginRenderPass(Re_MaterialRenderPassDesc, pass->fb, RENDER_COMMANDS_INLINE);

	Re_CmdSetViewport(0.f, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, *E_screenWidth, *E_screenHeight);

	uint32_t instance = 0;
	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
		struct Array *drawables = &Scn_activeScene->collect.arrays[i];
		struct Drawable *d = NULL;

		if (!drawables->count)
			continue;

		Rt_ArrayForEach(d, drawables) {
			Re_CmdBindPipeline(d->material->pipeline);
			Re_CmdBindIndexBuffer(d->indexBuffer, 0, d->indexType);
			
			constants.vertexAddress = d->vertexAddress;
			constants.materialAddress = d->materialAddress;
			constants.instanceAddress = Re_OffsetAddress(instanceRoot, (instance++ * sizeof(struct ModelInstance)));

			Re_CmdPushConstants(SS_ALL, sizeof(constants), &constants);
			Re_CmdDrawIndexed(d->indexCount, 1, d->firstIndex, 0, 0);
		}
	}

	Re_CmdEndRenderPass();
	Re_EndCommandBuffer();
}

static bool
_Init(struct ForwardPass **pass)
{
	*pass = Sys_Alloc(sizeof(struct ForwardPass), 1, MH_Render);
	if (!*pass)
		return false;

	(*pass)->outputHash = Rt_HashString("Re_output");
	(*pass)->depthHash = Rt_HashString("Re_depthBuffer");
	(*pass)->normalHash = Rt_HashString("Re_normalBuffer");
	(*pass)->sceneDataHash = Rt_HashString("Scn_data");
	(*pass)->instancesHash = Rt_HashString("Scn_instances");

	return true;
}

static void
_Term(struct ForwardPass *pass)
{
	Sys_Free(pass);
}
