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

static struct RenderPass _depthPrePass =
{
	.Init = (PassInitProc)_Init,
	.Term = (PassTermProc)_Term,
	.Setup = (PassSetupProc)_Setup,
	.Execute = (PassExecuteProc)_Execute
};

//DECL_PASS(DepthPrePass, )

struct DepthPrePass
{
	struct Framebuffer *fb;
	struct Pipeline *pipeline;
	struct RenderPassDesc *rpd;
	uint64_t normalHash, depthHash;
};

struct DepthConstants
{
	uint32_t vertexBuffer;
	uint32_t material;
};

static bool 
_Setup(struct DepthPrePass *pass, struct Array *resources)
{
	struct FramebufferAttachmentDesc fbAtDesc[2] =
	{
		{
			.usage = TU_COLOR_ATTACHMENT | TU_TRANSFER_DST,
			.format = TF_R16G16B16A16_SFLOAT
		},
		{
			.usage = TU_DEPTH_STENCIL_ATTACHMENT,
			.format = TF_D32_SFLOAT
		}
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
	Re_AddGraphTexture("NormalBuffer", &normalDesc, resources);

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
	Re_AddGraphTexture("DepthBuffer", &depthDesc, resources);

	return true;
}

static void
_Execute(struct DepthPrePass *pass, const struct Array *resources)
{
	Re_SetAttachment(pass->fb, 0, Re_GraphTexture(pass->normalHash, resources));
	Re_SetAttachment(pass->fb, 1, Re_GraphTexture(pass->depthHash, resources));

	Re_BeginDrawCommandBuffer();
	Re_CmdBeginRenderPass(pass->rpd, pass->fb, RENDER_COMMANDS_INLINE);

	Re_CmdBindPipeline(pass->pipeline);

	Re_CmdSetViewport(0.f, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, *E_screenWidth, *E_screenHeight);
	
	struct Array *drawables = Sys_TlsGet(0);
	struct DepthConstants constants;

	BufferHandle boundIndexBuffer = 0;
	struct Drawable *d;
	Rt_ArrayForEach(d, drawables) {
		constants.material = 0;
	//	constants.vertexBuffer = d->vertexBuffer;
		Re_CmdPushConstants(SS_ALL_GRAPHICS, sizeof(constants), &constants);

		if (d->indexBuffer != boundIndexBuffer) {
			Re_CmdBindIndexBuffer(d->indexBuffer, 0, d->indexType);
			boundIndexBuffer = d->indexBuffer;
		}
		Re_CmdDrawIndexed(d->indexCount, 1, d->firstIndex, 0, 0);
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
		.finalLayout = TL_SHADER_READ_ONLY,
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
	if ((*pass)->rpd)
		goto error;

	struct BlendAttachmentDesc blendAttachments[] =
	{
		{ .enableBlend = false, .writeMask = RE_WRITE_MASK_RGBA }
	};
	struct GraphicsPipelineDesc pipeDesc =
	{
		.flags = RE_TOPOLOGY_TRIANGLES | RE_POLYGON_FILL | RE_CULL_NONE | RE_FRONT_FACE_CW | RE_DEPTH_WRITE | RE_DEPTH_TEST,
		.shader = shader,
		.renderPassDesc = (*pass)->rpd,
		.pushConstantSize = sizeof(struct DepthConstants),
		.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
		.attachments = blendAttachments
	};
	(*pass)->pipeline = Re_GraphicsPipeline(&pipeDesc);
	if ((*pass)->pipeline)
		goto error;

	(*pass)->normalHash = Rt_HashString("NormalBuffer");
	(*pass)->depthHash = Rt_HashString("DepthBuffer");

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
