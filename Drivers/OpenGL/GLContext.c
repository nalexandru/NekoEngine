#include <assert.h>

#include <System/Memory.h>
#include <Runtime/Runtime.h>

#include "OpenGLDriver.h"

struct CommandBuffer
{
	void *a;
};

//

struct SubmitInfo
{
//	DkCmdBuf cmdBuffer;
//	DkQueue queue;
	void *a;
};

struct NeRenderContext *
GL_CreateContext(struct NeRenderDevice *dev)
{
	struct NeRenderContext *ctx = Sys_Alloc(1, sizeof(*ctx), MH_RenderDriver);
	if (!ctx)
		return NULL;

/*	VkCommandPoolCreateInfo poolInfo =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
	};

	struct Array *arrays = Sys_Alloc(RE_NUM_FRAMES * 4, sizeof(*arrays), MH_RenderDriver);
	if (!arrays)
		return NULL;

	ctx->graphicsCmdBuffers = arrays;
	ctx->secondaryCmdBuffers = &arrays[RE_NUM_FRAMES];
	ctx->computeCmdBuffers = &arrays[RE_NUM_FRAMES * 2];
	ctx->xferCmdBuffers = &arrays[RE_NUM_FRAMES * 3];

	VkCommandPool *pools = Sys_Alloc(RE_NUM_FRAMES * 3, sizeof(*pools), MH_RenderDriver);
	if (!pools)
		return NULL;

	ctx->graphicsPools = pools;
	ctx->computePools = &pools[RE_NUM_FRAMES];
	ctx->xferPools = &pools[RE_NUM_FRAMES * 2];

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		poolInfo.queueFamilyIndex = dev->graphicsFamily;
		if (vkCreateCommandPool(dev->dev, &poolInfo, Vkd_allocCb, &ctx->graphicsPools[i]) != VK_SUCCESS)
			goto error;

		poolInfo.queueFamilyIndex = dev->computeFamily;
		if (vkCreateCommandPool(dev->dev, &poolInfo, Vkd_allocCb, &ctx->computePools[i]) != VK_SUCCESS)
			goto error;

		poolInfo.queueFamilyIndex = dev->transferFamily;
		if (vkCreateCommandPool(dev->dev, &poolInfo, Vkd_allocCb, &ctx->xferPools[i]) != VK_SUCCESS)
			goto error;

		if (!Rt_InitPtrArray(&ctx->graphicsCmdBuffers[i], 10, MH_RenderDriver))
			goto error;

		if (!Rt_InitPtrArray(&ctx->xferCmdBuffers[i], 10, MH_RenderDriver))
			goto error;

		if (!Rt_InitPtrArray(&ctx->computeCmdBuffers[i], 10, MH_RenderDriver))
			goto error;

		if (!Rt_InitPtrArray(&ctx->secondaryCmdBuffers[i], 10, MH_RenderDriver))
			goto error;
	}

	VkFenceCreateInfo fci = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	vkCreateFence(dev->dev, &fci, Vkd_allocCb, &ctx->executeFence);

	ctx->vkDev = dev->dev;
	ctx->neDev = dev;
	ctx->descriptorSet = dev->descriptorSet;

	if (!Rt_InitArray(&ctx->queued.graphics, 10, sizeof(struct Vkd_SubmitInfo), MH_RenderDriver) ||
		!Rt_InitArray(&ctx->queued.compute, 10, sizeof(struct Vkd_SubmitInfo), MH_RenderDriver) ||
		!Rt_InitArray(&ctx->queued.xfer, 10, sizeof(struct Vkd_SubmitInfo), MH_RenderDriver))
		goto error;*/

	return ctx;

/*error:
	if (pools) {
		for (uint32_t i = 0; i < RE_NUM_FRAMES * 3; ++i)
			if (pools[i] != VK_NULL_HANDLE)
				vkDestroyCommandPool(dev->dev, pools[i], Vkd_allocCb);
	}

	Sys_Free(arrays);
	Sys_Free(pools);
	Sys_Free(ctx);

	return NULL;*/
}

