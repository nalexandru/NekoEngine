#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>

struct DepthPrePass;
static bool _Init(struct DepthPrePass **pass);
static void _Term(struct DepthPrePass *pass);
static bool _Setup(struct DepthPrePass *pass, struct Array *resources);
static void _Execute(struct DepthPrePass *pass, const struct Array *resources);

struct RenderPass RP_depthPrePass =
{
	.Init = (PassInitProc)_Init,
	.Term = (PassTermProc)_Term,
	.Setup = (PassSetupProc)_Setup,
	.Execute = (PassExecuteProc)_Execute
};

struct DepthPrePass
{
	struct Framebuffer *fb;
	struct Pipeline *pipeline;
	struct RenderPassDesc *rpd;
	uint64_t normalHash, depthHash;
};

struct Constants
{
	uint64_t vertexAddress;
	uint64_t materialAddress;
	struct mat4 mvp;
};

static bool 
_Setup(struct DepthPrePass *pass, struct Array *resources)
{
	struct FramebufferAttachmentDesc fbAtDesc[2] =
	{
		{ .usage = TU_COLOR_ATTACHMENT, .format = TF_R16G16B16A16_SFLOAT },
		{ .usage = TU_DEPTH_STENCIL_ATTACHMENT, .format = TF_D32_SFLOAT }
	};
	struct FramebufferDesc fbDesc =
	{
		.attachmentCount = 2,
		.attachments = fbAtDesc,
		.width = *E_screenWidth,
		.height = *E_screenHeight,
		.layers = 1,
		.renderPassDesc = pass->rpd
	};
	pass->fb = Re_CreateFramebuffer(&fbDesc);
	Re_Destroy(pass->fb);

	struct TextureDesc normalDesc =
	{
		.width = *E_screenWidth,
		.height = *E_screenHeight,
		.depth = 1,
		.type = TT_2D,
		.usage = TU_COLOR_ATTACHMENT,
		.format = TF_R16G16B16A16_SFLOAT,
		.arrayLayers = 1,
		.mipLevels = 1,
		.gpuOptimalTiling = true,
		.memoryType = MT_GPU_LOCAL
	};
	Re_AddGraphTexture("Re_normalBuffer", &normalDesc, resources);

	struct TextureDesc depthDesc =
	{
		.width = *E_screenWidth,
		.height = *E_screenHeight,
		.depth = 1,
		.type = TT_2D,
		.usage = TU_DEPTH_STENCIL_ATTACHMENT,
		.format = TF_D32_SFLOAT,
		.arrayLayers = 1,
		.mipLevels = 1,
		.gpuOptimalTiling = true,
		.memoryType = MT_GPU_LOCAL
	};
	Re_AddGraphTexture("Re_depthBuffer", &depthDesc, resources);

	return true;
}

static void
_Execute(struct DepthPrePass *pass, const struct Array *resources)
{
	struct Constants constants;

	Re_SetAttachment(pass->fb, 0, Re_GraphTexture(pass->normalHash, resources));
	Re_SetAttachment(pass->fb, 1, Re_GraphTexture(pass->depthHash, resources));

	Re_BeginDrawCommandBuffer();
	Re_CmdBeginRenderPass(pass->rpd, pass->fb, RENDER_COMMANDS_INLINE);

	Re_CmdSetViewport(0.f, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, *E_screenWidth, *E_screenHeight);

	Re_CmdBindPipeline(pass->pipeline);

	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
		struct Array *drawables = &Scn_activeScene->collect.arrays[i];
		struct Drawable *d = NULL;

		if (!drawables->count)
			continue;

		Rt_ArrayForEach(d, drawables) {
			Re_CmdBindIndexBuffer(d->indexBuffer, 0, d->indexType);

			constants.vertexAddress = d->vertexAddress;
			constants.materialAddress = d->materialAddress;
			m4_copy(&constants.mvp, &d->mvp);

			Re_CmdPushConstants(SS_ALL, sizeof(constants), &constants);
			Re_CmdDrawIndexed(d->indexCount, 1, d->firstIndex, 0, 0);
		}
	}

	Re_CmdEndRenderPass();
	Re_EndCommandBuffer();
}

static bool
_Init(struct DepthPrePass **pass)
{
	*pass = Sys_Alloc(sizeof(struct DepthPrePass), 1, MH_Render);
	if (!*pass)
		return false;

	struct Shader *shader = Re_GetShader("Depth");

	struct AttachmentDesc atDesc =
	{
		.mayAlias = false,
		.format = TF_R16G16B16A16_SFLOAT,
		.loadOp = ATL_CLEAR,
		.storeOp = ATS_STORE,
		.samples = ASC_1_SAMPLE,
		.initialLayout = TL_UNKNOWN,
		.layout = TL_COLOR_ATTACHMENT,
		.finalLayout = TL_COLOR_ATTACHMENT,
		.clearColor = { 0.f, 0.f, 0.f, 0.f }
	};
	struct AttachmentDesc depthDesc =
	{
		.mayAlias = false,
		.format = TF_D32_SFLOAT,
		.loadOp = ATL_CLEAR,
		.storeOp = ATS_STORE,
		.samples = ASC_1_SAMPLE,
		.initialLayout = TL_UNKNOWN,
		.layout = TL_DEPTH_ATTACHMENT,
		.finalLayout = TL_DEPTH_READ_ONLY_ATTACHMENT,
		.clearDepth = 0.f
	};
	(*pass)->rpd = Re_CreateRenderPassDesc(&atDesc, 1, &depthDesc);
	if (!(*pass)->rpd)
		goto error;

	struct BlendAttachmentDesc blendAttachments[] =
	{
		{ .enableBlend = false, .writeMask = RE_WRITE_MASK_RGBA }
	};
	struct GraphicsPipelineDesc pipeDesc =
	{
		.flags = RE_TOPOLOGY_TRIANGLES | RE_POLYGON_FILL |
					RE_CULL_NONE | RE_FRONT_FACE_CW |
					RE_DEPTH_TEST | RE_DEPTH_WRITE | RE_DEPTH_OP_GREATER_EQUAL,
		.shader = shader,
		.renderPassDesc = (*pass)->rpd,
		.pushConstantSize = sizeof(struct Constants),
		.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
		.attachments = blendAttachments,
		.depthFormat = TF_D32_SFLOAT
	};
	(*pass)->pipeline = Re_GraphicsPipeline(&pipeDesc);
	if (!(*pass)->pipeline)
		goto error;

	(*pass)->normalHash = Rt_HashString("Re_normalBuffer");
	(*pass)->depthHash = Rt_HashString("Re_depthBuffer");

	return true;

error:
	if ((*pass)->rpd)
		Re_DestroyRenderPassDesc((*pass)->rpd);

	Sys_Free(*pass);

	return false;
}

static void
_Term(struct DepthPrePass *pass)
{
	Re_DestroyRenderPassDesc(pass->rpd);
	Sys_Free(pass);
}
