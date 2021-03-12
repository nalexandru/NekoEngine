#include <assert.h>

#include <System/Memory.h>
#include <Render/Render.h>
#include <Runtime/Runtime.h>

#include "VulkanDriver.h"

struct SubmitInfo
{
	VkCommandBuffer cmdBuffer;
	VkQueue queue;
};

struct RenderContext *
Vk_CreateContext(struct RenderDevice *dev)
{
	struct RenderContext *ctx = calloc(1, sizeof(*ctx));
	if (!ctx)
		return NULL;

	VkCommandPoolCreateInfo poolInfo =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
	};

	struct Array *arrays = calloc(RE_NUM_FRAMES * 3, sizeof(*arrays));
	if (!arrays)
		return NULL;

	ctx->graphicsCmdBuffers = arrays;
	ctx->computeCmdBuffers = &arrays[RE_NUM_FRAMES];
	ctx->transferCmdBuffers = &arrays[RE_NUM_FRAMES * 2];

	VkCommandPool *pools = calloc(RE_NUM_FRAMES * 3, sizeof(*pools));
	if (!pools)
		return NULL;

	ctx->graphicsPools = pools;
	ctx->computePools = &pools[RE_NUM_FRAMES];
	ctx->transferPools = &pools[RE_NUM_FRAMES * 2];

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		poolInfo.queueFamilyIndex = dev->graphicsFamily;
		if (vkCreateCommandPool(dev->dev, &poolInfo, Vkd_allocCb, &ctx->graphicsPools[i]) != VK_SUCCESS)
			goto error;

		poolInfo.queueFamilyIndex = dev->computeFamily;
		if (vkCreateCommandPool(dev->dev, &poolInfo, Vkd_allocCb, &ctx->computePools[i]) != VK_SUCCESS)
			goto error;

		poolInfo.queueFamilyIndex = dev->transferFamily;
		if (vkCreateCommandPool(dev->dev, &poolInfo, Vkd_allocCb, &ctx->transferPools[i]) != VK_SUCCESS)
			goto error;

		if (!Rt_InitPtrArray(&ctx->graphicsCmdBuffers[i], 10))
			goto error;
	
		if (!Rt_InitPtrArray(&ctx->transferCmdBuffers[i], 10))
			goto error;
	
		if (!Rt_InitPtrArray(&ctx->computeCmdBuffers[i], 10))
			goto error;
	}

	VkFenceCreateInfo fci = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	vkCreateFence(dev->dev, &fci, Vkd_allocCb, &ctx->executeFence);

	ctx->dev = dev->dev;

	Rt_ArrayAddPtr(&Vkd_contexts, ctx);

	return ctx;

error:
	if (pools) {
		for (uint32_t i = 0; i < RE_NUM_FRAMES * 3; ++i)
			if (pools[i] != VK_NULL_HANDLE)
				vkDestroyCommandPool(dev->dev, pools[i], Vkd_allocCb);
	}

	free(pools);
	free(ctx);

	return NULL;
}

void
Vk_DestroyContext(struct RenderDevice *dev, struct RenderContext *ctx)
{
	vkDestroyFence(dev->dev, ctx->executeFence, Vkd_allocCb);

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		vkDestroyCommandPool(dev->dev, ctx->graphicsPools[i], Vkd_allocCb);
		vkDestroyCommandPool(dev->dev, ctx->computePools[i], Vkd_allocCb);
		vkDestroyCommandPool(dev->dev, ctx->transferPools[i], Vkd_allocCb);
		Rt_TermArray(&ctx->graphicsCmdBuffers[i]);
		Rt_TermArray(&ctx->transferCmdBuffers[i]);
		Rt_TermArray(&ctx->computeCmdBuffers[i]);
	}

	free(ctx->graphicsCmdBuffers);
	free(ctx->graphicsPools);
	free(ctx);
}

