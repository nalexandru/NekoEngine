#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>

NE_RENDER_PASS(NeTransparentPass,
{
	struct NeFramebuffer *fb;
});

static bool
NeTransparentPass_Setup(struct NeTransparentPass *pass, struct NeArray *resources)
{
//	if (!Re_GraphTexture(pass->depthHash, resources))
//		return false;
	const struct NeTextureDesc *outDesc = Re_GraphTextureDesc(Rt_HashLiteral(RE_OUTPUT), resources);

	struct NeFramebufferAttachmentDesc fbAtDesc[3] =
	{
		{ .format = outDesc->format, .usage = outDesc->usage },
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
		.renderPassDesc = Re_TransparentMaterialRenderPassDesc
	};
	pass->fb = Re_CreateFramebuffer(&fbDesc);
	Re_Destroy(pass->fb);

	return true;
}

static void
NeTransparentPass_Execute(struct NeTransparentPass *pass, const struct NeArray *resources)
{
	struct NeMaterialRenderConstants constants{};

	struct NeTextureDesc *outDesc;
	struct NeTexture *outTexture = Re_GraphTexture(Rt_HashLiteral(RE_OUTPUT), resources, NULL, &outDesc);
	struct NeSemaphore *passSemaphore = (struct NeSemaphore *)Re_GraphData(Rt_HashLiteral(RE_PASS_SEMAPHORE), resources);

	constants.sceneAddress = Re_GraphBuffer(Rt_HashLiteral(RE_SCENE_DATA), resources, NULL);
	constants.visibleIndicesAddress = Re_GraphBuffer(Rt_HashLiteral(RE_VISIBLE_LIGHT_INDICES), resources, NULL);
	constants.aoMap = Re_GraphTextureLocation(Rt_HashLiteral(RE_AO_BUFFER), resources);
	uint64_t instanceRoot = Re_GraphBuffer(Rt_HashLiteral(RE_SCENE_INSTANCES), resources, NULL);

	Re_SetAttachment(pass->fb, 0, outTexture);
	Re_SetAttachment(pass->fb, 1, Re_GraphTexturePtr(Rt_HashLiteral(RE_DEPTH_BUFFER), resources));
	Re_SetAttachment(pass->fb, 2,  Re_GraphTexturePtr(Rt_HashLiteral(RE_NORMAL_BUFFER), resources));

	Re_BeginDrawCommandBuffer(passSemaphore);
	Re_CmdBeginRenderPass(Re_TransparentMaterialRenderPassDesc, pass->fb, RENDER_COMMANDS_INLINE);

	Re_CmdSetViewport(0.f, 0.f, (float)outDesc->width, (float)outDesc->height, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, outDesc->width, outDesc->height);

	const struct NeDrawable *d = NULL;
	Rt_ArrayForEach(d, &Scn_activeScene->collect.blendedDrawables, const struct NeDrawable *) {
		Re_CmdBindPipeline(d->material->pipeline);
		Re_CmdBindVertexBuffer(d->vertexBuffer, d->vertexOffset);
		Re_CmdBindIndexBuffer(d->indexBuffer, 0, d->indexType);
		
		constants.vertexAddress = d->vertexAddress;
		constants.materialAddress = d->materialAddress;
		constants.instanceAddress = Re_OffsetAddress(instanceRoot, (((uint64_t)d->instanceId) * sizeof(struct NeModelInstance)));

		Re_CmdPushConstants(SS_ALL, sizeof(constants), &constants);
		Re_CmdDrawIndexed(d->indexCount, 1, d->firstIndex, 0, 0);
	}

	Re_CmdEndRenderPass();
	Re_QueueGraphics(Re_EndCommandBuffer(), passSemaphore);
}

static bool
NeTransparentPass_Init(struct NeTransparentPass **pass)
{
	*pass = (struct NeTransparentPass *)Sys_Alloc(sizeof(struct NeTransparentPass), 1, MH_Render);
	return *pass != nullptr;
}

static void
NeTransparentPass_Term(struct NeTransparentPass *pass)
{
	Sys_Free(pass);
}

/* NekoEngine
 *
 * Transparent.cxx
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
