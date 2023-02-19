#include <Math/Math.h>
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
	XMFLOAT4X4A mvp;
	uint64_t vertexAddress;
};

static bool 
_Setup(struct NeSkyPass *pass, struct NeArray *resources)
{
	if (Scn_activeScene->environmentMap == E_INVALID_HANDLE)
		return false;

	const struct NeTextureDesc *outDesc = Re_GraphTextureDesc(pass->outputHash, resources);

	struct NeFramebufferAttachmentDesc fbAtDesc[2] =
	{
		{ .format = outDesc->format, .usage = outDesc->usage },
		{ .format = TF_D32_SFLOAT, .usage = TU_DEPTH_STENCIL_ATTACHMENT | TU_SAMPLED },
	};

	struct NeFramebufferDesc fbDesc =
	{
		.width = outDesc->width,
		.height = outDesc->height,
		.layers = 1,
		.attachments = fbAtDesc,
		.attachmentCount = 2,
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
		.texture = E_ResHandleToGPU(Scn_activeScene->environmentMap),
		.exposure = 1.f,
		.gamma = 2.2f,
		.vertexAddress = Re_BufferAddress(pass->vertexBuffer, 0)
	};
	const struct NeTextureDesc *outDesc = Re_GraphTextureDesc(pass->outputHash, resources);

	Re_SetAttachment(pass->fb, 0, Re_GraphTexture(pass->outputHash, resources));
	Re_SetAttachment(pass->fb, 1, Re_GraphTexture(pass->depthHash, resources));

	Re_BeginDrawCommandBuffer();
	Re_CmdBeginRenderPass(pass->rpd, pass->fb, RENDER_COMMANDS_INLINE);

	Re_CmdSetViewport(0.f, 0.f, (float)outDesc->width, (float)outDesc->height, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, outDesc->width, outDesc->height);

	Re_CmdBindPipeline(pass->pipeline);

	Re_CmdBindIndexBuffer(pass->indexBuffer, 0, IT_UINT_16);
	
	constants.invGamma = 1.f / constants.gamma;

	const XMMATRIX mvp = XMMatrixMultiply(M_Load(&Scn_activeCamera->viewMatrix), M_Load(&Scn_activeCamera->projMatrix));
	XMStoreFloat4x4A(&constants.mvp, XMMatrixMultiply(XMMatrixScaling(1000000.0, 1000000.0, 1000000.0), mvp));

	Re_CmdPushConstants(SS_ALL, sizeof(constants), &constants);
	Re_CmdDrawIndexed(sizeof(_indices) / sizeof(_indices[0]), 1, 0, 0, 0);

	Re_CmdEndRenderPass();

	struct NeSemaphore *passSemaphore = (struct NeSemaphore *)Re_GraphData(pass->passSemaphoreHash, resources);
	Re_QueueGraphics(Re_EndCommandBuffer(), passSemaphore, passSemaphore);
}

static bool
_Init(struct NeSkyPass **pass)
{
	*pass = (struct NeSkyPass *)Sys_Alloc(sizeof(struct NeSkyPass), 1, MH_Render);
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
		.layout = TL_COLOR_ATTACHMENT,
		.initialLayout = TL_COLOR_ATTACHMENT,
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
		.layout = TL_DEPTH_ATTACHMENT,
		.initialLayout = TL_DEPTH_ATTACHMENT,
		.finalLayout = TL_DEPTH_ATTACHMENT,
		.clearDepth = 0.f
	};

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
		.pushConstantSize = sizeof(struct Constants),
		.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
		.attachments = blendAttachments,
		.depthFormat = TF_D32_SFLOAT
	};

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

	(*pass)->rpd = Re_CreateRenderPassDesc(&atDesc, 1, &depthDesc, NULL, 0);
	if (!(*pass)->rpd)
		goto error;

	pipeDesc.renderPassDesc = (*pass)->rpd;

	(*pass)->pipeline = Re_GraphicsPipeline(&pipeDesc);
	if (!(*pass)->pipeline)
		goto error;

	(*pass)->depthHash = Rt_HashString(RE_DEPTH_BUFFER);
	(*pass)->outputHash = Rt_HashString(RE_OUTPUT);
	(*pass)->passSemaphoreHash = Rt_HashString(RE_PASS_SEMAPHORE);

	///////////////// FIXME
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

/* NekoEngine
 *
 * Sky.c
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