void
GL_ResetContext(struct NeRenderDevice *dev, struct NeRenderContext *ctx)
{
/*	if (ctx->secondaryCmdBuffers[Re_frameId].count) {
		vkFreeCommandBuffers(dev->dev, ctx->graphicsPools[Re_frameId],
			(uint32_t)ctx->secondaryCmdBuffers[Re_frameId].count, (const VkCommandBuffer *)ctx->secondaryCmdBuffers[Re_frameId].data);
		Rt_ClearArray(&ctx->secondaryCmdBuffers[Re_frameId], false);
	}

	if (ctx->graphicsCmdBuffers[Re_frameId].count) {
		vkFreeCommandBuffers(dev->dev, ctx->graphicsPools[Re_frameId],
			(uint32_t)ctx->graphicsCmdBuffers[Re_frameId].count, (const VkCommandBuffer *)ctx->graphicsCmdBuffers[Re_frameId].data);
		vkResetCommandPool(dev->dev, ctx->graphicsPools[Re_frameId], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		Rt_ClearArray(&ctx->graphicsCmdBuffers[Re_frameId], false);
	}

	if (ctx->xferCmdBuffers[Re_frameId].count) {
		vkFreeCommandBuffers(dev->dev, ctx->xferPools[Re_frameId],
			(uint32_t)ctx->xferCmdBuffers[Re_frameId].count, (const VkCommandBuffer *)ctx->xferCmdBuffers[Re_frameId].data);
		vkResetCommandPool(dev->dev, ctx->xferPools[Re_frameId], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		Rt_ClearArray(&ctx->xferCmdBuffers[Re_frameId], false);
	}
	ctx->lastSubmittedXfer = 0;

	if (ctx->computeCmdBuffers[Re_frameId].count) {
		vkFreeCommandBuffers(dev->dev, ctx->computePools[Re_frameId],
			(uint32_t)ctx->computeCmdBuffers[Re_frameId].count, (const VkCommandBuffer *)ctx->computeCmdBuffers[Re_frameId].data);
		vkResetCommandPool(dev->dev, ctx->computePools[Re_frameId], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		Rt_ClearArray(&ctx->computeCmdBuffers[Re_frameId], false);
	}
	ctx->lastSubmittedCompute = 0;

	Rt_ClearArray(&ctx->queued.graphics, false);
	Rt_ClearArray(&ctx->queued.compute, false);
	Rt_ClearArray(&ctx->queued.xfer, false);*/
}

void
GL_DestroyContext(struct NeRenderDevice *dev, struct NeRenderContext *ctx)
{
/*	vkDestroyFence(dev->dev, ctx->executeFence, Vkd_allocCb);

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		vkDestroyCommandPool(dev->dev, ctx->graphicsPools[i], Vkd_allocCb);
		vkDestroyCommandPool(dev->dev, ctx->computePools[i], Vkd_allocCb);
		vkDestroyCommandPool(dev->dev, ctx->xferPools[i], Vkd_allocCb);
		Rt_TermArray(&ctx->graphicsCmdBuffers[i]);
		Rt_TermArray(&ctx->secondaryCmdBuffers[i]);
		Rt_TermArray(&ctx->xferCmdBuffers[i]);
		Rt_TermArray(&ctx->computeCmdBuffers[i]);
	}

	Rt_TermArray(&ctx->queued.graphics);
	Rt_TermArray(&ctx->queued.compute);
	Rt_TermArray(&ctx->queued.xfer);

	Sys_Free(ctx->graphicsCmdBuffers);
	Sys_Free(ctx->graphicsPools);*/
	Sys_Free(ctx);
}

static NeCommandBufferHandle
_BeginSecondary(struct NeRenderContext *ctx, struct NeRenderPassDesc *passDesc)
{
/*	ctx->cmdBuffer = Vkd_AllocateCmdBuffer(ctx->vkDev, VK_COMMAND_BUFFER_LEVEL_SECONDARY,
														ctx->graphicsPools[Re_frameId], &ctx->secondaryCmdBuffers[Re_frameId]);
	assert(ctx->cmdBuffer);

	VkCommandBufferInheritanceInfo inheritanceInfo =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
		.renderPass = passDesc->rp,
	};
	VkCommandBufferBeginInfo beginInfo =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
		.pInheritanceInfo = &inheritanceInfo
	};
	vkBeginCommandBuffer(ctx->cmdBuffer, &beginInfo);*/

	return NULL; // ctx->cmdBuffer;
}

