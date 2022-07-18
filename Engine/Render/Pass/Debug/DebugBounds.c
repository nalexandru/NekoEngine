#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>

struct NeDebugBoundsPass;
static bool _Init(struct NeDebugBoundsPass **pass);
static void _Term(struct NeDebugBoundsPass *pass);
static bool _Setup(struct NeDebugBoundsPass *pass, struct NeArray *resources);
static void _Execute(struct NeDebugBoundsPass *pass, const struct NeArray *resources);

static float _cubeVertices[] =
{
	-0.5, -0.5, -0.5,
	 0.5, -0.5, -0.5,
	 0.5,  0.5, -0.5,
	-0.5,  0.5, -0.5,
	-0.5, -0.5,  0.5,
	 0.5, -0.5,  0.5,
	 0.5,  0.5,  0.5,
	-0.5,  0.5,  0.5
};

static uint16_t _cubeIndices[] =
{
	0, 1, 1, 2, 2, 3, 3, 0,
	0, 4, 4, 7, 7, 3,
	7, 6, 6, 2,
	6, 5, 5, 4,
	5, 1
};

struct NeRenderPass RP_debugBounds =
{
	.Init = (NePassInitProc)_Init,
	.Term = (NePassTermProc)_Term,
	.Setup = (NePassSetupProc)_Setup,
	.Execute = (NePassExecuteProc)_Execute
};

struct NeDebugBoundsPass
{
	struct NeFramebuffer *fb;
	struct NePipeline *pipeline;
	struct NeRenderPassDesc *rpd;
	NeBufferHandle vertexBuffer, indexBuffer;
	uint64_t outputHash, sceneDataHash, instancesHash, passSemaphoreHash, visibleLightIndicesHash;
};

struct Constants
{
	struct NeMatrix mvp;
	uint64_t vertexAddress;
};

static bool
_Setup(struct NeDebugBoundsPass *pass, struct NeArray *resources)
{
	struct NeFramebufferAttachmentDesc fbAtDesc[1] =
	{
		{ .usage = 0, .format = 0 }
	};
	Re_SwapchainDesc(Re_swapchain, &fbAtDesc[0]);

	struct NeFramebufferDesc fbDesc =
	{
		.attachmentCount = 1,
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
_Execute(struct NeDebugBoundsPass *pass, const struct NeArray *resources)
{
	struct Constants constants;

	Re_SetAttachment(pass->fb, 0, Re_GraphTexture(pass->outputHash, resources));

	Re_BeginDrawCommandBuffer();
	Re_CmdBeginRenderPass(pass->rpd, pass->fb, RENDER_COMMANDS_INLINE);

	Re_CmdSetViewport(0.f, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, *E_screenWidth, *E_screenHeight);

	Re_CmdBindPipeline(pass->pipeline);
	Re_CmdSetLineWidth(1.f);

//	Re_CmdBindVertexBuffer(pass->vertexBuffer, 0);
	Re_CmdBindIndexBuffer(pass->indexBuffer, 0, IT_UINT_16);

	constants.vertexAddress = Re_BufferAddress(pass->vertexBuffer, 0);

	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
		const struct NeArray *drawables = &Scn_activeScene->collect.opaqueDrawableArrays[i];
		const struct NeDrawable *d = NULL;

		if (!drawables->count)
			continue;

		Rt_ArrayForEach(d, drawables) {
			struct NeVec3 size, center;

			M_DivS(&center, M_AddVec3(&size, &d->bounds->aabb.max, &d->bounds->aabb.min), 2.f);
			M_Sub(&size, &d->bounds->aabb.max, &d->bounds->aabb.min);
			
			struct NeMatrix boundsModel, m_scale, m_translate;

			M_ScaleMatrix(&m_scale, size.x, size.y, size.z);
			M_TranslationMatrix(&m_translate, center.x, center.y, center.z);
			M_MulMatrix(&boundsModel, &m_translate, &m_scale);
		//	M_MulMatrix(&constants.mvp, &Scn_activeScene->collect.vp, M_MulMatrix(&boundsModel, d->modelMatrix, &boundsModel));
			M_MulMatrix(&constants.mvp, &Scn_activeScene->collect.vp, &boundsModel);

			Re_CmdPushConstants(SS_ALL, sizeof(constants), &constants);

			Re_CmdDrawIndexed(sizeof(_cubeIndices) / sizeof(_cubeIndices[0]), 1, 0, 0, 0);
		}
	}

	Re_CmdEndRenderPass();

	struct NeSemaphore *passSemaphore = Re_GraphData(pass->passSemaphoreHash, resources);
	Re_QueueGraphics(Re_EndCommandBuffer(), passSemaphore, passSemaphore);
}

static bool
_Init(struct NeDebugBoundsPass **pass)
{
	*pass = Sys_Alloc(sizeof(struct NeDebugBoundsPass), 1, MH_Render);
	if (!*pass)
		return false;

	struct NeShader *shader = Re_GetShader("DebugBounds");

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
		.clearColor = { .0f, .0f, .0f, .0f }
	};
	(*pass)->rpd = Re_CreateRenderPassDesc(&atDesc, 1, NULL, NULL, 0);
	if (!(*pass)->rpd)
		goto error;

	struct NeBlendAttachmentDesc blendAttachments[] =
	{
		{ .enableBlend = false, .writeMask = RE_WRITE_MASK_RGBA }
	};
	struct NeGraphicsPipelineDesc pipeDesc =
	{
		.flags = RE_TOPOLOGY_LINES | RE_POLYGON_LINE | RE_CULL_NONE,
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

	(*pass)->outputHash = Rt_HashString(RE_OUTPUT);
	(*pass)->sceneDataHash = Rt_HashString(RE_SCENE_DATA);
	(*pass)->instancesHash = Rt_HashString(RE_SCENE_INSTANCES);
	(*pass)->passSemaphoreHash = Rt_HashString(RE_PASS_SEMAPHORE);
	(*pass)->visibleLightIndicesHash = Rt_HashString(RE_VISIBLE_LIGHT_INDICES);

	///////////////// FIXME

	struct NeBufferCreateInfo bci =
	{
		.desc =
		{
			.size = sizeof(_cubeVertices),
			.usage = BU_STORAGE_BUFFER | BU_TRANSFER_DST,
			.memoryType = MT_GPU_LOCAL
		},
		.data = _cubeVertices,
		.dataSize = sizeof(_cubeVertices),
		.keepData = true
	};
	if (!Re_CreateBuffer(&bci, &(*pass)->vertexBuffer))
		return false;

	bci.desc.usage |= BU_INDEX_BUFFER;
	bci.data = _cubeIndices;
	bci.desc.size = bci.dataSize = sizeof(_cubeIndices);
	if (!Re_CreateBuffer(&bci, &(*pass)->indexBuffer))
		return false;

	return true;

error:
	if ((*pass)->rpd)
		Re_DestroyRenderPassDesc((*pass)->rpd);

	Sys_Free(*pass);

	return false;
}

static void
_Term(struct NeDebugBoundsPass *pass)
{
	Re_Destroy(pass->vertexBuffer);
	Re_Destroy(pass->indexBuffer);
	Re_DestroyRenderPassDesc(pass->rpd);
	Sys_Free(pass);
}
