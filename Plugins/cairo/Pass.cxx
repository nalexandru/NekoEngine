#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>
#include <UI/UI.h>
#include <Engine/Console.h>

#include "CairoUI_Internal.h"

NE_RENDER_PASS(NeCairoUI,
{
	struct NeFramebuffer *fb;
	struct NePipeline *pipeline;
	struct NeRenderPassDesc *rpd;
	bool imageUpdated;
});

static void UpdateImage(int workerId, struct NeCairoUI *pass);

static bool
NeCairoUI_Setup(struct NeCairoUI *pass, struct NeArray *resources)
{
	const struct NeTextureDesc *outDesc = Re_GraphTextureDesc(Rt_HashLiteral(RE_OUTPUT), resources);
	struct NeFramebufferAttachmentDesc atDesc = { .format = outDesc->format, .usage = outDesc->usage };
	const struct NeFramebufferDesc fbDesc =
	{
		.width = outDesc->width,
		.height = outDesc->height,
		.layers = 1,
		.attachments = &atDesc,
		.attachmentCount = 1,
		.renderPassDesc = pass->rpd
	};
	pass->fb = Re_CreateFramebuffer(&fbDesc);
	Re_Destroy(pass->fb);

	pass->imageUpdated = false;
	E_ExecuteJob((NeJobProc)UpdateImage, pass, nullptr, nullptr);
	return true;
}

static void
NeCairoUI_Execute(struct NeCairoUI *pass, const struct NeArray *resources)
{
	struct NeTextureDesc *outDesc = NULL;
	Re_SetAttachment(pass->fb, 0, Re_GraphTexture(Rt_HashLiteral(RE_OUTPUT), resources, NULL, &outDesc));
	struct NeSemaphore *passSemaphore = (struct NeSemaphore *)Re_GraphData(Rt_HashLiteral(RE_PASS_SEMAPHORE), resources);

	Re_BeginDrawCommandBuffer(passSemaphore);
	Re_CmdBeginRenderPass(pass->rpd, pass->fb, RENDER_COMMANDS_INLINE);

	Re_CmdBindPipeline(pass->pipeline);

	while (!pass->imageUpdated)
		Sys_Yield();

	struct {
		uint64_t address; uint32_t width, height;
	} constants = {
			.address = Re_BufferAddress(CRUI_imageBuffer, CRUI_bufferSize * Re_frameId),
			.width = *E_screenWidth,
			.height = *E_screenHeight
	};
	Re_CmdPushConstants(SS_ALL, sizeof(constants), &constants);

	Re_CmdSetViewport(0.f, 0.f, (float)outDesc->width, (float)outDesc->height, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, outDesc->width, outDesc->height);

	Re_CmdDraw(3, 1, 0, 0);

	Re_CmdEndRenderPass();
	Re_QueueGraphics(Re_EndCommandBuffer(), NULL);
}

static bool
NeCairoUI_Init(struct NeCairoUI **pass)
{
	*pass = (struct NeCairoUI *)Sys_Alloc(sizeof(struct NeCairoUI), 1, MH_Render);
	if (!*pass)
		return false;

	struct NeShader *shader = Re_GetShader("BufferBlend");
	struct NeAttachmentDesc atDesc =
	{
		.mayAlias = false,
		.format = Re_SwapchainFormat(Re_swapchain),
		.loadOp = ATL_LOAD,
		.storeOp = ATS_STORE,
		.samples = ASC_1_SAMPLE,
		.layout = TL_COLOR_ATTACHMENT,
		.initialLayout = TL_PRESENT_SRC,
		.finalLayout = TL_PRESENT_SRC,
		.clearColor = { .3f, .0f, .4f, 1.f }
	};
	struct NeBlendAttachmentDesc blendAttachments[] =
	{
		{
			.enableBlend = true,
			.srcColor = RE_BF_SRC_ALPHA,
			.dstColor = RE_BF_ONE_MINUS_SRC_ALPHA,
			.colorOp = RE_BOP_ADD,
			.srcAlpha = RE_BF_ONE,
			.dstAlpha = RE_BF_ZERO,
			.alphaOp = RE_BOP_ADD,
			.writeMask = RE_WRITE_MASK_RGBA
		}
	};
	struct NeGraphicsPipelineDesc pipeDesc =
	{
		.flags = RE_TOPOLOGY_TRIANGLES | RE_POLYGON_FILL | RE_CULL_NONE | RE_FRONT_FACE_CW,
		.stageInfo = &shader->opaqueStages,

		.pushConstantSize = 16,
		.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
		.attachments = blendAttachments
	};

	(*pass)->rpd = Re_CreateRenderPassDesc(&atDesc, 1, NULL, NULL, 0);
	if (!(*pass)->rpd)
		goto error;
	pipeDesc.renderPassDesc = (*pass)->rpd;

	(*pass)->pipeline = Re_GraphicsPipeline(&pipeDesc);
	if (!(*pass)->pipeline)
		goto error;

	return true;

	error:
	if ((*pass)->rpd)
		Re_DestroyRenderPassDesc((*pass)->rpd);

	Sys_Free(*pass);

	return false;
}

static void
NeCairoUI_Term(struct NeCairoUI *pass)
{
	Re_DestroyRenderPassDesc(pass->rpd);
	Sys_Free(pass);
}

static void
UpdateImage(int workerId, struct NeCairoUI *pass)
{
	memset(CRUI_imageData + CRUI_bufferSize * Re_frameId, 0x0, CRUI_bufferSize);

	const struct NeArray *comp = E_GetAllComponents(NE_CAIRO_CONTEXT_ID);
	struct NeCairoContext *ctx;
	Rt_ArrayForEach(ctx, comp, struct NeCairoContext *) {
		cairo_set_source_surface(CRUI_cairo[Re_frameId], ctx->surface, 0.0, 0.0);
		cairo_paint(CRUI_cairo[Re_frameId]);

		cairo_set_operator(ctx->cairo, CAIRO_OPERATOR_CLEAR);
		cairo_paint(ctx->cairo);
		cairo_set_operator(ctx->cairo, CAIRO_OPERATOR_OVER);
	}

	if (E_ConsoleVisible())
		CRUI_DrawConsole();

	pass->imageUpdated = true;
}

/* NekoEngine CairoUI Plugin
 *
 * Pass.cxx
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
