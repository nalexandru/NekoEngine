#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>

NE_RENDER_PASS(NeOpaquePass,
{
	struct NeFramebuffer *fb;
});

static bool
NeOpaquePass_Setup(struct NeOpaquePass *pass, struct NeArray *resources)
{
//	if (!Re_GraphTexture(pass->depthHash, resources))
//		return false;
	struct NeTextureDesc *outDesc;
	Re_GraphTexture(Rt_HashLiteral(RE_OUTPUT), resources, NULL, &outDesc);

	struct NeFramebufferAttachmentDesc fbAtDesc[3] =
	{
		{ .format = outDesc->format, .usage = outDesc->usage  },
		{ .format = TF_D32_SFLOAT, .usage = TU_DEPTH_STENCIL_ATTACHMENT | TU_SAMPLED },
		{ .format = TF_R16G16B16A16_SFLOAT, .usage = TU_COLOR_ATTACHMENT | TU_INPUT_ATTACHMENT }
	};

	struct NeFramebufferDesc fbDesc =
	{
		.width = outDesc->width,
		.height = outDesc->height,
		.layers = 1,
		.attachments = fbAtDesc,
		.attachmentCount = 3,
		.renderPassDesc = Re_MaterialRenderPassDesc,
		.name = "OpaqueFramebuffer"
	};
	pass->fb = Re_CreateFramebuffer(&fbDesc);
	Re_Destroy(pass->fb);

	return true;
}