static void
_BeginDrawCommandBuffer(struct NeRenderContext *ctx)
{
/*	ctx->cmdBuffer = Vkd_AllocateCmdBuffer(ctx->vkDev, VK_COMMAND_BUFFER_LEVEL_PRIMARY, ctx->graphicsPools[Re_frameId], &ctx->graphicsCmdBuffers[Re_frameId]);
	assert(ctx->cmdBuffer);

	VkCommandBufferBeginInfo beginInfo =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	vkBeginCommandBuffer(ctx->cmdBuffer, &beginInfo);*/
}

static void
_BeginComputeCommandBuffer(struct NeRenderContext *ctx)
{
/*	ctx->cmdBuffer = Vkd_AllocateCmdBuffer(ctx->vkDev, VK_COMMAND_BUFFER_LEVEL_PRIMARY, ctx->computePools[Re_frameId], &ctx->computeCmdBuffers[Re_frameId]);
	assert(ctx->cmdBuffer);

	VkCommandBufferBeginInfo beginInfo =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	vkBeginCommandBuffer(ctx->cmdBuffer, &beginInfo);*/
}

static void
_BeginTransferCommandBuffer(struct NeRenderContext *ctx)
{
/*	ctx->cmdBuffer = Vkd_AllocateCmdBuffer(ctx->vkDev, VK_COMMAND_BUFFER_LEVEL_PRIMARY, ctx->xferPools[Re_frameId], &ctx->xferCmdBuffers[Re_frameId]);
	assert(ctx->cmdBuffer);

	VkCommandBufferBeginInfo beginInfo =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	vkBeginCommandBuffer(ctx->cmdBuffer, &beginInfo);*/
}

static NeCommandBufferHandle
_EndCommandBuffer(struct NeRenderContext *ctx)
{
/*	VkCommandBuffer cb = ctx->cmdBuffer;

	vkEndCommandBuffer(ctx->cmdBuffer);
	ctx->cmdBuffer = VK_NULL_HANDLE;

	return cb;*/
	return NULL;
}

static void
_BindPipeline(struct NeRenderContext *ctx, struct NePipeline *pipeline)
{
/*	VkDescriptorSet sets[] = {ctx->descriptorSet, ctx->iaSet};
	vkCmdBindDescriptorSets(ctx->cmdBuffer, pipeline->bindPoint, pipeline->layout, 0, ctx->iaSet ? 2 : 1, sets, 0, NULL); */

/*	dkCmdBufBindRasterizerState(ctx->cmdBuffer, &pipeline->rasterizer);
	dkCmdBufBindColorState(ctx->cmdBuffer, &pipeline->color);
	dkCmdBufBindColorWriteState(ctx->cmdBuffer, &pipeline->colorWrite);
	dkCmdBufBindMultisampleState(ctx->cmdBuffer, &pipeline->multisample);
	dkCmdBufBindBlendStates(ctx->cmdBuffer, 0, pipeline->blendStates, pipeline->blendStateCount);
	dkCmdBufBindDepthStencilState(ctx->cmdBuffer, &pipeline->depthStencil);

	dkCmdBufBindShaders(ctx->cmdBuffer, pipeline->stageMask, (const DkShader * const *)pipeline->shaders, pipeline->shaderCount);*/

	ctx->boundPipeline = pipeline;
}

static void
_PushConstants(struct NeRenderContext *ctx, enum NeShaderStage stage, uint32_t size, const void *data)
{
//	vkCmdPushConstants(ctx->cmdBuffer, ctx->boundPipeline->layout, stage, 0, size, data);
}

static void
_BindIndexBuffer(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset, enum NeIndexType type)
{
//	vkCmdBindIndexBuffer(ctx->cmdBuffer, buff->buff, offset, type);
}

static void
_ExecuteSecondary(struct NeRenderContext *ctx, NeCommandBufferHandle *cmdBuffers, uint32_t count)
{
//	vkCmdExecuteCommands(ctx->cmdBuffer, count, (VkCommandBuffer *)cmdBuffers);
}

