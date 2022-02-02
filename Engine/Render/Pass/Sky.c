#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <Engine/Engine.h>
#include <Engine/Resource.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>

struct NeSkyPass;
static bool _Init(struct NeSkyPass **pass);
static void _Term(struct NeSkyPass *pass);
static bool _Setup(struct NeSkyPass *pass, struct NeArray *resources);
static void _Execute(struct NeSkyPass *pass, const struct NeArray *resources);

static float _vertices[] =
{
	-1.f, -1.f,  1.f,
	 1.f, -1.f,  1.f,
	 1.f,  1.f,  1.f,
	-1.f,  1.f,  1.f,
	-1.f, -1.f, -1.f,
	 1.f, -1.f, -1.f,
	 1.f,  1.f, -1.f,
	-1.f,  1.f, -1.f
};

static uint16_t _indices[] =
{
	0, 3, 2, 1, 0, 2,
	1, 2, 6, 5, 1, 6,
	7, 4, 5, 6, 7, 5,
	4, 7, 3, 0, 4, 3,
	4, 0, 1, 5, 4, 1,
	3, 7, 6, 2, 3, 6
};

struct NeRenderPass RP_sky =
{
	.Init = (NePassInitProc)_Init,
	.Term = (NePassTermProc)_Term,
	.Setup = (NePassSetupProc)_Setup,
	.Execute = (NePassExecuteProc)_Execute
};

struct NeSkyPass
{
	struct NeFramebuffer *fb;
	struct NePipeline *pipeline;
	struct NeRenderPassDesc *rpd;
	uint64_t depthHash, outputHash, passSemaphoreHash;
	NeBufferHandle vertexBuffer, indexBuffer;
};

struct Constants
{
	uint32_t texture;
	float exposure;
	float gamma;
	float invGamma;
	struct mat4 mvp;
	uint64_t vertexAddress;
};

static bool 
_Setup(struct NeSkyPass *pass, struct NeArray *resources)
{
	if (Scn_activeScene->environmentMap == E_INVALID_HANDLE)
		return false;

	struct NeFramebufferAttachmentDesc fbAtDesc[2] =
	{
		{ .usage = 0, .format = 0 },
		{ .usage = TU_DEPTH_STENCIL_ATTACHMENT, .format = TF_D32_SFLOAT },
	};
	Re_SwapchainDesc(Re_swapchain, &fbAtDesc[0]);

	struct NeFramebufferDesc fbDesc =
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

	return true;
}

static void
_Execute(struct NeSkyPass *pass, const struct NeArray *resources)
{
	struct Constants constants =
	{
		.vertexAddress = Re_BufferAddress(pass->vertexBuffer, 0),
		.texture = E_ResHandleToGPU(Scn_activeScene->environmentMap),
		.exposure = 1.f,
		.gamma = 2.2f
	};

	Re_SetAttachment(pass->fb, 0, Re_GraphTexture(pass->outputHash, resources));
	Re_SetAttachment(pass->fb, 1, Re_GraphTexture(pass->depthHash, resources));

	Re_BeginDrawCommandBuffer();
	Re_CmdBeginRenderPass(pass->rpd, pass->fb, RENDER_COMMANDS_INLINE);

	Re_CmdSetViewport(0.f, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, *E_screenWidth, *E_screenHeight);

	Re_CmdBindPipeline(pass->pipeline);

	Re_CmdBindIndexBuffer(pass->indexBuffer, 0, IT_UINT_16);
	
	constants.invGamma = 1.f / constants.gamma;
	m4_mul(&constants.mvp, &Scn_activeCamera->projMatrix, &Scn_activeCamera->viewMatrix);

	struct mat4 tmp;
	m4_scale(&tmp, 100000.0, 100000.0, 100000.0);
	m4_mul(&constants.mvp, &constants.mvp, &tmp);

	Re_CmdPushConstants(SS_ALL, sizeof(constants), &constants);
	Re_CmdDrawIndexed(sizeof(_indices) / sizeof(_indices[0]), 1, 0, 0, 0);

	Re_CmdEndRenderPass();

	struct NeSemaphore *passSemaphore = Re_GraphData(pass->passSemaphoreHash, resources);
	Re_QueueGraphics(Re_EndCommandBuffer(), passSemaphore, passSemaphore);
}