static void
_BeginDrawCommandBuffer(struct RenderContext *ctx)
{
	ctx->cmdBuffer = Vkd_AllocateCmdBuffer(ctx->dev, VK_COMMAND_BUFFER_LEVEL_PRIMARY, ctx->graphicsPools[Re_frameId], &ctx->graphicsCmdBuffers[Re_frameId]);
	assert(ctx->cmdBuffer);

	VkCommandBufferBeginInfo beginInfo =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	vkBeginCommandBuffer(ctx->cmdBuffer, &beginInfo);
}

static void
_BeginComputeCommandBuffer(struct RenderContext *ctx)
{
	ctx->cmdBuffer = Vkd_AllocateCmdBuffer(ctx->dev, VK_COMMAND_BUFFER_LEVEL_PRIMARY, ctx->computePools[Re_frameId], &ctx->computeCmdBuffers[Re_frameId]);
	assert(ctx->cmdBuffer);

	VkCommandBufferBeginInfo beginInfo =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	vkBeginCommandBuffer(ctx->cmdBuffer, &beginInfo);
}

static void
_BeginTransferCommandBuffer(struct RenderContext *ctx)
{
	ctx->cmdBuffer = Vkd_AllocateCmdBuffer(ctx->dev, VK_COMMAND_BUFFER_LEVEL_PRIMARY, ctx->transferPools[Re_frameId], &ctx->transferCmdBuffers[Re_frameId]);
	assert(ctx->cmdBuffer);

	VkCommandBufferBeginInfo beginInfo =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	vkBeginCommandBuffer(ctx->cmdBuffer, &beginInfo);
}

static void
_EndCommandBuffer(struct RenderContext *ctx)
{
	vkEndCommandBuffer(ctx->cmdBuffer);
	ctx->cmdBuffer = VK_NULL_HANDLE;
}

static void
_BindPipeline(struct RenderContext *ctx, struct Pipeline *pipeline)
{
	vkCmdBindPipeline(ctx->cmdBuffer, pipeline->bindPoint, pipeline->pipeline);
	ctx->boundPipeline = pipeline;
}

static void
_BindDescriptorSets(struct RenderContext *ctx, struct PipelineLayout *layout, uint32_t firstSet, uint32_t count, const struct DescriptorSet *sets)
{
	vkCmdBindDescriptorSets(ctx->cmdBuffer, ctx->boundPipeline->bindPoint, layout->layout, firstSet, count, (const VkDescriptorSet *)&sets, 0, NULL);
}

static void
_PushConstants(struct RenderContext *ctx, struct PipelineLayout *layout, enum ShaderStage stage, uint32_t size, const void *data)
{
	vkCmdPushConstants(ctx->cmdBuffer, layout->layout, stage, 0, size, data);
}

static void
_BindVertexBuffers(struct RenderContext *ctx, uint32_t count, struct Buffer **buffers, uint64_t *offsets)
{
	if (count == 1) {
		vkCmdBindVertexBuffers(ctx->cmdBuffer, 0, 1, &buffers[0]->buff, &offsets[0]);
	} else {
		VkBuffer *vb = Sys_Alloc(sizeof(*vb), count, MH_Transient);
		for (uint32_t i = 0; i < count; ++i)
			vb[i] = buffers[i]->buff;
		vkCmdBindVertexBuffers(ctx->cmdBuffer, 0, count, vb, offsets);
	}
}

