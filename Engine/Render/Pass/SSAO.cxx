#include <random>

#include <Math/Math.h>
#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Engine/ECSystem.h>
#include <Engine/Resource.h>
#include <Render/Render.h>
#include <Render/Systems.h>
#include <Render/Graph/Pass.h>
#include <Render/Graph/Graph.h>

using namespace std;

#define UNFILTERED_AO	"SSAO_unfiltered"

struct AOConstants
{
	struct NeMatrix inverseView;
	uint64_t dataAddress;
	uint64_t sceneAddress;
	uint32_t depthTexture;
};

struct BlurConstants
{
	uint32_t ssaoTexture;
	float texelSizeX;
	float texelSizeY;
};

struct SSAOData
{
	uint32_t kernelSize;
	float radius;
	float powerExponent;
	float threshold;
	float bias;
	uint32_t noiseSize;
	uint32_t noiseTexture;
	float padding;
};

NE_RENDER_PASS(NeSSAO,
{
	struct NePipeline *aoPipeline;
	struct NePipeline *blurPipeline;

	struct NeRenderPassDesc *aoRpd;
	struct NeRenderPassDesc *blurRpd;

	struct NeFramebuffer *aoFb;
	struct NeFramebuffer *blurFb;

	NeBufferHandle dataBuffer;
	NeHandle noiseTexture;

	NeTextureHandle unfilteredTexture;
	NeTextureHandle filteredTexture;
});

static bool
NeSSAO_Setup(struct NeSSAO *pass, struct NeArray *resources)
{
	const struct NeTextureDesc *swDesc = Re_GraphTextureDesc(Rt_HashLiteral(RE_OUTPUT), resources);

	struct NeTextureDesc aoDesc =
	{
		.width = swDesc->width,
		.height = swDesc->height,
		.depth = 1,
		.type = TT_2D,
		.usage = TU_COLOR_ATTACHMENT | TU_SAMPLED,
		.format = TF_R8_UNORM,
		.arrayLayers = 1,
		.mipLevels = 1,
		.gpuOptimalTiling = true,
		.memoryType = MT_GPU_LOCAL
	};
	Re_AddGraphTexture(UNFILTERED_AO, &aoDesc, pass->unfilteredTexture, resources);
	Re_AddGraphTexture(RE_AO_BUFFER, &aoDesc, pass->filteredTexture, resources);

	struct NeFramebufferAttachmentDesc fbAtDesc[2] =
	{
		{ .format = TF_R8_UNORM, .usage = TU_COLOR_ATTACHMENT | TU_SAMPLED },
		{ .format = TF_R16G16B16A16_SFLOAT, .usage = TU_COLOR_ATTACHMENT | TU_INPUT_ATTACHMENT }
	};
	struct NeFramebufferDesc fbDesc =
	{
		.width = swDesc->width,
		.height = swDesc->height,
		.layers = 1,
		.attachments = fbAtDesc,
		.attachmentCount = 2,
		.renderPassDesc = pass->aoRpd
	};
	pass->aoFb = Re_CreateFramebuffer(&fbDesc);
	Re_Destroy(pass->aoFb);

	fbDesc.attachmentCount = 1;
	fbDesc.renderPassDesc = pass->blurRpd;
	pass->blurFb = Re_CreateFramebuffer(&fbDesc);
	Re_Destroy(pass->blurFb);

	return true;
}

