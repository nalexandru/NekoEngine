#include <Math/Math.h>
#include <Scene/Scene.h>
#include <Scene/Light.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>

NE_RENDER_PASS(NeLightBoundsPass,
{
	struct NeFramebuffer *fb;
	struct NePipeline *pipeline;
	struct NeRenderPassDesc *rpd;
	NeBufferHandle vertexBuffer;
	NeBufferHandle indexBuffer;
});

static float f_cubeVertices[] =
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

static uint16_t f_cubeIndices[] =
{
	0, 1, 1, 2, 2, 3, 3, 0,
	0, 4, 4, 7, 7, 3,
	7, 6, 6, 2,
	6, 5, 5, 4,
	5, 1
};

struct Constants
{
	XMFLOAT4X4A mvp;
	uint64_t vertexAddress;
};

static bool
NeLightBoundsPass_Setup(struct NeLightBoundsPass *pass, struct NeArray *resources)
{
	const struct NeTextureDesc *outDesc = Re_GraphTextureDesc(Rt_HashLiteral(RE_OUTPUT), resources);
	struct NeFramebufferAttachmentDesc fbAtDesc[1] = { { .format = outDesc->format, .usage = outDesc->usage } };

	struct NeFramebufferDesc fbDesc =
	{
		.width = outDesc->width,
		.height = outDesc->height,
		.layers = 1,
		.attachments = fbAtDesc,
		.attachmentCount = 1,
		.renderPassDesc = pass->rpd
	};
	pass->fb = Re_CreateFramebuffer(&fbDesc);
	Re_Destroy(pass->fb);

	return true;
}

static void
NeLightBoundsPass_Execute(struct NeLightBoundsPass *pass, const struct NeArray *resources)
{
	struct Constants constants;
	struct NeTextureDesc *outDesc;
	struct NeSemaphore *passSemaphore = (struct NeSemaphore *)Re_GraphData(Rt_HashLiteral(RE_PASS_SEMAPHORE), resources);

	Re_SetAttachment(pass->fb, 0, Re_GraphTexture(Rt_HashLiteral(RE_OUTPUT), resources, NULL, &outDesc));

	Re_BeginDrawCommandBuffer(passSemaphore);
	Re_CmdBeginRenderPass(pass->rpd, pass->fb, RENDER_COMMANDS_INLINE);

	Re_CmdSetViewport(0.f, 0.f, (float)outDesc->width, (float)outDesc->height, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, outDesc->width, outDesc->height);

	Re_CmdBindPipeline(pass->pipeline);
	Re_CmdSetLineWidth(1.f);

//	Re_CmdBindVertexBuffer(pass->vertexBuffer, 0);
	Re_CmdBindIndexBuffer(pass->indexBuffer, 0, IT_UINT_16);

	constants.vertexAddress = Re_BufferAddress(pass->vertexBuffer, 0);

	const struct NeLightData * const lights = Scn_VisibleLights(Scn_activeScene);
	for (uint32_t i = 0; i < Scn_activeScene->lightCount; ++i) {
		const struct NeLightData *const ld = &lights[i];

		if (ld->type == LT_Directional)
			continue;

		const float radius = ld->outerRadius * 2;
		const XMMATRIX boundsModel = XMMatrixMultiply(
			XMMatrixScaling(radius, radius, radius),
			XMMatrixTranslation(ld->position[0], ld->position[1], ld->position[2])
		);
		XMStoreFloat4x4A(&constants.mvp, XMMatrixMultiply(boundsModel, M_Load(&Scn_activeScene->collect.vp)));

		Re_CmdPushConstants(SS_ALL, sizeof(constants), &constants);
		Re_CmdDrawIndexed(sizeof(f_cubeIndices) / sizeof(f_cubeIndices[0]), 1, 0, 0, 0);
	}

	Re_CmdEndRenderPass();
	Re_QueueGraphics(Re_EndCommandBuffer(), passSemaphore);
}

static bool
NeLightBoundsPass_Init(struct NeLightBoundsPass **pass)
{
	*pass = (struct NeLightBoundsPass *)Sys_Alloc(sizeof(struct NeLightBoundsPass), 1, MH_Render);
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
		.layout = TL_COLOR_ATTACHMENT,
		.initialLayout = TL_PRESENT_SRC,
		.finalLayout = TL_PRESENT_SRC,
		.clearColor = { .0f, .0f, .0f, .0f }
	};
	struct NeBlendAttachmentDesc blendAttachments[] =
	{
		{ .enableBlend = false, .writeMask = RE_WRITE_MASK_RGBA }
	};
	struct NeGraphicsPipelineDesc pipeDesc =
	{
		.flags = RE_TOPOLOGY_LINES | RE_POLYGON_LINE | RE_CULL_NONE,
		.stageInfo = &shader->opaqueStages,
		.pushConstantSize = sizeof(struct Constants),
		.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
		.attachments = blendAttachments
	};

	///////////////// FIXME
	struct NeBufferCreateInfo bci =
	{
		.desc =
		{
			.size = sizeof(f_cubeVertices),
			.usage = BU_STORAGE_BUFFER | BU_TRANSFER_DST,
			.memoryType = MT_GPU_LOCAL
		},
		.data = f_cubeVertices,
		.dataSize = sizeof(f_cubeVertices),
		.keepData = true
	};

	(*pass)->rpd = Re_CreateRenderPassDesc(&atDesc, 1, nullptr, nullptr, 0);
	if (!(*pass)->rpd)
		goto error;

	pipeDesc.renderPassDesc = (*pass)->rpd;

	(*pass)->pipeline = Re_GraphicsPipeline(&pipeDesc);
	if (!(*pass)->pipeline)
		goto error;

	///////////////// FIXME

	if (!Re_CreateBuffer(&bci, &(*pass)->vertexBuffer))
		return false;

	bci.desc.usage |= BU_INDEX_BUFFER;
	bci.data = f_cubeIndices;
	bci.desc.size = bci.dataSize = sizeof(f_cubeIndices);
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
NeLightBoundsPass_Term(struct NeLightBoundsPass *pass)
{
	Re_Destroy(pass->vertexBuffer);
	Re_Destroy(pass->indexBuffer);
	Re_DestroyRenderPassDesc(pass->rpd);
	Sys_Free(pass);
}

/* NekoEngine
 *
 * LightBounds.c
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
