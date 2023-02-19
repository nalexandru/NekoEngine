#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>
#include <UI/UI.h>

#include "CEFUIInternal.h"

#define CEFUI_UI_TEXTURE "CEFUI_UITexture"

struct NeCEFUIPass;
static bool _Init(struct NeCEFUIPass **pass);
static void _Term(struct NeCEFUIPass *pass);
static bool _Setup(struct NeCEFUIPass *pass, struct NeArray *resources);
static void _Execute(struct NeCEFUIPass *pass, const struct NeArray *resources);

struct NeRenderPass RP_ui =
{
	.Init = (NePassInitProc)_Init,
	.Term = (NePassTermProc)_Term,
	.Setup = (NePassSetupProc)_Setup,
	.Execute = (NePassExecuteProc)_Execute
};

struct NeCEFUIPass
{
	struct NeFramebuffer *fb;
	struct NePipeline *pipeline;
	struct NeRenderPassDesc *rpd;
	uint64_t uiHash, outputHash, passSemaphoreHash;
	struct NeBufferDesc stagingDesc;
	NeBufferHandle stagingBuffer;
};

static bool
_Setup(struct NeCEFUIPass *pass, struct NeArray *resources)
{
	const struct NeTextureDesc *outDesc = Re_GraphTextureDesc(pass->outputHash, resources);
	struct NeFramebufferAttachmentDesc atDesc = { .format = outDesc->format, .usage = outDesc->usage };

	struct NeFramebufferDesc fbDesc =
	{
		.attachmentCount = 1,
		.attachments = &atDesc,
		.width = outDesc->width,
		.height = outDesc->height,
		.layers = 1,
		.renderPassDesc = pass->rpd
	};
	pass->fb = Re_CreateFramebuffer(&fbDesc);
	Re_Destroy(pass->fb);
	
	struct NeTextureDesc uiDesc =
	{
		.width = outDesc->width,
		.height = outDesc->height,
		.depth = 1,
		.type = TT_2D,
		.usage = TU_TRANSFER_DST | TU_SAMPLED,
		.format = TF_R8G8B8A8_UNORM,
		.arrayLayers = 1,
		.mipLevels = 1,
		.gpuOptimalTiling = true,
		.memoryType = MT_GPU_LOCAL
	};
	Re_AddGraphTexture(CEFUI_UI_TEXTURE, &uiDesc, 0, resources);
	
	return true;
}

static void
_Execute(struct NeCEFUIPass *pass, const struct NeArray *resources)
{
	Re_SetAttachment(pass->fb, 0, Re_GraphTexture(pass->outputHash, resources));
	const struct NeTextureDesc *outDesc = Re_GraphTextureDesc(pass->outputHash, resources);

	uint64_t reqSize = outDesc->width * outDesc->height * 4;
	if (pass->stagingDesc.size < reqSize) {
		//
	}
	
	void *buff = Re_MapBuffer(pass->stagingBuffer);
	CEFUI_RenderUI(buff, outDesc->width, outDesc->height);
	Re_UnmapBuffer(pass->stagingBuffer);
	
	Re_BeginDrawCommandBuffer();
	
	struct NeBufferImageCopy bic;
	Re_CmdCopyBufferToTexture(pass->stagingBuffer, Re_GraphTexture(pass->uiHash, resources), &bic);
	
	Re_CmdBeginRenderPass(pass->rpd, pass->fb, RENDER_COMMANDS_INLINE);

	Re_CmdBindPipeline(pass->pipeline);
	Re_CmdSetViewport(0.f, 0.f, (float)outDesc->width, (float)outDesc->height, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, outDesc->width, outDesc->height);

	Re_CmdDraw(3, 1, 0, 0);
	
	Re_CmdEndRenderPass();

	struct NeSemaphore *passSemaphore = Re_GraphData(pass->passSemaphoreHash, resources);
	Re_QueueGraphics(Re_EndCommandBuffer(), passSemaphore, NULL);
}

static bool
_Init(struct NeCEFUIPass **pass)
{
	*pass = Sys_Alloc(sizeof(struct NeCEFUIPass), 1, MH_Render);
	if (!*pass)
		return false;

	struct NeShader *shader = Re_GetShader("IABlend");

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
		.clearColor = { .3f, .0f, .4f, 1.f }
	};
	(*pass)->rpd = Re_CreateRenderPassDesc(&atDesc, 1, NULL, NULL, 0);
	if (!(*pass)->rpd)
		goto error;

	struct NeBlendAttachmentDesc blendAttachments[] =
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
	struct NeGraphicsPipelineDesc pipeDesc =
	{
		.flags = RE_TOPOLOGY_TRIANGLES | RE_POLYGON_FILL | RE_CULL_NONE | RE_FRONT_FACE_CW,
		.stageInfo = &shader->opaqueStages,
		.renderPassDesc = (*pass)->rpd,
		.pushConstantSize = 0,
		.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
		.attachments = blendAttachments
	};
	(*pass)->pipeline = Re_GraphicsPipeline(&pipeDesc);
	if (!(*pass)->pipeline)
		goto error;

	(*pass)->uiHash = Rt_HashString(CEFUI_UI_TEXTURE);
	(*pass)->outputHash = Rt_HashString(RE_OUTPUT);
	(*pass)->passSemaphoreHash = Rt_HashString(RE_PASS_SEMAPHORE);

	return true;

error:
	if ((*pass)->rpd)
		Re_DestroyRenderPassDesc((*pass)->rpd);

	Sys_Free(*pass);

	return false;
}

static void
_Term(struct NeCEFUIPass *pass)
{
	Re_DestroyRenderPassDesc(pass->rpd);
	Sys_Free(pass);
}

/* NekoEngine CEF UI Plugin
 *
 * RenderPass.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
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