static void
NeSSAO_Execute(struct NeSSAO *pass, const struct NeArray *resources)
{
	struct AOConstants aoConstants{};
	struct BlurConstants blurConstants{};
	const struct NeTextureDesc *outDesc = Re_GraphTextureDesc(Rt_HashLiteral(RE_OUTPUT), resources);
	struct NeSemaphore *passSemaphore = (struct NeSemaphore *)Re_GraphData(Rt_HashLiteral(RE_PASS_SEMAPHORE), resources);

	const struct NeCamera *cam = (const struct NeCamera *)Re_GraphData(Rt_HashLiteral(RE_CAMERA), resources);
	M_Store(&aoConstants.inverseView, XMMatrixInverse(NULL, M_Load(&cam->viewMatrix)));
	aoConstants.dataAddress = Re_BufferAddress(pass->dataBuffer, 0);
	aoConstants.sceneAddress = Re_GraphBuffer(Rt_HashLiteral(RE_SCENE_DATA), resources, NULL);
	aoConstants.depthTexture = Re_GraphTextureLocation(Rt_HashLiteral(RE_DEPTH_BUFFER), resources);

	blurConstants.texelSizeX = 1.f / *E_screenWidth;
	blurConstants.texelSizeY = 1.f / *E_screenWidth;

	Re_SetAttachment(pass->aoFb, 0, Re_GraphTexture(Rt_HashLiteral(UNFILTERED_AO), resources, &blurConstants.ssaoTexture, NULL));
	Re_SetAttachment(pass->aoFb, 1, Re_GraphTexturePtr(Rt_HashLiteral(RE_NORMAL_BUFFER), resources));
	Re_SetAttachment(pass->blurFb, 0, Re_GraphTexturePtr(Rt_HashLiteral(RE_AO_BUFFER), resources));

	Re_BeginDrawCommandBuffer(passSemaphore);

	Re_CmdBeginRenderPass(pass->aoRpd, pass->aoFb, RENDER_COMMANDS_INLINE);

	Re_CmdSetViewport(0.f, 0.f, (float)outDesc->width, (float)outDesc->height, 0.f, 1.f);
	Re_CmdSetScissor(0, 0, outDesc->width, outDesc->height);

	Re_CmdBindPipeline(pass->aoPipeline);
	Re_CmdPushConstants(SS_ALL, sizeof(aoConstants), &aoConstants);
	Re_CmdDraw(3, 1, 0, 0);

	Re_CmdEndRenderPass();

	Re_CmdBeginRenderPass(pass->blurRpd, pass->blurFb, RENDER_COMMANDS_INLINE);

	// FIXME
	Re_CmdSetViewport(0.f, (float)outDesc->height, (float)outDesc->width, -(float)outDesc->height, 0.f, 1.f);

	Re_CmdBindPipeline(pass->blurPipeline);
	Re_CmdPushConstants(SS_ALL, sizeof(blurConstants), &blurConstants);
	Re_CmdDraw(3, 1, 0, 0);

	Re_CmdEndRenderPass();
	Re_QueueGraphics(Re_EndCommandBuffer(), passSemaphore);
}