static void
_BindIndexBuffer(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset, enum IndexType type)
{
	vkCmdBindIndexBuffer(ctx->cmdBuffer, buff->buff, offset, type == IT_UINT_32 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
}

static void
_ExecuteSecondary(struct RenderContext *ctx, struct RenderContext **contexts, uint32_t count)
{
	VkCommandBuffer *buffers = Sys_Alloc(sizeof(*buffers), count, MH_Transient);
	for (uint32_t i = 0; i < count; ++i)
		buffers[i] = contexts[i]->cmdBuffer;
	vkCmdExecuteCommands(ctx->cmdBuffer, count, buffers);
}

static void
_BeginRenderPass(struct RenderContext *ctx, struct RenderPass *pass, struct Framebuffer *fb)
{
	VkRenderPassAttachmentBeginInfo atbi = 
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO,
		.attachmentCount = fb->attachmentCount,
		.pAttachments = fb->attachments
	};

	VkClearValue value =
	{
		.color.float32 = { 1.f, 0.f, 0.f, 1.f }
	};
	VkRenderPassBeginInfo rpbi =
	{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = &atbi,
		.renderPass = pass->rp,
		.framebuffer = fb->fb,
		.renderArea = { { 0, 0 }, { fb->width, fb->height } },
		.clearValueCount = 1,
		.pClearValues = &value 
	};

	vkCmdBeginRenderPass(ctx->cmdBuffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
}

static void
_EndRenderPass(struct RenderContext *ctx)
{
	vkCmdEndRenderPass(ctx->cmdBuffer);
}

static void
_SetViewport(struct RenderContext *ctx, float x, float y, float width, float height, float minDepth, float maxDepth)
{
	VkViewport vp = { x, height + y, width, -height, minDepth, maxDepth };
	vkCmdSetViewport(ctx->cmdBuffer, 0, 1, &vp);
}

static void
_SetScissor(struct RenderContext *ctx, int32_t x, int32_t y, int32_t width, int32_t height)
{
	VkRect2D scissor = { { x, y }, { width, height } };
	vkCmdSetScissor(ctx->cmdBuffer, 0, 1, &scissor);
}

static void
_Draw(struct RenderContext *ctx, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	vkCmdDraw(ctx->cmdBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

static void
_DrawIndexed(struct RenderContext *ctx, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	vkCmdDrawIndexed(ctx->cmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

static void
_DrawIndirect(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	vkCmdDrawIndirect(ctx->cmdBuffer, buff->buff, offset, count, stride);
}

static void
_DrawIndexedIndirect(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	vkCmdDrawIndexedIndirect(ctx->cmdBuffer, buff->buff, offset, count, stride);
}

static void
_Dispatch(struct RenderContext *ctx, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	vkCmdDispatch(ctx->cmdBuffer, groupCountX, groupCountY, groupCountZ);
}

static void
_DispatchIndirect(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset)
{
	vkCmdDispatchIndirect(ctx->cmdBuffer, buff->buff, offset);
}

static void
_TraceRays(struct RenderContext *ctx, uint32_t width, uint32_t height, uint32_t depth)
{
//	vkCmdTraceRaysKHR(ctx->cmdBuffer, )
}

static void
_TraceRaysIndirect(struct RenderContext *ctx, struct Buffer *buff, uint64_t offset)
{
//	vkCmdTraceRaysIndirectKHR
}

static void
_BuildAccelerationStructure(struct RenderContext *ctx, struct AccelerationStructure *as, struct Buffer *scratch)
{
//	vkCmdBuildAccelerationStructuresKHR(ctx->cmdBuffer)
}

static void
_Barrier(struct RenderContext *ctx)
{
//	vkCmdPipelineBarrier
}

static void
_Transition(struct RenderContext *ctx, struct Texture *tex, enum TextureLayout newLayout)
{
	VkPipelineStageFlagBits src = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, dst = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkImageMemoryBarrier barrier =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.oldLayout = tex->layout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = tex->image,
		.subresourceRange =
		{
			.baseMipLevel = 0,
			.baseArrayLayer = 0,
			.levelCount = 1,
			.layerCount = 1
		}
	};

	switch (barrier.oldLayout) {
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		src = VK_PIPELINE_STAGE_HOST_BIT;
	break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		src = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		src = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		src = VK_PIPELINE_STAGE_TRANSFER_BIT;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		src = VK_PIPELINE_STAGE_TRANSFER_BIT;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		src = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	break;
	default:
		barrier.srcAccessMask = 0;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	break;
	}

	switch (newLayout) {
	case TL_COLOR_ATTACHMENT:
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dst = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	break;
	case TL_DEPTH_STENCIL_ATTACHMENT:
		barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dst = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	break;
	case TL_TRANSFER_SRC:
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		dst = VK_PIPELINE_STAGE_TRANSFER_BIT;
	break;
	case TL_TRANSFER_DST:
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		dst = VK_PIPELINE_STAGE_TRANSFER_BIT;
	break; 
	case TL_SHADER_READ_ONLY:
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		dst = VK_PIPELINE_STAGE_TRANSFER_BIT;
		break;
	default:
		barrier.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.dstAccessMask = 0;
	break;
	}

	vkCmdPipelineBarrier(ctx->cmdBuffer, src, dst, 0, 0, NULL, 0, NULL, 1, &barrier);
}

static void
_CopyBuffer(struct RenderContext *ctx, struct Buffer *dst, uint64_t dstOffset, struct Buffer *src, uint64_t srcOffset, uint64_t size)
{
	VkBufferCopy r =
	{
		.srcOffset = srcOffset,
		.dstOffset = dstOffset,
		.size = size
	};
	vkCmdCopyBuffer(ctx->cmdBuffer, src->buff, dst->buff, 1, &r);
}

static void
_CopyImage(struct RenderContext *ctx, struct Texture *dst, struct Texture *src)
{
	vkCmdCopyImage(ctx->cmdBuffer, src->image, src->layout, dst->image, dst->layout, 1, NULL);
}

static void
_CopyBufferToImage(struct RenderContext *ctx, struct Buffer *src, struct Texture *dst, const struct BufferImageCopy *bic)
{
	VkBufferImageCopy b =
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
	vkCmdCopyBufferToImage(ctx->cmdBuffer, src->buff, dst->image, dst->layout, 1, &b);
}

static void
_CopyImageToBuffer(struct RenderContext *ctx, struct Texture *src, struct Buffer *dst, const struct BufferImageCopy *bic)
{
	VkBufferImageCopy b =
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
	vkCmdCopyImageToBuffer(ctx->cmdBuffer, src->image, src->layout, dst->buff, 1, &b);
}

static void
_Blit(struct RenderContext *ctx, struct Texture *dst, struct Texture *src, const struct BlitRegion *regions, uint32_t regionCount, enum BlitFilter filter)
{
	VkFilter f = VK_FILTER_NEAREST;

	switch (filter) {
	case BF_NEAREST: f = VK_FILTER_NEAREST; break;
	case BF_LINEAR: f = VK_FILTER_LINEAR; break;
	case BF_CUBIC: f = VK_FILTER_CUBIC_EXT; break;
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

	vkCmdBlitImage(ctx->cmdBuffer, src->image, src->layout, dst->image, dst->layout, regionCount, r, f);
}

static bool
_Submit(struct RenderDevice *dev, struct RenderContext *ctx)
{
//	vkQueueSubmit(dev->graphicsQueue, 1, NULL, )
	return false;
}

void
Vk_InitContextProcs(struct RenderContextProcs *p)
{
	p->BeginDrawCommandBuffer = _BeginDrawCommandBuffer;
	p->BeginComputeCommandBuffer = _BeginComputeCommandBuffer;
	p->BeginTransferCommandBuffer = _BeginTransferCommandBuffer;
	p->EndCommandBuffer = _EndCommandBuffer;
	p->BindPipeline = _BindPipeline;
	p->BindDescriptorSets = _BindDescriptorSets;
	p->PushConstants = _PushConstants;
	p->BindVertexBuffers = _BindVertexBuffers;
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
	p->BuildAccelerationStructure = _BuildAccelerationStructure;
	p->Barrier = _Barrier;
	p->Transition = _Transition;
	p->CopyBuffer = _CopyBuffer;
	p->CopyImage = _CopyImage;
	p->CopyBufferToImage = _CopyBufferToImage;
	p->CopyImageToBuffer = _CopyImageToBuffer;
	p->Blit = _Blit;
	p->Submit = _Submit;
}

