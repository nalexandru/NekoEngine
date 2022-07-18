#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>

struct NeDepthPrePass;
static bool _Init(struct NeDepthPrePass **pass);
static void _Term(struct NeDepthPrePass *pass);
static bool _Setup(struct NeDepthPrePass *pass, struct NeArray *resources);
static void _Execute(struct NeDepthPrePass *pass, const struct NeArray *resources);

struct NeRenderPass RP_depthPrePass =
{
	.Init = (NePassInitProc)_Init,
	.Term = (NePassTermProc)_Term,
	.Setup = (NePassSetupProc)_Setup,
	.Execute = (NePassExecuteProc)_Execute
};

struct NeDepthPrePass
{
	struct NeFramebuffer *fb;
	struct NePipeline *pipeline;
	struct NeRenderPassDesc *rpd;
	uint64_t normalHash, depthHash, instancesHash, passSemaphoreHash;
};

struct Constants
{
	uint64_t vertexAddress;
	uint64_t materialAddress;
	uint64_t instanceAddress;
};

static bool 
_Setup(struct NeDepthPrePass *pass, struct NeArray *resources)
{
	struct NeFramebufferAttachmentDesc fbAtDesc[2] =
	{
		{ .usage = TU_COLOR_ATTACHMENT | TU_INPUT_ATTACHMENT, .format = TF_R16G16B16A16_SFLOAT },
		{ .usage = TU_DEPTH_STENCIL_ATTACHMENT | TU_SAMPLED, .format = TF_D32_SFLOAT }
	};
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

	struct NeTextureDesc normalDesc =
	{
		.width = *E_screenWidth,
		.height = *E_screenHeight,
		.depth = 1,
		.type = TT_2D,
		.usage = TU_COLOR_ATTACHMENT | TU_INPUT_ATTACHMENT,
		.format = TF_R16G16B16A16_SFLOAT,
		.arrayLayers = 1,
		.mipLevels = 1,
		.gpuOptimalTiling = true,
		.memoryType = MT_GPU_LOCAL
	};
	Re_AddGraphTexture(RE_NORMAL_BUFFER, &normalDesc, 0, resources);

	struct NeTextureDesc depthDesc =
	{
		.width = *E_screenWidth,
		.height = *E_screenHeight,
		.depth = 1,
		.type = TT_2D,
		.usage = TU_DEPTH_STENCIL_ATTACHMENT | TU_SAMPLED,
		.format = TF_D32_SFLOAT,
		.arrayLayers = 1,
		.mipLevels = 1,
		.gpuOptimalTiling = true,
		.memoryType = MT_GPU_LOCAL
	};
	Re_AddGraphTexture(RE_DEPTH_BUFFER, &depthDesc, RE_DEPTH_BUFFER_LOCATION, resources);

	return true;
}

static void
_Execute(struct NeDepthPrePass *pass, const struct NeArray *resources)
{
	struct Constants constants;

	Re_SetAttachment(pass->fb, 0, Re_GraphTexture(pass->normalHash, resources));
	Re_SetAttachment(pass->fb, 1, Re_GraphTexture(pass->depthHash, resources));
	uint64_t instanceRoot = Re_GraphBuffer(pass->instancesHash, resources, NULL);

	Re_BeginDrawCommandBuffer();
	Re_CmdBeginRenderPass(pass->rpd, pass->fb, RENDER_COMMANDS_INLINE);

	Re_CmdSetViewport(0.f, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, *E_screenWidth, *E_screenHeight);

	Re_CmdBindPipeline(pass->pipeline);

	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
		const uint32_t instanceOffset = Scn_activeScene->collect.instanceOffset[i];
		const struct NeArray *drawables = &Scn_activeScene->collect.opaqueDrawableArrays[i];
		const struct NeDrawable *d = NULL;

		if (!drawables->count)
			continue;

		Rt_ArrayForEach(d, drawables) {
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
	Re_QueueGraphics(Re_EndCommandBuffer(), NULL, passSemaphore);
}

static bool
_Init(struct NeDepthPrePass **pass)
{
	*pass = Sys_Alloc(sizeof(struct NeDepthPrePass), 1, MH_Render);
	if (!*pass)
		return false;

	struct NeVertexAttribute attribs[] =
	{
		{ 0, 0, VF_FLOAT3, 0 },						//float x, y, z;
		{ 1, 0, VF_FLOAT3, sizeof(float) * 3 },		//float nx, ny, nz;
		{ 2, 0, VF_FLOAT3, sizeof(float) * 6 },		//float tx, ty, tz;
		{ 3, 0, VF_FLOAT2, sizeof(float) * 9 },		//float u, v;
		{ 4, 0, VF_FLOAT4, sizeof(float) * 11 }		//float r, g, b, a;
	};

	struct NeVertexBinding bindings[] =
	{
		{ 0, sizeof(struct NeVertex), VIR_VERTEX }
	};

	struct NeShader *shader = Re_GetShader("Depth");

	struct NeAttachmentDesc atDesc =
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
	struct NeAttachmentDesc depthDesc =
	{
		.mayAlias = false,
		.format = TF_D32_SFLOAT,
		.loadOp = ATL_CLEAR,
		.storeOp = ATS_STORE,
		.samples = ASC_1_SAMPLE,
		.initialLayout = TL_UNKNOWN,
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
					RE_CULL_BACK | RE_FRONT_FACE_CCW |
					RE_DEPTH_TEST | RE_DEPTH_WRITE | RE_DEPTH_OP_GREATER_EQUAL,
		.stageInfo = &shader->opaqueStages,
		.renderPassDesc = (*pass)->rpd,
		.pushConstantSize = sizeof(struct Constants),
		.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
		.attachments = blendAttachments,
		.depthFormat = TF_D32_SFLOAT,

		.vertexDesc.attributes = attribs,
		.vertexDesc.attributeCount = sizeof(attribs) / sizeof(attribs[0]),
		.vertexDesc.bindings = bindings,
		.vertexDesc.bindingCount = sizeof(bindings) / sizeof(bindings[0])
	};
	(*pass)->pipeline = Re_GraphicsPipeline(&pipeDesc);
	if (!(*pass)->pipeline)
		goto error;

	(*pass)->normalHash = Rt_HashString(RE_NORMAL_BUFFER);
	(*pass)->depthHash = Rt_HashString(RE_DEPTH_BUFFER);
	(*pass)->instancesHash = Rt_HashString(RE_SCENE_INSTANCES);
	(*pass)->passSemaphoreHash = Rt_HashString(RE_PASS_SEMAPHORE);

	return true;

error:
	if ((*pass)->rpd)
		Re_DestroyRenderPassDesc((*pass)->rpd);

	Sys_Free(*pass);

	return false;
}

static void
_Term(struct NeDepthPrePass *pass)
{
	Re_DestroyRenderPassDesc(pass->rpd);
	Sys_Free(pass);
}