static bool
NeSSAO_Init(struct NeSSAO **pass)
{
	*pass = (struct NeSSAO *)Sys_Alloc(sizeof(struct NeSSAO), 1, MH_Render);
	if (!*pass)
		return false;

	uint32_t kernelSize = E_GetCVarU32("Renderer.SSAO_KernelSize", 128)->u32;
	uint64_t dataSize = sizeof(struct SSAOData) + sizeof(struct NeVec4) * kernelSize;
	struct SSAOData *data = (struct SSAOData *)Sys_Alloc(dataSize, 1, MH_Transient);
	struct NeVec4 *kernel = (struct NeVec4 *)(((uint8_t *)data) + sizeof(*data));

	data->kernelSize = kernelSize;
	data->radius = E_GetCVarFlt("Renderer.SSAO_Radius", 8.f)->flt;
	data->powerExponent = E_GetCVarFlt("Renderer.SSAO_PowerExponent", 2.f)->flt;
	data->threshold = E_GetCVarFlt("Renderer.SSAO_Threshold", .05f)->flt;
	data->bias = E_GetCVarFlt("Renderer.SSAO_Bias", .025f)->flt;
	data->noiseSize = E_GetCVarU32("Renderer.SSAO_NoiseSize", 4)->u32;

	uniform_real_distribution<float> rd(0.f, 1.f);
	default_random_engine engine;
	for (uint32_t i = 0; i < data->kernelSize; ++i) {
		float scale = (float)i / data->kernelSize;
		scale = .1f + (scale * scale) * .9f;

		M_Store(&kernel[i], XMVectorMultiply(
			XMVector3Normalize(XMVectorSet(rd(engine) * 2.f - 1.f, rd(engine) * 2.f - 1.f, rd(engine), 1.f)),
			XMVectorReplicate(rd(engine) * scale)
		));
	}

	struct NeShader *shader = Re_GetShader("SSAO");
	struct NeAttachmentDesc atDesc
	{
		.mayAlias = false,
		.format = TF_R8_UNORM,
		.loadOp = ATL_CLEAR,
		.storeOp = ATS_STORE,
		.samples = ASC_1_SAMPLE,
		.layout = TL_COLOR_ATTACHMENT,
		.initialLayout = TL_UNKNOWN,
		.finalLayout = TL_COLOR_ATTACHMENT,
		.clearColor = { .0f, .0f, .0f, .0f }
	};
	struct NeAttachmentDesc normalDesc =
	{
		.mayAlias = false,
		.format = TF_R16G16B16A16_SFLOAT,
		.loadOp = ATL_LOAD,
		.storeOp = ATS_STORE,
		.samples = ASC_1_SAMPLE,
		.layout = TL_SHADER_READ_ONLY,
		.initialLayout = TL_SHADER_READ_ONLY,
		.finalLayout = TL_SHADER_READ_ONLY,
		.clearColor = { .0f, .0f, .0f, .0f }
	};
	(*pass)->aoRpd = Re_CreateRenderPassDesc(&atDesc, 1, NULL, &normalDesc, 1);

	atDesc.loadOp = ATL_DONT_CARE;
	(*pass)->blurRpd = Re_CreateRenderPassDesc(&atDesc, 1, NULL, NULL, 0);

	struct NeBlendAttachmentDesc blendAttachments[] =
	{
		{ .enableBlend = false, .writeMask = RE_WRITE_MASK_RGBA }
	};
	struct NeGraphicsPipelineDesc desc =
	{
		.flags = RE_TOPOLOGY_TRIANGLES | RE_POLYGON_FILL | RE_CULL_NONE | RE_FRONT_FACE_CW,
		.stageInfo = &shader->opaqueStages,
		.renderPassDesc = (*pass)->aoRpd,
		.pushConstantSize = sizeof(struct AOConstants),
		.attachmentCount = sizeof(blendAttachments) / sizeof(blendAttachments[0]),
		.attachments = blendAttachments
	};
	(*pass)->aoPipeline = Re_GraphicsPipeline(&desc);

	shader = Re_GetShader("SSAOBlur");
	desc.stageInfo = &shader->opaqueStages,
	desc.renderPassDesc = (*pass)->blurRpd;
	desc.pushConstantSize = sizeof(struct BlurConstants);
	(*pass)->blurPipeline = Re_GraphicsPipeline(&desc);

	uint32_t noiseBytes = data->noiseSize * data->noiseSize * 2;
	uint8_t *texData = (uint8_t *)Sys_Alloc(sizeof(*texData), noiseBytes, MH_Transient);
	for (uint32_t i = 0; i < noiseBytes; ++i)
		texData[i] = (uint8_t)((rd(engine) * 2.f - 1.f) * 255.f);

	struct NeTextureCreateInfo tci =
	{
		.desc =
		{
			.width = data->noiseSize,
			.height = data->noiseSize,
			.depth = 1,
			.type = TT_2D,
			.usage = TU_SAMPLED | TU_TRANSFER_DST,
			.format = TF_R8G8_UNORM,
			.arrayLayers = 1,
			.mipLevels = 1,
			.gpuOptimalTiling = true,
			.memoryType = MT_GPU_LOCAL
		},
		.data = texData,
		.dataSize = noiseBytes
	};
	(*pass)->noiseTexture = E_CreateResource("SSAONoise", RES_TEXTURE, &tci);
	E_ReleaseResource((*pass)->noiseTexture);
	data->noiseTexture = E_ResHandleToGPU((*pass)->noiseTexture);

	struct NeBufferCreateInfo bci{};
	bci.desc.size = dataSize;
	bci.desc.usage = BU_TRANSFER_DST | BU_STORAGE_BUFFER;
	bci.desc.memoryType = MT_GPU_LOCAL;
	bci.desc.name = "SSAO Data";
	bci.dataSize = dataSize;
	bci.data = data;
	Re_CreateBuffer(&bci, &(*pass)->dataBuffer);

	return Re_ReserveTextureId(&(*pass)->unfilteredTexture) && Re_ReserveTextureId(&(*pass)->filteredTexture);
}

static void
NeSSAO_Term(struct NeSSAO *pass)
{
	Re_ReleaseTextureId(pass->unfilteredTexture);
	Re_ReleaseTextureId(pass->filteredTexture);

	Re_DestroyRenderPassDesc(pass->aoRpd);
	Re_DestroyRenderPassDesc(pass->blurRpd);
	Re_Destroy(pass->dataBuffer);

	Sys_Free(pass);
}

/* NekoEngine
 *
 * SSAO.cxx
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