static void
_BeginRenderPass(struct NeRenderContext *ctx, struct NeRenderPassDesc *passDesc, struct NeFramebuffer *fb, enum NeRenderCommandContents contents)
{
	//if (!Re_deviceInfo.features.coherentMemory)
	//	Vkd_StagingBarrier(ctx->cmdBuffer);

/*	if (passDesc->inputAttachments) {
		ctx->iaSet = GL_AllocateIADescriptorSet(ctx->neDev);
		for (uint32_t i = 0; i < passDesc->inputAttachments; ++i)
			GL_SetInputAttachment(ctx->neDev, ctx->iaSet, i, fb->attachments[fb->attachmentCount - i - 1]);
	}

	VkRenderPassAttachmentBeginInfo atbi =
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO,
		.attachmentCount = fb->attachmentCount,
		.pAttachments = fb->attachments
	};
	VkRenderPassBeginInfo rpbi =
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = &atbi,
		.renderPass = passDesc->rp,
		.framebuffer = fb->fb,
		.renderArea = { { 0, 0 }, { fb->width, fb->height } },
		.clearValueCount = passDesc->clearValueCount,
		.pClearValues = passDesc->clearValues
	};
	vkCmdBeginRenderPass(ctx->cmdBuffer, &rpbi, contents);*/
}

static void
_EndRenderPass(struct NeRenderContext *ctx)
{
//	vkCmdEndRenderPass(ctx->cmdBuffer);
//	ctx->iaSet = VK_NULL_HANDLE;
}

static void
_SetViewport(struct NeRenderContext *ctx, float x, float y, float width, float height, float minDepth, float maxDepth)
{
//	DkViewport vp = { x, height + y, width, -height, minDepth, maxDepth };
//	dkCmdBufSetViewports(ctx->cmdBuffer, 0, &vp, 1);
}

static void
_SetScissor(struct NeRenderContext *ctx, int32_t x, int32_t y, int32_t width, int32_t height)
{
//	DkScissor scissor = { x, y, width, height };
//	dkCmdBufSetScissors(ctx->cmdBuffer, 0, &scissor, 1);
}

static void
_Draw(struct NeRenderContext *ctx, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
//	dkCmdBufDraw(ctx->cmdBuffer, ctx->boundPipeline->primitiveType, vertexCount, instanceCount, firstVertex, firstInstance);
}

