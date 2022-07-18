#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>
#include <UI/UI.h>

#include "Internal.h"

struct NeUIPass;
static bool _Init(struct NeUIPass **pass);
static void _Term(struct NeUIPass *pass);
static bool _Setup(struct NeUIPass *pass, struct NeArray *resources);
static void _Execute(struct NeUIPass *pass, const struct NeArray *resources);

struct NeRenderPass RP_ui =
{
	.Init = (NePassInitProc)_Init,
	.Term = (NePassTermProc)_Term,
	.Setup = (NePassSetupProc)_Setup,
	.Execute = (NePassExecuteProc)_Execute
};

struct NeUIConstants
{
	uint64_t vertexAddress;
	uint32_t texture;
	uint32_t __padding;
	struct NeMatrix mvp;
};

struct NeUIPass
{
	struct NeFramebuffer *fb;
	struct NePipeline *pipeline;
	struct NeRenderPassDesc *rpd;
	uint64_t outputHash, passSemaphoreHash;
	volatile bool updated;
	struct NeUIConstants constants;
};

static void _UIUpdateJob(int i, struct NeUIPass *pass);

static bool
_Setup(struct NeUIPass *pass, struct NeArray *resources)
{
	struct NeFramebufferAttachmentDesc atDesc;
	Re_SwapchainDesc(Re_swapchain, &atDesc);

	struct NeFramebufferDesc fbDesc =
	{
		.attachmentCount = 1,
		.attachments = &atDesc,
		.width = *E_screenWidth,
		.height = *E_screenHeight,
		.layers = 1,
		.renderPassDesc = pass->rpd 
	};
	pass->fb = Re_CreateFramebuffer(&fbDesc);
	Re_Destroy(pass->fb);

	pass->updated = false;
	E_ExecuteJob((NeJobProc)_UIUpdateJob, pass, NULL, NULL);

	M_OrthographicMatrix(&pass->constants.mvp, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 0.f, 1.f);

	return true;
}

static void
_Execute(struct NeUIPass *pass, const struct NeArray *resources)
{
	Re_SetAttachment(pass->fb, 0, Re_GraphTexture(pass->outputHash, resources));

	while (!pass->updated);

	Re_BeginDrawCommandBuffer();
	Re_CmdBeginRenderPass(pass->rpd, pass->fb, RENDER_COMMANDS_INLINE);

	Re_CmdBindPipeline(pass->pipeline);
	Re_CmdSetViewport(0.f, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, *E_screenWidth, *E_screenHeight);
	Re_CmdBindIndexBuffer(UI_indexBuffer, UI_indexBufferSize * Re_frameId, IT_UINT_16);

	pass->constants.vertexAddress = Re_BufferAddress(UI_vertexBuffer, UI_vertexBufferSize * Re_frameId);
	pass->constants.texture = 0;
	E_ExecuteSystemS(Scn_activeScene, UI_DRAW_CONTEXT, &pass->constants);

	struct NeUIContext *ctx;
	Rt_ArrayForEachPtr(ctx, &UI_standaloneContexts)
		_UI_DrawContext((void **)&ctx, &pass->constants);

	Re_CmdEndRenderPass();

	struct NeSemaphore *passSemaphore = Re_GraphData(pass->passSemaphoreHash, resources);
	Re_QueueGraphics(Re_EndCommandBuffer(), passSemaphore, NULL);
}

static bool
_Init(struct NeUIPass **pass)
{
	*pass = Sys_Alloc(sizeof(struct NeUIPass), 1, MH_Render);
	if (!*pass)
		return false;

	struct NeShader *shader = Re_GetShader("UI");

	struct NeAttachmentDesc atDesc =
	{
		.mayAlias = false,
		.format = Re_SwapchainFormat(Re_swapchain),
		.loadOp = ATL_LOAD,
		.storeOp = ATS_STORE,
		.samples = ASC_1_SAMPLE,
		.initialLayout = TL_PRESENT_SRC,
		.layout = TL_COLOR_ATTACHMENT,
		.finalLayout = TL_PRESENT_SRC,
		.clearColor = { .3f, .0f, .4f, 1.f }
	};
	(*pass)->rpd = Re_CreateRenderPassDesc(&atDesc, 1, NULL, NULL, 0);
	if (!(*pass)->rpd)
		goto error;

	struct NeBlendAttachmentDesc blendAttachments[] =
	{
		{
			.enableBlend = true,
			.writeMask = RE_WRITE_MASK_RGBA,
			.srcColor = RE_BF_SRC_ALPHA,
			.dstColor = RE_BF_ONE_MINUS_SRC_ALPHA,
			.colorOp = RE_BOP_ADD,
			.srcAlpha = RE_BF_ONE,
			.dstAlpha = RE_BF_ZERO,
			.alphaOp = RE_BOP_ADD
		}
	};
	struct NeGraphicsPipelineDesc pipeDesc =
	{
		.flags = RE_TOPOLOGY_TRIANGLES | RE_POLYGON_FILL | RE_CULL_NONE | RE_FRONT_FACE_CW,
		.stageInfo = &shader->opaqueStages,
		.renderPassDesc = (*pass)->rpd,
		.pushConstantSize = sizeof(struct NeUIConstants),
		.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
		.attachments = blendAttachments
	};
	(*pass)->pipeline = Re_GraphicsPipeline(&pipeDesc);
	if (!(*pass)->pipeline)
		goto error;

	(*pass)->outputHash = Rt_HashString(RE_OUTPUT);
	(*pass)->passSemaphoreHash = Rt_HashString(RE_PASS_SEMAPHORE);

	return true;

error:
	if ((*pass)->rpd)
		Re_DestroyRenderPassDesc((*pass)->rpd);

	Sys_Free(*pass);

	return false;
}

static void
_Term(struct NeUIPass *pass)
{
	Re_DestroyRenderPassDesc(pass->rpd);
	Sys_Free(pass);
}

static void _UIUpdateJob(int i, struct NeUIPass *pass)
{
	UI_Update(Scn_activeScene);
	pass->updated = true;
}
