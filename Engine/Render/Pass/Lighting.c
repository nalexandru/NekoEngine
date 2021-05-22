#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>

struct Lighting
{
	struct Framebuffer *fb;
	struct RenderPassDesc *rpd;
};

struct LightingConstants
{
	uint32_t vertexBuffer;
	uint32_t material;
};

static void
_Setup(struct Lighting *pass)
{
	struct FramebufferAttachmentDesc fbAtDesc[3] =
	{
		{
			.usage = TU_COLOR_ATTACHMENT | TU_TRANSFER_DST,
			.format = Re_SwapchainFormat(Re_swapchain)
		},
		{
			.usage = TU_INPUT_ATTACHMENT,
			.format = TF_R16G16B16A16_SFLOAT
		},
		{
			.usage = TU_DEPTH_STENCIL_ATTACHMENT,
			.format = TF_D32_SFLOAT
		}
	};
	struct FramebufferDesc fbDesc =
	{
		.attachmentCount = 3,
		.attachments = fbAtDesc,
		.width = *E_screenWidth,
		.height = *E_screenHeight,
		.layers = 1,
		.renderPassDesc = pass->rpd
	};
	pass->fb = Re_CreateFramebuffer(&fbDesc);
	Re_Destroy(pass->fb);
}

static void
_Execute(struct Lighting *pass, void *resources)
{
/*	Re_SetAttachment(pass->fb, 0, NULL);
	Re_SetAttachment(pass->fb, 1, NULL);

	Re_BeginDrawCommandBuffer();
	Re_BeginRenderPass(pass->rpd, pass->fb, RENDER_COMMANDS_INLINE);


	Re_SetViewport(0.f, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 1.f);
	Re_SetScissor(0, 0, *E_screenWidth, *E_screenHeight);
	
	struct Array *drawables = Sys_TlsGet(0);
	struct DepthConstants constants;

	BufferHandle boundIndexBuffer = 0;
	struct Drawable *d;
	Rt_ArrayForEach(d, drawables) {
		constants.material = 0;
		constants.vertexBuffer = d->vertexBuffer;

		Re_BindPipeline(d->);
		Re_PushConstants(SS_ALL_GRAPHICS, sizeof(constants), &constants);

		if (d->indexBuffer != boundIndexBuffer) {
			Re_BindIndexBuffer(d->indexBuffer, 0, d->indexType);
			boundIndexBuffer = d->indexBuffer;
		}
		Re_DrawIndexed(d->indexCount, 1, d->firstIndex, 0, 0);
	}

	Re_EndRenderPass();
	Re_EndCommandBuffer();*/
}

static bool
_Init(struct Lighting *pass)
{
/*	struct Shader *shader = Re_GetShader("Depth");

	struct AttachmentDesc atDesc =
	{
		.mayAlias = false,
		.format = TF_R16G16B16A16_SFLOAT,
		.loadOp = ATL_CLEAR,
		.storeOp = ATS_STORE,
		.samples = ASC_1_SAMPLE,
		.clearColor = { 0.f, 0.f, 0.f, 0.f }
	};
	struct AttachmentDesc depthDesc =
	{
		.mayAlias = false,
		.format = TF_D32_SFLOAT,
		.loadOp = ATL_CLEAR,
		.storeOp = ATS_STORE,
		.samples = ASC_1_SAMPLE,
		.clearDepth = 0.f
	};
	pass->rpd = Re_CreateRenderPassDesc(&atDesc, 1, &depthDesc);

	struct BlendAttachmentDesc blendAttachments[] =
	{
		{ .enableBlend = false, .writeMask = RE_WRITE_MASK_RGBA }
	};
	struct GraphicsPipelineDesc pipeDesc =
	{
		.flags = RE_TOPOLOGY_TRIANGLES | RE_POLYGON_FILL | RE_CULL_NONE | RE_FRONT_FACE_CW | RE_DEPTH_WRITE | RE_DEPTH_TEST,
		.shader = shader,
		.renderPassDesc = pass->rpd,
		.pushConstantSize = sizeof(struct DepthConstants),
		.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
		.attachments = blendAttachments
	};
	pass->pipeline = Re_GraphicsPipeline(&pipeDesc);*/

	return false;
}

static void
_Term(struct Lighting *pass)
{
	Re_DestroyRenderPassDesc(pass->rpd);
}