static void
_DrawIndexed(struct NeRenderContext *ctx, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
//	dkCmdBufDrawIndexed(ctx->cmdBuffer, ctx->boundPipeline->primitiveType, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

static void
_DrawIndirect(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
//	dkCmdBufDrawIndirect(ctx->cmdBuffer, ctx->boundPipeline->primitiveType, dkMemBlockGetGpuAddr(buff->memory) + offset);
}

static void
_DrawIndexedIndirect(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
//	dkCmdBufDrawIndexedIndirect(ctx->cmdBuffer, ctx->boundPipeline->primitiveType, dkMemBlockGetGpuAddr(buff->memory) + offset);
}

static void
_Dispatch(struct NeRenderContext *ctx, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
//	dkCmdBufDispatchCompute(ctx->cmdBuffer, groupCountX, groupCountY, groupCountZ);
}

static void
_DispatchIndirect(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset)
{
//	dkCmdBufDispatchComputeIndirect(ctx->cmdBuffer, dkMemBlockGetGpuAddr(buff->memory) + offset);
}

static void
_TraceRays(struct NeRenderContext *ctx, uint32_t width, uint32_t height, uint32_t depth)
{ }

static void
_TraceRaysIndirect(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset)
{ }

static void
_BuildAccelerationStructures(struct NeRenderContext *ctx, uint32_t count, struct NeAccelerationStructureBuildInfo *buildInfo, const struct NeAccelerationStructureRangeInfo **rangeInfo)
{ }

static void
_Barrier(struct NeRenderContext *ctx, enum NePipelineStage srcStage, enum NePipelineStage dstStage, enum NePipelineDependency dep,
	uint32_t memBarrierCount, const struct NeMemoryBarrier *memBarriers, uint32_t bufferBarrierCount, const struct NeBufferBarrier *bufferBarriers,
	uint32_t imageBarrierCount, const struct NeImageBarrier *imageBarriers)
{
/*	VkMemoryBarrier *vkMemBarriers = NULL;
	VkBufferMemoryBarrier *vkBufferBarriers = NULL;
	VkImageMemoryBarrier *vkImageBarriers = NULL;

	if (memBarrierCount) {
		vkMemBarriers = Sys_Alloc(sizeof(*vkMemBarriers), memBarrierCount, MH_Transient);

		for (uint32_t i = 0; i < memBarrierCount; ++i) {
			vkMemBarriers[i].sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			vkMemBarriers[i].pNext = NULL;
			vkMemBarriers[i].srcAccessMask = memBarriers[i].srcAccess;
			vkMemBarriers[i].dstAccessMask = memBarriers[i].dstAccess;
		}
	}

	if (bufferBarrierCount) {
		vkBufferBarriers = Sys_Alloc(sizeof(*vkBufferBarriers), bufferBarrierCount, MH_Transient);

		for (uint32_t i = 0; i < bufferBarrierCount; ++i) {
			vkBufferBarriers[i].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			vkBufferBarriers[i].pNext = NULL;
			vkBufferBarriers[i].srcAccessMask = bufferBarriers[i].srcAccess;
			vkBufferBarriers[i].dstAccessMask = bufferBarriers[i].dstAccess;
			vkBufferBarriers[i].srcQueueFamilyIndex = _queueFamilyIndex(ctx->neDev, bufferBarriers[i].srcQueue);
			vkBufferBarriers[i].dstQueueFamilyIndex = _queueFamilyIndex(ctx->neDev, bufferBarriers[i].dstQueue);
			vkBufferBarriers[i].buffer = bufferBarriers[i].buffer->buff;
			vkBufferBarriers[i].offset = bufferBarriers[i].offset;
			vkBufferBarriers[i].size = bufferBarriers[i].size;
		}
	}

	if (imageBarrierCount) {
		vkImageBarriers = Sys_Alloc(sizeof(*vkImageBarriers), imageBarrierCount, MH_Transient);

		for (uint32_t i = 0; i < imageBarrierCount; ++i) {
			vkImageBarriers[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			vkImageBarriers[i].pNext = NULL;
			vkImageBarriers[i].srcAccessMask = imageBarriers[i].srcAccess;
			vkImageBarriers[i].dstAccessMask = imageBarriers[i].dstAccess;
			vkImageBarriers[i].oldLayout = NeToVkImageLayout(imageBarriers[i].oldLayout);
			vkImageBarriers[i].newLayout = NeToVkImageLayout(imageBarriers[i].newLayout);
			vkImageBarriers[i].srcQueueFamilyIndex = _queueFamilyIndex(ctx->neDev, imageBarriers[i].srcQueue);
			vkImageBarriers[i].dstQueueFamilyIndex = _queueFamilyIndex(ctx->neDev, imageBarriers[i].dstQueue);
			vkImageBarriers[i].image = imageBarriers[i].texture->image;
			vkImageBarriers[i].subresourceRange.aspectMask = imageBarriers[i].subresource.aspect;
			vkImageBarriers[i].subresourceRange.baseMipLevel = imageBarriers[i].subresource.mipLevel;
			vkImageBarriers[i].subresourceRange.baseArrayLayer = imageBarriers[i].subresource.baseArrayLayer;
			vkImageBarriers[i].subresourceRange.layerCount = imageBarriers[i].subresource.layerCount;
			vkImageBarriers[i].subresourceRange.levelCount = imageBarriers[i].subresource.levelCount;
		}
	}

	vkCmdPipelineBarrier(ctx->cmdBuffer, srcStage, dstStage, dep, memBarrierCount, vkMemBarriers,
							bufferBarrierCount, vkBufferBarriers, imageBarrierCount, vkImageBarriers);*/
}

static void
_Transition(struct NeRenderContext *ctx, struct NeTexture *tex, enum NeTextureLayout newLayout)
{
//	Vkd_TransitionImageLayout(ctx->cmdBuffer, tex->image, tex->layout, NeToVkImageLayout(newLayout));
//	tex->layout = NeToVkImageLayout(newLayout);
}

static void
_CopyBuffer(struct NeRenderContext *ctx, const struct NeBuffer *src, uint64_t srcOffset, struct NeBuffer *dst, uint64_t dstOffset, uint64_t size)
{
/*	VkBufferCopy r =
	{
		.srcOffset = srcOffset,
		.dstOffset = dstOffset,
		.size = size
	};
	vkCmdCopyBuffer(ctx->cmdBuffer, src->buff, dst->buff, 1, &r);*/
}

static void
_CopyImage(struct NeRenderContext *ctx, const struct NeTexture *src, struct NeTexture *dst)
{
//	vkCmdCopyImage(ctx->cmdBuffer, src->image, src->layout, dst->image, dst->layout, 1, NULL);
}

static void
_CopyBufferToTexture(struct NeRenderContext *ctx, const struct NeBuffer *src, struct NeTexture *dst, const struct NeBufferImageCopy *bic)
{
/*	VkBufferImageCopy b =
	{
		.bufferOffset = bic->bufferOffset,
		.bufferRowLength = bic->rowLength,
		.bufferImageHeight = bic->imageHeight,
		.imageSubresource =
		{
			.aspectMask = bic->subresource.aspect,
			.mipLevel = bic->subresource.mipLevel,
			.baseArrayLayer = bic->subresource.baseArrayLayer,
			.layerCount = bic->subresource.layerCount
		},
		.imageOffset = { bic->imageOffset.x, bic->imageOffset.y, bic->imageOffset.z },
		.imageExtent = { bic->imageSize.width, bic->imageSize.height, bic->imageSize.depth }
	};
	VkImageSubresourceRange range =
	{
		.aspectMask = bic->subresource.aspect,
		.baseMipLevel = bic->subresource.mipLevel,
		.baseArrayLayer = bic->subresource.baseArrayLayer,
		.levelCount = 1,
		.layerCount = 1
	};

	Vkd_TransitionImageLayoutRange(ctx->cmdBuffer, dst->image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &range);
	vkCmdCopyBufferToImage(ctx->cmdBuffer, src->buff, dst->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &b);
	Vkd_TransitionImageLayoutRange(ctx->cmdBuffer, dst->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dst->layout, &range);*/
}

static void
_CopyTextureToBuffer(struct NeRenderContext *ctx, const struct NeTexture *src, struct NeBuffer *dst, const struct NeBufferImageCopy *bic)
{
/*	VkBufferImageCopy b =
	{
		.bufferOffset = bic->bufferOffset,
		.bufferRowLength = bic->rowLength,
		.bufferImageHeight = bic->imageHeight,
		.imageSubresource =
		{
			.aspectMask = bic->subresource.aspect,
			.mipLevel = bic->subresource.mipLevel,
			.baseArrayLayer = bic->subresource.baseArrayLayer,
			.layerCount = bic->subresource.layerCount
		},
		.imageOffset = { bic->imageOffset.x, bic->imageOffset.y, bic->imageOffset.z },
		.imageExtent = { bic->imageSize.width, bic->imageSize.height, bic->imageSize.depth }
	};
	vkCmdCopyImageToBuffer(ctx->cmdBuffer, src->image, src->layout, dst->buff, 1, &b);*/
}

static void
_Blit(struct NeRenderContext *ctx, const struct NeTexture *src, struct NeTexture *dst, const struct NeBlitRegion *regions, uint32_t regionCount, enum NeImageFilter filter)
{
/*	VkFilter f = VK_FILTER_NEAREST;

	switch (filter) {
	case IF_NEAREST: f = VK_FILTER_NEAREST; break;
	case IF_LINEAR: f = VK_FILTER_LINEAR; break;
	case IF_CUBIC: f = VK_FILTER_CUBIC_EXT; break;
	}

	VkImageBlit *r = Sys_Alloc(sizeof(*r), regionCount, MH_Transient);

	for (uint32_t i = 0; i < regionCount; ++i) {
		memcpy(&r[i].srcOffsets[0].x, &regions[i].srcOffset.x, sizeof(int32_t) * 6);
		memcpy(&r[i].dstOffsets[0].x, &regions[i].dstOffset.x, sizeof(int32_t) * 6);

		r[i].srcSubresource.aspectMask = regions[i].srcSubresource.aspect;
		r[i].srcSubresource.mipLevel = regions[i].srcSubresource.mipLevel;
		r[i].srcSubresource.layerCount = regions[i].srcSubresource.layerCount;
		r[i].srcSubresource.baseArrayLayer = regions[i].srcSubresource.baseArrayLayer;

		r[i].dstSubresource.aspectMask = regions[i].dstSubresource.aspect;
		r[i].dstSubresource.mipLevel = regions[i].dstSubresource.mipLevel;
		r[i].dstSubresource.layerCount = regions[i].dstSubresource.layerCount;
		r[i].dstSubresource.baseArrayLayer = regions[i].dstSubresource.baseArrayLayer;
	}

	vkCmdBlitImage(ctx->cmdBuffer, src->image, src->layout, dst->image, dst->layout, regionCount, r, f);*/
}

static bool
_QueueGraphics(struct NeRenderContext *ctx, NeCommandBufferHandle cmdBuffer, struct NeSemaphore *wait, struct NeSemaphore *signal)
{
/*	struct Vkd_SubmitInfo si =
	{
		.wait = wait ? wait->sem : VK_NULL_HANDLE,
		.waitValue = wait ? wait->value : 0,
		.signal = signal ? signal->sem : VK_NULL_HANDLE,
		.signalValue = signal ? ++signal->value : 0,
		.cmdBuffer = cmdBuffer
	};
	return Rt_ArrayAdd(&ctx->queued.graphics, &si);*/
	return true;
}

static bool
_QueueCompute(struct NeRenderContext *ctx, NeCommandBufferHandle cmdBuffer, struct NeSemaphore *wait, struct NeSemaphore *signal)
{
/*	struct Vkd_SubmitInfo si =
	{
		.wait = wait ? wait->sem : VK_NULL_HANDLE,
		.waitValue = wait ? wait->value : 0,
		.signal = signal ? signal->sem : VK_NULL_HANDLE,
		.signalValue = signal ? ++signal->value : 0,
		.cmdBuffer = cmdBuffer
	};
	return Rt_ArrayAdd(&ctx->queued.compute, &si);*/
	return true;
}

static bool
_QueueTransfer(struct NeRenderContext *ctx, NeCommandBufferHandle cmdBuffer, struct NeSemaphore *wait, struct NeSemaphore *signal)
{
/*	struct Vkd_SubmitInfo si =
	{
		.wait = wait ? wait->sem : VK_NULL_HANDLE,
		.waitValue = wait ? wait->value : 0,
		.signal = signal ? signal->sem : VK_NULL_HANDLE,
		.signalValue = signal ? ++signal->value : 0,
		.cmdBuffer = cmdBuffer
	};
	return Rt_ArrayAdd(&ctx->queued.xfer, &si);*/
	return true;
}

static bool _ExecuteGraphics(struct NeRenderContext *ctx, NeCommandBufferHandle cmdBuffer) { return true; }
static bool _ExecuteCompute(struct NeRenderContext *ctx, NeCommandBufferHandle cmdBuffer) { return true; }
static bool _ExecuteTransfer(struct NeRenderContext *ctx, NeCommandBufferHandle cmdBuffer) { return true; }

void
GL_InitContextProcs(struct NeRenderContextProcs *p)
{
	p->BeginSecondary = _BeginSecondary;
	p->BeginDrawCommandBuffer = _BeginDrawCommandBuffer;
	p->BeginComputeCommandBuffer = _BeginComputeCommandBuffer;
	p->BeginTransferCommandBuffer = _BeginTransferCommandBuffer;
	p->EndCommandBuffer = _EndCommandBuffer;
	p->BindPipeline = _BindPipeline;
	p->PushConstants = _PushConstants;
	p->BindIndexBuffer = _BindIndexBuffer;
	p->ExecuteSecondary = _ExecuteSecondary;
	p->BeginRenderPass = _BeginRenderPass;
	p->EndRenderPass = _EndRenderPass;
	p->SetViewport = _SetViewport;
	p->SetScissor = _SetScissor;
	p->Draw = _Draw;
	p->DrawIndexed = _DrawIndexed;
	p->DrawIndirect = _DrawIndirect;
	p->DrawIndexedIndirect = _DrawIndexedIndirect;
	p->Dispatch = _Dispatch;
	p->DispatchIndirect = _DispatchIndirect;
	p->TraceRays = _TraceRays;
	p->TraceRaysIndirect = _TraceRaysIndirect;
	p->BuildAccelerationStructures = _BuildAccelerationStructures;
	p->Barrier = _Barrier;
	p->Transition = _Transition;
	p->CopyBuffer = _CopyBuffer;
	p->CopyImage = _CopyImage;
	p->CopyBufferToTexture = _CopyBufferToTexture;
	p->CopyTextureToBuffer = _CopyTextureToBuffer;
	p->Blit = _Blit;
	p->QueueCompute = _QueueCompute;
	p->QueueGraphics = _QueueGraphics;
	p->QueueTransfer = _QueueTransfer;
	p->ExecuteCompute = _ExecuteCompute;
	p->ExecuteGraphics = _ExecuteGraphics;
	p->ExecuteTransfer = _ExecuteTransfer;
}