static bool
_Init(struct NeSkyPass **pass)
{
	*pass = Sys_Alloc(sizeof(struct NeSkyPass), 1, MH_Render);
	if (!*pass)
		return false;

	struct NeShader *shader = Re_GetShader("Sky");

	struct NeAttachmentDesc atDesc =
	{
		.mayAlias = false,
		.format = Re_SwapchainFormat(Re_swapchain),
		.loadOp = ATL_LOAD,
		.storeOp = ATS_STORE,
		.samples = ASC_1_SAMPLE,
		.initialLayout = TL_COLOR_ATTACHMENT,
		.layout = TL_COLOR_ATTACHMENT,
		.finalLayout = TL_COLOR_ATTACHMENT,
		.clearColor = { .0f, .0f, .0f, .0f }
	};
	struct NeAttachmentDesc depthDesc =
	{
		.mayAlias = false,
		.format = TF_D32_SFLOAT,
		.loadOp = ATL_LOAD,
		.storeOp = ATS_STORE,
		.samples = ASC_1_SAMPLE,
		.initialLayout = TL_DEPTH_ATTACHMENT,
		.layout = TL_DEPTH_ATTACHMENT,
		.finalLayout = TL_DEPTH_ATTACHMENT,
		.clearDepth = 0.f
	};
	(*pass)->rpd = Re_CreateRenderPassDesc(&atDesc, 1, &depthDesc, NULL, 0);
	if (!(*pass)->rpd)
		goto error;

	struct NeBlendAttachmentDesc blendAttachments[] =
	{
		{ .enableBlend = false, .writeMask = RE_WRITE_MASK_RGBA }
	};
	struct NeGraphicsPipelineDesc pipeDesc =
	{
		.flags = RE_TOPOLOGY_TRIANGLES | RE_POLYGON_FILL |
					RE_CULL_NONE | RE_FRONT_FACE_CW |
					RE_DEPTH_TEST | RE_DEPTH_OP_GREATER_EQUAL,
		.stageInfo = &shader->opaqueStages,
		.renderPassDesc = (*pass)->rpd,
		.pushConstantSize = sizeof(struct Constants),
		.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
		.attachments = blendAttachments,
		.depthFormat = TF_D32_SFLOAT
	};
	(*pass)->pipeline = Re_GraphicsPipeline(&pipeDesc);
	if (!(*pass)->pipeline)
		goto error;

	(*pass)->depthHash = Rt_HashString("Re_depthBuffer");
	(*pass)->outputHash = Rt_HashString("Re_output");
	(*pass)->passSemaphoreHash = Rt_HashString("Re_passSemaphore");

	///////////////// FIXME

	struct NeBufferCreateInfo bci =
	{
		.desc =
		{
			.size = sizeof(_vertices),
			.usage = BU_STORAGE_BUFFER | BU_TRANSFER_DST,
			.memoryType = MT_GPU_LOCAL
		},
		.data = _vertices,
		.dataSize = sizeof(_vertices),
		.keepData = true
	};
	if (!Re_CreateBuffer(&bci, &(*pass)->vertexBuffer))
		return false;

	bci.desc.usage |= BU_INDEX_BUFFER;
	bci.data = _indices;
	bci.desc.size = bci.dataSize = sizeof(_indices);
	if (!Re_CreateBuffer(&bci, &(*pass)->indexBuffer))
		return false;

	return true;

error:
	if ((*pass)->rpd)
		Re_DestroyRenderPassDesc((*pass)->rpd);

	if ((*pass)->vertexBuffer)
		Re_Destroy((*pass)->vertexBuffer);

	if ((*pass)->indexBuffer)
		Re_Destroy((*pass)->indexBuffer);

	Sys_Free(*pass);

	return false;
}

static void
_Term(struct NeSkyPass *pass)
{
	Re_Destroy(pass->vertexBuffer);
	Re_Destroy(pass->indexBuffer);
	Re_DestroyRenderPassDesc(pass->rpd);
	Sys_Free(pass);
}
