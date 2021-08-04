#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>
#include <UI/UI.h>

#include "Internal.h"

struct UIPass;
static bool _Init(struct UIPass **pass);
static void _Term(struct UIPass *pass);
static bool _Setup(struct UIPass *pass, struct Array *resources);
static void _Execute(struct UIPass *pass, const struct Array *resources);

struct RenderPass RP_ui =
{
	.Init = (PassInitProc)_Init,
	.Term = (PassTermProc)_Term,
	.Setup = (PassSetupProc)_Setup,
	.Execute = (PassExecuteProc)_Execute
};

struct UIConstants
{
	uint64_t vertexAddress;
	uint32_t texture;
	uint32_t __padding;
	struct mat4 mvp;
};

struct UIPass
{
	struct Framebuffer *fb;
	struct Pipeline *pipeline;
	struct RenderPassDesc *rpd;
	uint64_t outputHash;
	volatile bool updated;
	struct UIConstants constants;
};

static void _UIUpdateJob(int i, struct UIPass *pass);

static bool
_Setup(struct UIPass *pass, struct Array *resources)
{
	struct FramebufferAttachmentDesc atDesc;
	Re_SwapchainDesc(Re_swapchain, &atDesc);

	struct FramebufferDesc fbDesc =
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
	E_ExecuteJob((JobProc)_UIUpdateJob, pass, NULL);

	m4_ortho(&pass->constants.mvp, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 0.f, 1.f);

	return true;
}

static void
_Execute(struct UIPass *pass, const struct Array *resources)
{
	Re_SetAttachment(pass->fb, 0, Re_GraphTexture(pass->outputHash, resources));

	while (!pass->updated) ;

	Re_BeginDrawCommandBuffer();
	Re_CmdBeginRenderPass(pass->rpd, pass->fb, RENDER_COMMANDS_INLINE);

	Re_CmdBindPipeline(pass->pipeline);
	Re_CmdSetViewport(0.f, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, *E_screenWidth, *E_screenHeight);
	Re_CmdBindIndexBuffer(UI_indexBuffer, UI_indexBufferSize * Re_frameId, IT_UINT_16);

	pass->constants.vertexAddress = Re_BufferAddress(UI_vertexBuffer, UI_vertexBufferSize * Re_frameId);
	pass->constants.texture = 0;
	E_ExecuteSystemS(Scn_activeScene, UI_DRAW_CONTEXT, &pass->constants);

	Re_CmdEndRenderPass();
	Re_EndCommandBuffer();
}

static bool
_Init(struct UIPass **pass)
{
	*pass = Sys_Alloc(sizeof(struct UIPass), 1, MH_Render);
	if (!*pass)
		return false;

	struct Shader *shader = Re_GetShader("UI");

	struct AttachmentDesc atDesc =
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

	struct BlendAttachmentDesc blendAttachments[] =
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
	struct GraphicsPipelineDesc pipeDesc =
	{
		.flags = RE_TOPOLOGY_TRIANGLES | RE_POLYGON_FILL | RE_CULL_NONE | RE_FRONT_FACE_CW,
		.shader = shader,
		.renderPassDesc = (*pass)->rpd,
		.pushConstantSize = sizeof(struct UIConstants),
		.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
		.attachments = blendAttachments
	};
	(*pass)->pipeline = Re_GraphicsPipeline(&pipeDesc);
	if (!(*pass)->pipeline)
		goto error;

	(*pass)->outputHash = Rt_HashString("Re_output");

	return true;

error:
	if ((*pass)->rpd)
		Re_DestroyRenderPassDesc((*pass)->rpd);

	Sys_Free(*pass);

	return false;
}

static void
_Term(struct UIPass *pass)
{
	Re_DestroyRenderPassDesc(pass->rpd);
	Sys_Free(pass);
}

static void _UIUpdateJob(int i, struct UIPass *pass)
{
	UI_Update(Scn_activeScene);
	pass->updated = true;
}
