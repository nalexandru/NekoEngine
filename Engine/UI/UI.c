#include <assert.h>

#include <Math/Math.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>

#include "Internal.h"

// FIXME
#include <Engine/IO.h>
#include <Engine/Asset.h>
#include <Engine/Resource.h>

struct UpdateBufferArgs
{
    struct UIVertex *vertices;
    uint16_t *indices;
    uint32_t vertexCount, indexCount;
};

struct UIConstants {
	uint64_t vertexAddress;
	uint32_t texture;
	uint32_t __padding;
	struct mat4 mvp;
};

struct Font UI_sysFont;

static struct mat4 _projection;
static BufferHandle _vertexBuffer, _indexBuffer;
static struct RenderPassDesc *_renderPass;
static struct Pipeline *_pipeline;
static uint64_t _vertexBufferSize = 4000 * sizeof(struct UIVertex);
static uint64_t _indexBufferSize = 6000 * sizeof(uint16_t);
static uint8_t *_vertexPtr, *_indexPtr;

bool
UI_InitUI(void)
{
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
	_renderPass = Re_CreateRenderPassDesc(&atDesc, 1, NULL);

	struct BufferCreateInfo vtxInfo =
	{
		.desc =
		{
			.size = _vertexBufferSize * RE_NUM_FRAMES,
			.usage = BU_STORAGE_BUFFER | BU_TRANSFER_DST,
			.memoryType = MT_CPU_COHERENT
		},
	};
	Re_CreateBuffer(&vtxInfo, &_vertexBuffer);
	_vertexPtr = Re_MapBuffer(_vertexBuffer);

	struct BufferCreateInfo idxInfo =
	{
		.desc =
		{
			.size = _indexBufferSize * RE_NUM_FRAMES,
			.usage = BU_INDEX_BUFFER | BU_TRANSFER_DST,
			.memoryType = MT_CPU_COHERENT
		},
	};
	Re_CreateBuffer(&idxInfo, &_indexBuffer);
	_indexPtr = Re_MapBuffer(_indexBuffer);

	struct Shader *shader = Re_GetShader("UI");

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
		.renderPassDesc = _renderPass,
		.pushConstantSize = sizeof(struct UIConstants),
		.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
		.attachments = blendAttachments
	};
	_pipeline = Re_GraphicsPipeline(&pipeDesc);

	m4_ortho(&_projection, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 0.f, 1.f);

	struct Stream stm;
	E_FileStream("/System/System.fnt", IO_READ, &stm);
	E_LoadFontAsset(&stm, &UI_sysFont);
	E_CloseStream(&stm);

	return true;
}

void
UI_TermUI(void)
{
	Re_UnmapBuffer(_vertexBuffer);
	Re_UnmapBuffer(_indexBuffer);

	E_UnloadResource(UI_sysFont.texture);
	Sys_Free(UI_sysFont.glyphs);

	Re_DestroyRenderPassDesc(_renderPass);
	Re_Destroy(_vertexBuffer);
	Re_Destroy(_indexBuffer);
}

void
UI_Update(struct Scene *s)
{
	struct UpdateBufferArgs updateArgs =
	{
		.vertices = (struct UIVertex *)(_vertexPtr + (_vertexBufferSize * Re_frameId)),
		.indices = (uint16_t *)(_indexPtr + (_indexBufferSize * Re_frameId)),
		.vertexCount = 0,
		.indexCount = 0
	};
	E_ExecuteSystemS(s, UI_UPDATE_BUFFERS, &updateArgs);
}

void
UI_Render(struct Scene *s, void *image)
{
	Re_BeginDrawCommandBuffer();

	struct FramebufferAttachmentDesc atDesc =
	{
		.usage = TU_COLOR_ATTACHMENT | TU_TRANSFER_DST,
		.format = Re_SwapchainFormat(Re_swapchain)
	};
	struct FramebufferDesc fbDesc =
	{
		.attachmentCount = 1,
		.attachments = &atDesc,
		.width = *E_screenWidth,
		.height = *E_screenHeight,
		.layers = 1,
		.renderPassDesc = _renderPass
	};
	struct Framebuffer *fb = Re_CreateFramebuffer(&fbDesc);
	Re_SetAttachment(fb, 0, Re_SwapchainTexture(Re_swapchain, image));
	Re_Destroy(fb);

	Re_CmdBeginRenderPass(_renderPass, fb, RENDER_COMMANDS_INLINE);

	Re_CmdBindPipeline(_pipeline);
	Re_CmdSetViewport(0.f, 0.f, (float)*E_screenWidth, (float)*E_screenHeight, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, *E_screenWidth, *E_screenHeight);
	Re_CmdBindIndexBuffer(_indexBuffer, _indexBufferSize * Re_frameId, IT_UINT_16);

	struct UIConstants c =
	{
		.vertexAddress = Re_BufferAddress(_vertexBuffer, _vertexBufferSize * Re_frameId),
		.texture = 0
	};
	m4_copy(&c.mvp, &_projection);

	E_ExecuteSystemS(s, UI_DRAW_CONTEXT, &c);

	Re_CmdEndRenderPass();
	Re_EndCommandBuffer();
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

	if (!Rt_InitArray(&ctx->draws, drawCallCount, sizeof(struct UIDrawCmd), MH_Render))
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

void
UI_UpdateBuffers(void **comp, void *a)
{
	size_t i = 0;
	struct UIContext *ctx = comp[0];
	struct UpdateBufferArgs *args = a;

	for (i = 0; i < ctx->draws.count; ++i) {
		struct UIDrawCmd dc = *(struct UIDrawCmd *)Rt_ArrayGet(&ctx->draws, i);

		assert(args->vertexCount + dc.vtxCount < 1000);
		assert(args->indexCount + dc.idxCount < 4000);

		memcpy(args->vertices, Rt_ArrayGet(&ctx->vertices, dc.vtxOffset), sizeof(*args->vertices) * dc.vtxCount);
		args->vertices += dc.vtxCount;

		memcpy(args->indices, Rt_ArrayGet(&ctx->indices, dc.idxOffset), sizeof(*args->indices) * dc.idxCount);
		args->indices += dc.idxCount;

		args->vertexCount += dc.vtxCount;

		dc.idxOffset = args->indexCount;
		args->indexCount += dc.idxCount;

		*(struct UIDrawCmd *)Rt_ArrayGet(&ctx->draws, i) = dc;
	}
}

void
UI_DrawContext(void **comp, void *args)
{
	struct UIContext *ctx = comp[0];
	struct UIConstants *c = args;

	for (size_t i = 0; i < ctx->draws.count; ++i) {
		const struct UIDrawCmd dc = *(const struct UIDrawCmd *)Rt_ArrayGet(&ctx->draws, i);

		c->texture = E_ResHandleToGPU(dc.texture);
		Re_CmdPushConstants(SS_ALL, sizeof(*c), c);

		Re_CmdDrawIndexed(dc.idxCount, 1, dc.idxOffset, 0, 0);
	}

	Rt_ClearArray(&ctx->vertices, false);
	Rt_ClearArray(&ctx->indices, false);
	Rt_ClearArray(&ctx->draws, false);
}
