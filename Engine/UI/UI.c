#include <UI/UI.h>
#include <Math/Math.h>
#include <Engine/Engine.h>
#include <Render/Render.h>

struct mat4 _projection;
struct Buffer *_vertexBuffer, *_indexBuffer;
struct RenderPass *_renderPass;
struct Framebuffer *_framebuffer;
struct Pipeline *_pipeline;
struct DescriptorSet *_descriptorSet;
struct DescriptorSetLayout *_dsLayout;

bool
UI_InitUI(void)
{
	/*struct AttachmentDesc atDesc =
	{
		.mayAlias = false,
		.format = Re_SwapchainFormat(Re_swapchain),
		.loadOp = ATL_LOAD,
		.storeOp = ATS_STORE,
		.samples = ASC_1_SAMPLE,
	};
	struct RenderPassDesc desc =
	{
		.attachmentCount = 1,
		.attachments = &atDesc,
	};
	_renderPass = Re_CreateRenderPass(&desc);

	struct FramebufferAttachmentDesc fbAtDesc =
	{
		.usage = TU_COLOR_ATTACHMENT | TU_TRANSFER_DST,
		.format = Re_SwapchainFormat(Re_swapchain)
	};
	struct FramebufferDesc fbDesc =
	{
		.attachmentCount = 1,
		.attachments = &fbAtDesc,
		.width = *E_screenWidth,
		.height = *E_screenHeight,
		.layers = 1,
		.renderPass = _renderPass
	};
	_framebuffer = Re_CreateFramebuffer(&fbDesc);

	struct BufferCreateInfo vtxInfo =
	{
		.desc =
		{
			.size = 1000,
			.usage = BU_STORAGE_BUFFER | BU_TRANSFER_DST,
			.memoryType = MT_CPU_COHERENT
		},
	};
	_vertexBuffer = Re_CreateBuffer(&vtxInfo);

	struct BufferCreateInfo idxInfo =
	{
		.desc =
		{
			.size = 1000,
			.usage = BU_INDEX_BUFFER | BU_TRANSFER_DST,
			.memoryType = MT_CPU_COHERENT
		},
	};
	_indexBuffer = Re_CreateBuffer(&idxInfo);

	struct Shader *shader = Re_GetShader("UI");

	struct BlendAttachmentDesc blendAttachments[] =
	{
		{ .enableBlend = false, .writeMask = RE_WRITE_MASK_RGB }
	};
	struct GraphicsPipelineDesc pipeDesc =
	{
		.flags = RE_TOPOLOGY_TRIANGLES | RE_POLYGON_FILL | RE_CULL_NONE | RE_FRONT_FACE_CW,
		.shader = shader,
		.renderPass = _renderPass,
		.pushConstantSize = 0,
		.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
		.attachments = blendAttachments
	};
	_pipeline = Re_GraphicsPipeline(&pipeDesc);

	m4_ortho(&_projection, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 0.f, 1.f);*/

	return true;
}

void
UI_TermUI(void)
{
	//
}

bool
UI_InitContext(struct UIContext *ctx, const void **args)
{
	uint32_t vertexCount = 64, indexCount = 100, drawCallCount = 10;

	for (; *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "VertexCount", len))
			vertexCount = atoi((char *)*(++args));
		else if (!strncmp(arg, "IndexCount", len))
			indexCount = atoi((char *)*(++args));
		else if (!strncmp(arg, "DrawCallCount", len))
			drawCallCount = atoi((char *)*(++args));
	}

	if (!Rt_InitArray(&ctx->vertices, vertexCount, sizeof(struct UIVertex), MH_Render))
		return false;

	if (!Rt_InitArray(&ctx->indices, indexCount, sizeof(uint16_t), MH_Render))
		return false;

	if (!Rt_InitArray(&ctx->draws, drawCallCount, sizeof(struct UIDrawCall), MH_Render))
		return false;

	return true;
}

void
UI_TermContext(struct UIContext *ctx)
{
	Rt_TermArray(&ctx->vertices);
	Rt_TermArray(&ctx->indices);
	Rt_TermArray(&ctx->draws);
}

void
UI_ResetContext(void **comp, void *args)
{
	struct UIContext *ctx = comp[0];

	Rt_ClearArray(&ctx->vertices, false);
	Rt_ClearArray(&ctx->indices, false);
	Rt_ClearArray(&ctx->draws, false);
}
