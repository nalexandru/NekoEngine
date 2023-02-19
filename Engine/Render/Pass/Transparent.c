#include <Scene/Scene.h>
#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>

struct NeTransparentPass;
static bool _Init(struct NeTransparentPass **pass);
static void _Term(struct NeTransparentPass *pass);
static bool _Setup(struct NeTransparentPass *pass, struct NeArray *resources);
static void _Execute(struct NeTransparentPass *pass, const struct NeArray *resources);

struct NeRenderPass RP_transparent =
{
	.Init = (NePassInitProc)_Init,
	.Term = (NePassTermProc)_Term,
	.Setup = (NePassSetupProc)_Setup,
	.Execute = (NePassExecuteProc)_Execute
};

struct NeTransparentPass
{
	struct NeFramebuffer *fb;
	uint64_t outputHash, depthHash, normalHash, sceneDataHash, instancesHash, passSemaphoreHash, visibleLightIndicesHash;
};

static bool
_Setup(struct NeTransparentPass *pass, struct NeArray *resources)
{
//	if (!Re_GraphTexture(pass->depthHash, resources))
//		return false;
	const struct NeTextureDesc *outDesc = Re_GraphTextureDesc(pass->outputHash, resources);

	struct NeFramebufferAttachmentDesc fbAtDesc[3] =
	{
		{ .usage = outDesc->usage, .format = outDesc->format },
		{ .usage = TU_DEPTH_STENCIL_ATTACHMENT | TU_SAMPLED, .format = TF_D32_SFLOAT },
		{ .usage = TU_COLOR_ATTACHMENT | TU_INPUT_ATTACHMENT, .format = TF_R16G16B16A16_SFLOAT }
	};
	
	struct NeFramebufferDesc fbDesc =
	{
		.attachmentCount = 3,
		.attachments = fbAtDesc,
		.width = outDesc->width,
		.height = outDesc->height,
		.layers = 1,
		.renderPassDesc = Re_TransparentMaterialRenderPassDesc
	};
	pass->fb = Re_CreateFramebuffer(&fbDesc);
	Re_Destroy(pass->fb);

	return true;
}

static void
_Execute(struct NeTransparentPass *pass, const struct NeArray *resources)
{
	struct NeMaterialRenderConstants constants;
	const struct NeTextureDesc *outDesc = Re_GraphTextureDesc(pass->outputHash, resources);

	constants.sceneAddress = Re_GraphBuffer(pass->sceneDataHash, resources, NULL);
	constants.visibleIndicesAddress = Re_GraphBuffer(pass->visibleLightIndicesHash, resources, NULL);
	uint64_t instanceRoot = Re_GraphBuffer(pass->instancesHash, resources, NULL);

	struct NeTexture *depthTexture = Re_GraphTexture(pass->depthHash, resources);
	struct NeTexture *normalTexture = Re_GraphTexture(pass->normalHash, resources);
	Re_SetAttachment(pass->fb, 0, Re_GraphTexture(pass->outputHash, resources));
	Re_SetAttachment(pass->fb, 1, depthTexture);
	Re_SetAttachment(pass->fb, 2, normalTexture);

	Re_BeginDrawCommandBuffer();

	/*struct NeImageBarrier depthBarrier =
	{
		.srcAccess = RE_PA_DEPTH_STENCIL_ATTACHMENT_WRITE,
		.dstAccess = RE_PA_DEPTH_STENCIL_ATTACHMENT_READ,
		.srcQueue = RE_QUEUE_GRAPHICS,
		.dstQueue = RE_QUEUE_GRAPHICS,
		.oldLayout = TL_DEPTH_ATTACHMENT,
		.newLayout = TL_DEPTH_ATTACHMENT,
		.texture = depthTexture,
		.subresource.aspect = IA_DEPTH,
		.subresource.baseArrayLayer = 0,
		.subresource.layerCount = 1,
		.subresource.mipLevel = 0,
		.subresource.levelCount = 1
	};

	struct NeImageBarrier normalBarrier =
	{
		.srcAccess = RE_PA_COLOR_ATTACHMENT_WRITE,
		.dstAccess = RE_PA_INPUT_ATTACHMENT_READ,
		.srcQueue = RE_QUEUE_GRAPHICS,
		.dstQueue = RE_QUEUE_GRAPHICS,
		.oldLayout = TL_SHADER_READ_ONLY,
		.newLayout = TL_SHADER_READ_ONLY,
		.texture = normalTexture,
		.subresource.aspect = IA_COLOR,
		.subresource.baseArrayLayer = 0,
		.subresource.layerCount = 1,
		.subresource.mipLevel = 0,
		.subresource.levelCount = 1
	};

	Re_Barrier(RE_PS_LATE_FRAGMENT_TESTS, RE_PS_EARLY_FRAGMENT_TESTS, RE_PD_BY_REGION, 0, NULL, 0, NULL, 1, &depthBarrier);
	Re_Barrier(RE_PS_COLOR_ATTACHMENT_OUTPUT, RE_PS_FRAGMENT_SHADER, RE_PD_BY_REGION, 0, NULL, 0, NULL, 1, &normalBarrier);*/

	Re_CmdBeginRenderPass(Re_TransparentMaterialRenderPassDesc, pass->fb, RENDER_COMMANDS_INLINE);

	Re_CmdSetViewport(0.f, 0.f, (float)outDesc->width, (float)outDesc->height, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, outDesc->width, outDesc->height);

	const struct NeDrawable *d = NULL;
	Rt_ArrayForEach(d, &Scn_activeScene->collect.blendedDrawables) {
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

	struct NeSemaphore *passSemaphore = Re_GraphData(pass->passSemaphoreHash, resources);
	Re_QueueGraphics(Re_EndCommandBuffer(), passSemaphore, passSemaphore);
}

static bool
_Init(struct NeTransparentPass **pass)
{
	*pass = Sys_Alloc(sizeof(struct NeTransparentPass), 1, MH_Render);
	if (!*pass)
		return false;

	(*pass)->outputHash = Rt_HashString(RE_OUTPUT);
	(*pass)->depthHash = Rt_HashString(RE_DEPTH_BUFFER);
	(*pass)->normalHash = Rt_HashString(RE_NORMAL_BUFFER);
	(*pass)->sceneDataHash = Rt_HashString(RE_SCENE_DATA);
	(*pass)->instancesHash = Rt_HashString(RE_SCENE_INSTANCES);
	(*pass)->passSemaphoreHash = Rt_HashString(RE_PASS_SEMAPHORE);
	(*pass)->visibleLightIndicesHash = Rt_HashString(RE_VISIBLE_LIGHT_INDICES);

	return true;
}

static void
_Term(struct NeTransparentPass *pass)
{
	Sys_Free(pass);
}

/* NekoEngine
 *
 * Transparent.c
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