static void
NeOpaquePass_Execute(struct NeOpaquePass *pass, const struct NeArray *resources)
{
	struct NeMaterialRenderConstants constants{};
	struct NeSemaphore *passSemaphore = (struct NeSemaphore *)Re_GraphData(Rt_HashLiteral(RE_PASS_SEMAPHORE), resources);

	struct NeTextureDesc *outDesc;
	struct NeTexture *outTexture = Re_GraphTexture(Rt_HashLiteral(RE_OUTPUT), resources, NULL, &outDesc);
	struct NeTexture *depthTexture = Re_GraphTexture(Rt_HashLiteral(RE_DEPTH_BUFFER), resources, NULL, NULL);
	struct NeTexture *normalTexture = Re_GraphTexture(Rt_HashLiteral(RE_NORMAL_BUFFER), resources, NULL, NULL);
	struct NeTexture *aoTexture = Re_GraphTexture(Rt_HashLiteral(RE_AO_BUFFER), resources, &constants.aoMap, NULL);

	struct NeBuffer *visibleIndices;
	constants.sceneAddress = Re_GraphBuffer(Rt_HashLiteral(RE_SCENE_DATA), resources, NULL);
	constants.visibleIndicesAddress = Re_GraphBuffer(Rt_HashLiteral(RE_VISIBLE_LIGHT_INDICES), resources, &visibleIndices);
	uint64_t instanceRoot = Re_GraphBuffer(Rt_HashLiteral(RE_SCENE_INSTANCES), resources, NULL);

	Re_SetAttachment(pass->fb, 0, outTexture);
	Re_SetAttachment(pass->fb, 1, depthTexture);
	Re_SetAttachment(pass->fb, 2, normalTexture);

	Re_BeginDrawCommandBuffer(passSemaphore);

	struct NeBufferBarrier visIdxBarrier =
	{
		.dstStage = RE_PS_INDEX_INPUT,
		.dstAccess = RE_PA_MEMORY_READ,
		.srcQueue = RE_QUEUE_COMPUTE,
		.dstQueue = RE_QUEUE_GRAPHICS,
		.buffer = visibleIndices,
		.size = RE_WHOLE_SIZE
	};
	struct NeImageBarrier imgBarriers[3];
	{ // Depth
		imgBarriers[0].srcStage = RE_PS_EARLY_FRAGMENT_TESTS | RE_PS_LATE_FRAGMENT_TESTS;
		imgBarriers[0].dstStage = RE_PS_EARLY_FRAGMENT_TESTS | RE_PS_LATE_FRAGMENT_TESTS;
		imgBarriers[0].srcAccess = RE_PA_DEPTH_STENCIL_ATTACHMENT_WRITE;
		imgBarriers[0].dstAccess = RE_PA_DEPTH_STENCIL_ATTACHMENT_READ;
		imgBarriers[0].srcQueue = RE_QUEUE_GRAPHICS;
		imgBarriers[0].dstQueue = RE_QUEUE_GRAPHICS;
		imgBarriers[0].oldLayout = TL_DEPTH_ATTACHMENT;
		imgBarriers[0].newLayout = TL_DEPTH_ATTACHMENT;
		imgBarriers[0].texture = depthTexture;
		imgBarriers[0].subresource.aspect = IA_DEPTH;
		imgBarriers[0].subresource.baseArrayLayer = 0;
		imgBarriers[0].subresource.layerCount = 1;
		imgBarriers[0].subresource.mipLevel = 0;
		imgBarriers[0].subresource.levelCount = 1;
	}
	{ // Normal
		imgBarriers[1].srcStage = RE_PS_COLOR_ATTACHMENT_OUTPUT;
		imgBarriers[1].srcAccess = RE_PA_COLOR_ATTACHMENT_WRITE;
		imgBarriers[1].dstStage = RE_PS_FRAGMENT_SHADER;
		imgBarriers[1].dstAccess = RE_PA_SHADER_READ;
		imgBarriers[1].srcQueue = RE_QUEUE_GRAPHICS;
		imgBarriers[1].dstQueue = RE_QUEUE_GRAPHICS;
		imgBarriers[1].oldLayout = TL_SHADER_READ_ONLY;
		imgBarriers[1].newLayout = TL_SHADER_READ_ONLY;
		imgBarriers[1].texture = normalTexture;
		imgBarriers[1].subresource.aspect = IA_COLOR;
		imgBarriers[1].subresource.baseArrayLayer = 0;
		imgBarriers[1].subresource.layerCount = 1;
		imgBarriers[1].subresource.mipLevel = 0;
		imgBarriers[1].subresource.levelCount = 1;
	}

	if (aoTexture) {
		imgBarriers[2].srcStage = RE_PS_COLOR_ATTACHMENT_OUTPUT;
		imgBarriers[2].srcAccess = RE_PA_COLOR_ATTACHMENT_WRITE;
		imgBarriers[2].dstStage = RE_PS_FRAGMENT_SHADER;
		imgBarriers[2].dstAccess = RE_PA_SHADER_READ;
		imgBarriers[2].srcQueue = RE_QUEUE_GRAPHICS;
		imgBarriers[2].dstQueue = RE_QUEUE_GRAPHICS;
		imgBarriers[2].oldLayout = TL_SHADER_READ_ONLY;
		imgBarriers[2].newLayout = TL_SHADER_READ_ONLY;
		imgBarriers[2].texture = aoTexture;
		imgBarriers[2].subresource.aspect = IA_COLOR;
		imgBarriers[2].subresource.baseArrayLayer = 0;
		imgBarriers[2].subresource.layerCount = 1;
		imgBarriers[2].subresource.mipLevel = 0;
		imgBarriers[2].subresource.levelCount = 1;
	}

	Re_CmdBarrier(RE_PD_BY_REGION, 0, NULL, 1, &visIdxBarrier, aoTexture ? 3 : 2, imgBarriers);

	Re_CmdBeginRenderPass(Re_MaterialRenderPassDesc, pass->fb, RENDER_COMMANDS_INLINE);

	Re_CmdSetViewport(0.f, 0.f, (float)outDesc->width, (float)outDesc->height, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, outDesc->width, outDesc->height);

	for (uint32_t i = 0; i < E_JobWorkerThreads(); ++i) {
		const uint32_t instanceOffset = Scn_activeScene->collect.instanceOffset[i];
		const struct NeArray *drawables = &Scn_activeScene->collect.opaqueDrawableArrays[i];

		if (!drawables->count)
			continue;

		const struct NeDrawable *d = nullptr;
		Rt_ArrayForEach(d, drawables, const struct NeDrawable *) {
			Re_CmdBindPipeline(d->material->pipeline);
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
	Re_QueueGraphics(Re_EndCommandBuffer(), passSemaphore);
}

static bool
NeOpaquePass_Init(struct NeOpaquePass **pass)
{
	*pass = (struct NeOpaquePass *)Sys_Alloc(sizeof(struct NeOpaquePass), 1, MH_Render);
	return *pass != nullptr;
}

static void
NeOpaquePass_Term(struct NeOpaquePass *pass)
{
	Sys_Free(pass);
}

/* NekoEngine
 *
 * Opaque.cxx
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
