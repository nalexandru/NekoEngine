#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>

NE_RENDER_PASS(NeDepthPrePass,
{
	struct NeFramebuffer *fb;
	struct NePipeline *pipeline;
	struct NeRenderPassDesc *rpd;
	NeTextureHandle texture;
});

struct Constants
{
	uint64_t vertexAddress;
	uint64_t materialAddress;
	uint64_t instanceAddress;
};

static bool
NeDepthPrePass_Setup(struct NeDepthPrePass *pass, struct NeArray *resources)
{
	const struct NeTextureDesc *swDesc = Re_GraphTextureDesc(Rt_HashLiteral(RE_OUTPUT), resources);
	struct NeFramebufferAttachmentDesc fbAtDesc[2] =
	{
		{ .format = TF_R16G16B16A16_SFLOAT, .usage = TU_COLOR_ATTACHMENT | TU_INPUT_ATTACHMENT },
		{ .format = TF_D32_SFLOAT, .usage = TU_DEPTH_STENCIL_ATTACHMENT | TU_SAMPLED }
	};
	struct NeFramebufferDesc fbDesc =
	{
		.width = swDesc->width,
		.height = swDesc->height,
		.layers = 1,
		.attachments = fbAtDesc,
		.attachmentCount = 2,
		.renderPassDesc = pass->rpd
	};
	pass->fb = Re_CreateFramebuffer(&fbDesc);
	Re_Destroy(pass->fb);

	struct NeTextureDesc normalDesc =
	{
		.width = swDesc->width,
		.height = swDesc->height,
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
		.width = swDesc->width,
		.height = swDesc->height,
		.depth = 1,
		.type = TT_2D,
		.usage = TU_DEPTH_STENCIL_ATTACHMENT | TU_SAMPLED,
		.format = TF_D32_SFLOAT,
		.arrayLayers = 1,
		.mipLevels = 1,
		.gpuOptimalTiling = true,
		.memoryType = MT_GPU_LOCAL
	};
	Re_AddGraphTexture(RE_DEPTH_BUFFER, &depthDesc, pass->texture, resources);

	return true;
}

static void
NeDepthPrePass_Execute(struct NeDepthPrePass *pass, const struct NeArray *resources)
{
	struct Constants constants{};
	const struct NeTextureDesc *outDesc = Re_GraphTextureDesc(Rt_HashLiteral(RE_OUTPUT), resources);
	struct NeSemaphore *passSemaphore = (struct NeSemaphore *)Re_GraphData(Rt_HashLiteral(RE_PASS_SEMAPHORE), resources);

	Re_SetAttachment(pass->fb, 0, Re_GraphTexturePtr(Rt_HashLiteral(RE_NORMAL_BUFFER), resources));
	Re_SetAttachment(pass->fb, 1, Re_GraphTexturePtr(Rt_HashLiteral(RE_DEPTH_BUFFER), resources));
	uint64_t instanceRoot = Re_GraphBuffer(Rt_HashLiteral(RE_SCENE_INSTANCES), resources, nullptr);

	Re_BeginDrawCommandBuffer(passSemaphore);
	Re_CmdBeginRenderPass(pass->rpd, pass->fb, RENDER_COMMANDS_INLINE);

	Re_CmdSetViewport(0.f, 0.f, (float)outDesc->width, (float)outDesc->height, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, outDesc->width, outDesc->height);

	Re_CmdBindPipeline(pass->pipeline);

	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
		const uint32_t instanceOffset = Scn_activeScene->collect.instanceOffset[i];
		const NeArray *drawables = &Scn_activeScene->collect.opaqueDrawableArrays[i];

		if (!drawables->count)
			continue;

		const struct NeDrawable *d = nullptr;
		Rt_ArrayForEach(d, drawables, const struct NeDrawable *) {
			Re_CmdBindVertexBuffer(d->vertexBuffer, d->vertexOffset);
			Re_CmdBindIndexBuffer(d->indexBuffer, 0, d->indexType);

			constants.vertexAddress = d->vertexAddress;
			constants.materialAddress = d->materialAddress;
			constants.instanceAddress = Re_OffsetAddress(instanceRoot, (((uint64_t)d->instanceId + instanceOffset) * sizeof(struct NeModelInstance)));

			Re_CmdPushConstants(SS_ALL, sizeof(constants), &constants);
			Re_CmdDrawIndexed(d->indexCount, 1, d->firstIndex, 0, 0);
		}
	}

	//Re_Barrier()

	Re_CmdEndRenderPass();
	Re_QueueGraphics(Re_EndCommandBuffer(), passSemaphore);
}

static bool
NeDepthPrePass_Init(struct NeDepthPrePass **pass)
{
	*pass = (struct NeDepthPrePass *)Sys_Alloc(sizeof(struct NeDepthPrePass), 1, MH_Render);
	if (!*pass)
		return false;

	struct NeVertexAttribute attribs[] =
	{
		{ 0, 0, VF_FLOAT3, 0 },						//float x, y, z;
		{ 1, 0, VF_FLOAT3, sizeof(float) * 3 },		//float nx, ny, nz;
		//{ 2, 0, VF_FLOAT3, sizeof(float) * 6 },		//float tx, ty, tz;
		{ 2, 0, VF_FLOAT2, sizeof(float) * 9 },		//float u, v;
		//{ 3, 0, VF_FLOAT4, sizeof(float) * 11 }		//float r, g, b, a;
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
		.layout = TL_COLOR_ATTACHMENT,
		.initialLayout = TL_UNKNOWN,
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
		.layout = TL_DEPTH_ATTACHMENT,
		.initialLayout = TL_UNKNOWN,
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
					RE_CULL_NONE | RE_FRONT_FACE_CCW |
					RE_DEPTH_TEST | RE_DEPTH_WRITE | RE_DEPTH_OP_GREATER_EQUAL,
		.stageInfo = &shader->opaqueStages,
		.pushConstantSize = sizeof(struct Constants),
		.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
		.attachments = blendAttachments
	};
	pipeDesc.vertexDesc.attributes = attribs;
	pipeDesc.vertexDesc.attributeCount = sizeof(attribs) / sizeof(attribs[0]);
	pipeDesc.vertexDesc.bindings = bindings;
	pipeDesc.vertexDesc.bindingCount = sizeof(bindings) / sizeof(bindings[0]);

	(*pass)->rpd = Re_CreateRenderPassDesc(&atDesc, 1, &depthDesc, NULL, 0);
	if (!(*pass)->rpd)
		goto error;

	pipeDesc.renderPassDesc = (*pass)->rpd;

	(*pass)->pipeline = Re_GraphicsPipeline(&pipeDesc);
	if (!(*pass)->pipeline)
		goto error;

	return Re_ReserveTextureId(&(*pass)->texture);

error:
	if ((*pass)->rpd)
		Re_DestroyRenderPassDesc((*pass)->rpd);

	Sys_Free(*pass);

	return false;
}

static void
NeDepthPrePass_Term(struct NeDepthPrePass *pass)
{
	Re_ReleaseTextureId(pass->texture);
	Re_DestroyRenderPassDesc(pass->rpd);
	Sys_Free(pass);
}

/* NekoEngine
 *
 * Depth.cxx
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
