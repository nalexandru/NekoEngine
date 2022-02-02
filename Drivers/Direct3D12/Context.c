#include <System/Memory.h>
#include <Runtime/Runtime.h>

#include "D3D12Driver.h"

struct NeRenderContext *
D3D12_CreateContext(struct NeRenderDevice *dev)
{
	struct NeRenderContext *ctx = Sys_Alloc(1, sizeof(*ctx), MH_RenderDriver);
	if (!ctx)
		return NULL;

	ID3D12CommandAllocator **allocators = Sys_Alloc(RE_NUM_FRAMES * 3, sizeof(*allocators), MH_RenderDriver);
	if (!allocators)
		return NULL;

	ctx->graphicsAllocators = allocators;
	ctx->computeAllocators = &allocators[RE_NUM_FRAMES];
	ctx->copyAllocators = &allocators[RE_NUM_FRAMES * 2];

	struct NeArray *arrays = Sys_Alloc(RE_NUM_FRAMES * 4, sizeof(*arrays), MH_RenderDriver);
	if (!arrays)
		return NULL;

	ctx->closedList.graphics = arrays;
	ctx->closedList.compute = &arrays[RE_NUM_FRAMES];
	ctx->closedList.copy = &arrays[RE_NUM_FRAMES * 2];
//	ctx->transferCmdBuffers = &arrays[RE_NUM_FRAMES * 3];

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		HRESULT hr;

		hr = ID3D12Device5_CreateCommandAllocator(dev->dev, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, &ctx->graphicsAllocators[i]);
		if (FAILED(hr))
			goto error;

		hr = ID3D12Device5_CreateCommandAllocator(dev->dev, D3D12_COMMAND_LIST_TYPE_COMPUTE, &IID_ID3D12CommandAllocator, &ctx->computeAllocators[i]);
		if (FAILED(hr))
			goto error;

		hr = ID3D12Device5_CreateCommandAllocator(dev->dev, D3D12_COMMAND_LIST_TYPE_COPY, &IID_ID3D12CommandAllocator, &ctx->copyAllocators[i]);
		if (FAILED(hr))
			goto error;

		if (!Rt_InitPtrArray(&ctx->closedList.graphics[i], 10, MH_RenderDriver))
			goto error;

		if (!Rt_InitPtrArray(&ctx->closedList.compute[i], 10, MH_RenderDriver))
			goto error;

		if (!Rt_InitPtrArray(&ctx->closedList.copy[i], 10, MH_RenderDriver))
			goto error;

		if (!Rt_InitPtrArray(&ctx->freeList.graphics[i], 10, MH_RenderDriver))
			goto error;

		if (!Rt_InitPtrArray(&ctx->freeList.compute[i], 10, MH_RenderDriver))
			goto error;

		if (!Rt_InitPtrArray(&ctx->freeList.copy[i], 10, MH_RenderDriver))
			goto error;

	//	if (!Rt_InitPtrArray(&ctx->secondaryCmdBuffers[i], 10, MH_RenderDriver))
	//		goto error;
	}

	D3D12Drv_InitFence(dev->dev, &ctx->executeFence, false);

	return ctx;

error:
	if (allocators) {
		for (uint32_t i = 0; i < RE_NUM_FRAMES * 3; ++i)
			if (allocators[i])
				ID3D12CommandAllocator_Release(allocators[i]);
	}

	Sys_Free(arrays);
	Sys_Free(allocators);
	Sys_Free(ctx);

	return NULL;
}

void
D3D12_ResetContext(struct NeRenderDevice *dev, struct NeRenderContext *ctx)
{
	ID3D12GraphicsCommandList4 *cmdList;

	ID3D12CommandAllocator_Reset(ctx->graphicsAllocators[Re_frameId]);
	ID3D12CommandAllocator_Reset(ctx->computeAllocators[Re_frameId]);
	ID3D12CommandAllocator_Reset(ctx->copyAllocators[Re_frameId]);

	Rt_ClearArray(&ctx->freeList.graphics[Re_frameId], false);
	Rt_ArrayForEachPtr(cmdList, &ctx->closedList.graphics[Re_frameId]) {
		ID3D12GraphicsCommandList4_Reset(cmdList, ctx->graphicsAllocators[Re_frameId], NULL);
		Rt_ArrayAdd(&ctx->freeList.graphics[Re_frameId], cmdList);
	}
	Rt_ClearArray(&ctx->closedList.graphics[Re_frameId], false);

	Rt_ClearArray(&ctx->freeList.compute[Re_frameId], false);
	Rt_ArrayForEachPtr(cmdList, &ctx->closedList.compute[Re_frameId]) {
		ID3D12GraphicsCommandList4_Reset(cmdList, ctx->computeAllocators[Re_frameId], NULL);
		Rt_ArrayAdd(&ctx->freeList.compute[Re_frameId], cmdList);
	}
	Rt_ClearArray(&ctx->closedList.compute[Re_frameId], false);

	Rt_ClearArray(&ctx->freeList.copy[Re_frameId], false);
	Rt_ArrayForEachPtr(cmdList, &ctx->closedList.copy[Re_frameId]) {
		ID3D12GraphicsCommandList4_Reset(cmdList, ctx->copyAllocators[Re_frameId], NULL);
		Rt_ArrayAdd(&ctx->freeList.copy[Re_frameId], cmdList);
	}
	Rt_ClearArray(&ctx->closedList.copy[Re_frameId], false);
}

void
D3D12_DestroyContext(struct NeRenderDevice *dev, struct NeRenderContext *ctx)
{
	D3D12Drv_TermFence(&ctx->executeFence);

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		ID3D12CommandAllocator_Release(ctx->graphicsAllocators[i]);
		ID3D12CommandAllocator_Release(ctx->computeAllocators[i]);
		ID3D12CommandAllocator_Release(ctx->copyAllocators[i]);
		Rt_TermArray(&ctx->closedList.graphics[i]);
		Rt_TermArray(&ctx->closedList.compute[i]);
		Rt_TermArray(&ctx->closedList.copy[i]);
		Rt_TermArray(&ctx->freeList.graphics[i]);
		Rt_TermArray(&ctx->freeList.compute[i]);
		Rt_TermArray(&ctx->freeList.copy[i]);
		//Rt_TermArray(&ctx->computeCmdBuffers[i]);
	}

	Sys_Free(ctx->closedList.graphics);
	Sys_Free(ctx->graphicsAllocators);

	Sys_Free(ctx);
}

static NeCommandBufferHandle
_BeginSecondary(struct NeRenderContext *ctx, struct NeRenderPassDesc *passDesc)
{
/*	ctx->cmdBuffer = Vkd_AllocateCmdBuffer(ctx->dev, VK_COMMAND_BUFFER_LEVEL_SECONDARY,
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
	vkBeginCommandBuffer(ctx->cmdBuffer, &beginInfo);

	return ctx->cmdBuffer;*/
	return NULL;
}

static void
_BeginDrawCommandBuffer(struct NeRenderContext *ctx)
{
	if (ctx->freeList.graphics[Re_frameId].count) {
		ctx->cmdList = Rt_ArrayGetPtr(&ctx->freeList.graphics[Re_frameId], --ctx->freeList.graphics[Re_frameId].count);
	} else {
		ID3D12GraphicsCommandList *list;
		ID3D12Device5_CreateCommandList(ctx->dev, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, ctx->graphicsAllocators[Re_frameId], NULL, &IID_ID3D12GraphicsCommandList, &list);
		ID3D12GraphicsCommandList_QueryInterface(list, &IID_ID3D12GraphicsCommandList4, &ctx->cmdList);
	}
}

static void
_BeginComputeCommandBuffer(struct NeRenderContext *ctx)
{
	if (ctx->freeList.compute[Re_frameId].count) {
		ctx->cmdList = Rt_ArrayGetPtr(&ctx->freeList.compute[Re_frameId], --ctx->freeList.compute[Re_frameId].count);
	} else {
		ID3D12GraphicsCommandList *list;
		ID3D12Device5_CreateCommandList(ctx->dev, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, ctx->graphicsAllocators[Re_frameId], NULL, &IID_ID3D12GraphicsCommandList, &list);
		ID3D12GraphicsCommandList_QueryInterface(list, &IID_ID3D12GraphicsCommandList4, &ctx->cmdList);
	}
}

static void
_BeginTransferCommandBuffer(struct NeRenderContext *ctx)
{
	if (ctx->freeList.copy[Re_frameId].count) {
		ctx->cmdList = Rt_ArrayGetPtr(&ctx->freeList.copy[Re_frameId], --ctx->freeList.copy[Re_frameId].count);
	} else {
		ID3D12GraphicsCommandList *list;
		ID3D12Device5_CreateCommandList(ctx->dev, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, ctx->graphicsAllocators[Re_frameId], NULL, &IID_ID3D12GraphicsCommandList, &list);
		ID3D12GraphicsCommandList_QueryInterface(list, &IID_ID3D12GraphicsCommandList4, &ctx->cmdList);
	}
}

static NeCommandBufferHandle
_EndCommandBuffer(struct NeRenderContext *ctx)
{
	NeCommandBufferHandle cb = ctx->cmdList;
	ID3D12GraphicsCommandList4_Close(ctx->cmdList);
	ctx->cmdList = NULL;
	return cb;
}

static void
_BindPipeline(struct NeRenderContext *ctx, struct NePipeline *pipeline)
{
	ID3D12DescriptorHeap *heaps[] = { ctx->neDev->descriptorHeap[Re_frameId] };
	ID3D12GraphicsCommandList4_SetDescriptorHeaps(ctx->cmdList, _countof(heaps), heaps);

	ID3D12GraphicsCommandList4_SetGraphicsRootSignature(ctx->cmdList, pipeline->rs);

	ID3D12GraphicsCommandList4_IASetPrimitiveTopology(ctx->cmdList, pipeline->topology);
	ID3D12GraphicsCommandList4_SetPipelineState(ctx->cmdList, pipeline->ps);

//	ctx->boundPipeline = pipeline;
}

static void
_PushConstants(struct NeRenderContext *ctx, enum NeShaderStage stage, uint32_t size, const void *data)
{
	// ?
//	vkCmdPushConstants(ctx->cmdBuffer, ctx->boundPipeline->layout, stage, 0, size, data);
}

static void
_BindIndexBuffer(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset, enum NeIndexType type)
{
	D3D12_INDEX_BUFFER_VIEW view =
	{
		.BufferLocation = ID3D12Resource1_GetGPUVirtualAddress(buff->res) + offset,
		.SizeInBytes = (UINT)(buff->size - offset),
		.Format = type == IT_UINT_16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT
	};
	ID3D12GraphicsCommandList4_IASetIndexBuffer(ctx->cmdList, &view);
}

static void
_ExecuteSecondary(struct NeRenderContext *ctx, NeCommandBufferHandle *cmdBuffers, uint32_t count)
{
//	vkCmdExecuteCommands(ctx->cmdBuffer, count, (VkCommandBuffer *)cmdBuffers);
}

static void
_BeginRenderPass(struct NeRenderContext *ctx, struct NeRenderPassDesc *passDesc, struct NeFramebuffer *fb, enum NeRenderCommandContents contents)
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = { 0 };
	ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(ctx->neDev->rtvHeap, NULL);

	uint32_t barrierCount = 0;
	D3D12_RESOURCE_BARRIER barriers[9];

	D3D12_CPU_DESCRIPTOR_HANDLE handles[8];
	for (uint32_t i = 0; i < passDesc->attachmentCount; ++i) {
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc =
		{
			.Format = passDesc->rtvFormats[i],
			.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D
		};
		ID3D12Device5_CreateRenderTargetView(ctx->dev, fb->attachments[i], &rtvDesc, rtvHandle);

		if (passDesc->loadOp[i] == ATL_CLEAR)
			ID3D12GraphicsCommandList4_ClearRenderTargetView(ctx->cmdList, rtvHandle, &passDesc->clearValues[i].x, 0, NULL);

		handles[i] = rtvHandle;
		rtvHandle.ptr += ctx->neDev->rtvHeapIncrement;

		D3D12_RESOURCE_BARRIER *barrier = &barriers[barrierCount++];
		barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier->Transition.pResource = fb->attachments[i];
		barrier->Transition.StateBefore = passDesc->rtvInitialState[i];
		barrier->Transition.StateAfter = passDesc->rtvState[i];
		barrier->Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE *dsvHandle = NULL;
	if (passDesc->depthFormat != DXGI_FORMAT_UNKNOWN) {
		dsvHandle = &rtvHandle;

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc =
		{
			.Format = passDesc->depthFormat,
			.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
			.Flags = 0 // TODO: read only depth
		};
		ID3D12Device5_CreateDepthStencilView(ctx->dev, fb->depthAttachment, &dsvDesc, *dsvHandle);

		if (passDesc->depthLoadOp == ATL_CLEAR)
			ID3D12GraphicsCommandList4_ClearDepthStencilView(ctx->cmdList, *dsvHandle, D3D12_CLEAR_FLAG_DEPTH, passDesc->depthClearValue, 0, 0, NULL);

		D3D12_RESOURCE_BARRIER *barrier = &barriers[barrierCount++];
		barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier->Transition.pResource = fb->depthAttachment;
		barrier->Transition.StateBefore = passDesc->depthInitialState;
		barrier->Transition.StateAfter = passDesc->depthState;
		barrier->Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	}

	ID3D12GraphicsCommandList4_ResourceBarrier(ctx->cmdList, barrierCount, barriers);

	ID3D12DescriptorHeap_GetCPUDescriptorHandleForHeapStart(ctx->neDev->rtvHeap, NULL);
	ID3D12GraphicsCommandList4_OMSetRenderTargets(ctx->cmdList, passDesc->attachmentCount, handles, TRUE, dsvHandle);

	ctx->boundRenderPass = passDesc;
	ctx->boundFramebuffer = fb;
}

static void
_EndRenderPass(struct NeRenderContext *ctx)
{
	uint32_t barrierCount = 0;
	D3D12_RESOURCE_BARRIER barriers[9];

	for (uint32_t i = 0; i < ctx->boundRenderPass->attachmentCount; ++i) {
		D3D12_RESOURCE_BARRIER *barrier = &barriers[barrierCount++];
		barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier->Transition.pResource = ctx->boundFramebuffer->attachments[i];
		barrier->Transition.StateBefore = ctx->boundRenderPass->rtvState[i];
		barrier->Transition.StateAfter = ctx->boundRenderPass->rtvFinalState[i];
		barrier->Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	}

	if (ctx->boundRenderPass->depthFormat != DXGI_FORMAT_UNKNOWN) {
		D3D12_RESOURCE_BARRIER *barrier = &barriers[barrierCount++];
		barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier->Transition.pResource = ctx->boundFramebuffer->depthAttachment;
		barrier->Transition.StateBefore = ctx->boundRenderPass->depthState;
		barrier->Transition.StateAfter = ctx->boundRenderPass->depthFinalState;
		barrier->Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	}

	ID3D12GraphicsCommandList4_ResourceBarrier(ctx->cmdList, barrierCount, barriers);

	ctx->boundRenderPass = NULL;
	ctx->boundFramebuffer = NULL;
}

static void
_SetViewport(struct NeRenderContext *ctx, float x, float y, float width, float height, float minDepth, float maxDepth)
{
	D3D12_VIEWPORT vp = { x, height + y, width, -height, minDepth, maxDepth };
	ID3D12GraphicsCommandList4_RSSetViewports(ctx->cmdList, 1, &vp);
}

static void
_SetScissor(struct NeRenderContext *ctx, int32_t x, int32_t y, int32_t width, int32_t height)
{
	D3D12_RECT scissor = { x, y, width, height };
	ID3D12GraphicsCommandList4_RSSetScissorRects(ctx->cmdList, 1, &scissor);
}

static void
_Draw(struct NeRenderContext *ctx, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	ID3D12GraphicsCommandList4_DrawInstanced(ctx->cmdList, vertexCount, instanceCount, firstVertex, firstInstance);
}

static void
_DrawIndexed(struct NeRenderContext *ctx, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	ID3D12GraphicsCommandList4_DrawIndexedInstanced(ctx->cmdList, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

static void
_DrawIndirect(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
//	vkCmdDrawIndirect(ctx->cmdBuffer, buff->buff, offset, count, stride);
}

static void
_DrawIndexedIndirect(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
//	vkCmdDrawIndexedIndirect(ctx->cmdBuffer, buff->buff, offset, count, stride);
}

static void
_Dispatch(struct NeRenderContext *ctx, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	ID3D12GraphicsCommandList4_Dispatch(ctx->cmdList, groupCountX, groupCountY, groupCountZ);
}

static void
_DispatchIndirect(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset)
{
//	vkCmdDispatchIndirect(ctx->cmdBuffer, buff->buff, offset);
}

static void
_TraceRays(struct NeRenderContext *ctx, uint32_t width, uint32_t height, uint32_t depth)
{
	D3D12_DISPATCH_RAYS_DESC desc =
	{
//D3D12_GPU_VIRTUAL_ADDRESS_RANGE RayGenerationShaderRecord;
//D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE MissShaderTable;
//D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE HitGroupTable;
//D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE CallableShaderTable;
		.Width = width,
		.Height = height,
		.Depth = depth
	};
	ID3D12GraphicsCommandList4_DispatchRays(ctx->cmdList, &desc);
}

static void
_TraceRaysIndirect(struct NeRenderContext *ctx, struct NeBuffer *buff, uint64_t offset)
{
//	vkCmdTraceRaysIndirectKHR
}

static void
_BuildAccelerationStructures(struct NeRenderContext *ctx, uint32_t count, struct NeAccelerationStructureBuildInfo *buildInfo, const struct NeAccelerationStructureRangeInfo **rangeInfo)
{
/*	VkAccelerationStructureBuildGeometryInfoKHR *geomInfo = Sys_Alloc(count, sizeof(*geomInfo), MH_Transient);

	for (uint32_t i = 0; i < count; ++i) {
		const struct NeAccelerationStructureBuildInfo *src = &buildInfo[i];
		VkAccelerationStructureBuildGeometryInfoKHR *dst = &geomInfo[i];

		dst->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		dst->type = src->type;
		dst->flags = src->flags;
		dst->mode = src->mode;
		dst->srcAccelerationStructure = src->src ? src->src->as : VK_NULL_HANDLE;
		dst->dstAccelerationStructure = src->dst ? src->dst->as : VK_NULL_HANDLE;
		dst->geometryCount = src->geometryCount;
		dst->scratchData.deviceAddress = src->scratchAddress;

		VkAccelerationStructureGeometryKHR *geom = Sys_Alloc(src->geometryCount, sizeof(*geom), MH_Transient);
		dst->pGeometries = geom;

		for (uint32_t j = 0; j < src->geometryCount; ++j) {
			const struct NeAccelerationStructureGeometryDesc *srcGeom = &src->geometries[j];
			VkAccelerationStructureGeometryKHR *dstGeom = &geom[j];

			dstGeom->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
			dstGeom->geometryType = srcGeom->type;

			if (srcGeom->type == ASG_TRIANGLES) {
				dstGeom->geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
				dstGeom->geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT; // FIXME
				dstGeom->geometry.triangles.vertexData.deviceAddress = srcGeom->triangles.vertexBufferAddress;
				dstGeom->geometry.triangles.vertexStride = srcGeom->triangles.stride;
				dstGeom->geometry.triangles.maxVertex = srcGeom->triangles.vertexCount;
				dstGeom->geometry.triangles.indexType = srcGeom->triangles.indexType;
				dstGeom->geometry.triangles.indexData.deviceAddress = srcGeom->triangles.indexBufferAddress;
				dstGeom->geometry.triangles.transformData.deviceAddress = srcGeom->triangles.transformBufferAddress;
			} else if (srcGeom->type == ASG_INSTANCES) {
				dstGeom->geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
				dstGeom->geometry.instances.arrayOfPointers = VK_FALSE;
				dstGeom->geometry.instances.data.deviceAddress = srcGeom->instances.address;
			} else if (srcGeom->type == ASG_AABBS) {
				dstGeom->geometry.aabbs.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
				dstGeom->geometry.aabbs.stride = srcGeom->aabbs.stride;
				dstGeom->geometry.aabbs.data.deviceAddress = srcGeom->aabbs.address;
			}
		}
	}

	vkCmdBuildAccelerationStructuresKHR(ctx->cmdBuffer, count, geomInfo, (const VkAccelerationStructureBuildRangeInfoKHR * const *)rangeInfo);*/
}

static void
_Barrier(struct NeRenderContext *ctx, enum NePipelineStage srcStage, enum NePipelineStage dstStage, enum NePipelineDependency dep,
	uint32_t memBarrierCount, const struct NeMemoryBarrier *memBarriers, uint32_t bufferBarrierCount, const struct NeBufferBarrier *bufferBarriers,
	uint32_t imageBarrierCount, const struct NeImageBarrier *imageBarriers)
{
//	vkCmdPipelineBarrier
}

static void
_Transition(struct NeRenderContext *ctx, struct NeTexture *tex, enum NeTextureLayout newLayout)
{
/*	Vkd_TransitionImageLayout(ctx->cmdBuffer, tex->image, tex->layout, NeToVkImageLayout(newLayout));
	tex->layout = NeToVkImageLayout(newLayout);*/
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
	vkCmdCopyBufferToImage(ctx->cmdBuffer, src->buff, dst->image, dst->layout, 1, &b);*/
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
_QueueGraphics(struct NeRenderContext *ctx, ID3D12GraphicsCommandList4 *cmdBuffer, struct NeSemaphore *wait, struct NeSemaphore *signal)
{
/*	struct Vkd_SubmitInfo si =
	{
		.wait = wait ? wait->sem : VK_NULL_HANDLE,
		.waitValue = wait ? wait->value : 0,
		.signal = signal ? signal->sem : VK_NULL_HANDLE,
		.signalValue = signal ? ++signal->value : 0,
		.cmdBuffer = cmdBuffer
	};*/
	return Rt_ArrayAddPtr(&ctx->closedList.graphics[Re_frameId], &cmdBuffer);
}

static bool
_QueueCompute(struct NeRenderContext *ctx, ID3D12GraphicsCommandList4 *cmdBuffer, struct NeSemaphore *wait, struct NeSemaphore *signal)
{
	return false;
}

static bool
_QueueTransfer(struct NeRenderContext *ctx, ID3D12GraphicsCommandList4 *cmdBuffer, struct NeSemaphore *wait, struct NeSemaphore *signal)
{
	return false;
}

static bool
_ExecuteGraphics(struct NeRenderContext *ctx, ID3D12GraphicsCommandList4 *cmdBuffer)
{
	ID3D12CommandQueue_ExecuteCommandLists(ctx->neDev->graphicsQueue, 1, (ID3D12CommandList **)&cmdBuffer);
	Rt_ArrayAddPtr(&ctx->closedList.graphics[Re_frameId], &cmdBuffer);

	++ctx->executeFence.value;
	ID3D12CommandQueue_Signal(ctx->neDev->graphicsQueue, ctx->executeFence.fence, ctx->executeFence.value);

	DWORD rc = WAIT_FAILED;
	if (ID3D12Fence_GetCompletedValue(ctx->executeFence.fence) < ctx->executeFence.value) {
		HANDLE evt = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (!evt)
			return false;

		ID3D12Fence_SetEventOnCompletion(ctx->executeFence.fence, ctx->executeFence.value, evt);
		rc = WaitForSingleObject(evt, INFINITE);
		CloseHandle(evt);
	}

	return rc == WAIT_OBJECT_0;
}

static bool
_ExecuteCompute(struct NeRenderContext *ctx, ID3D12GraphicsCommandList4 *cmdBuffer)
{
	ID3D12CommandQueue_ExecuteCommandLists(ctx->neDev->computeQueue, 1, (ID3D12CommandList **)&cmdBuffer);
	Rt_ArrayAddPtr(&ctx->closedList.compute[Re_frameId], &cmdBuffer);

	++ctx->executeFence.value;
	ID3D12CommandQueue_Signal(ctx->neDev->computeQueue, ctx->executeFence.fence, ctx->executeFence.value);

	DWORD rc = WAIT_FAILED;
	if (ID3D12Fence_GetCompletedValue(ctx->executeFence.fence) < ctx->executeFence.value) {
		HANDLE evt = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (!evt)
			return false;

		ID3D12Fence_SetEventOnCompletion(ctx->executeFence.fence, ctx->executeFence.value, evt);
		rc = WaitForSingleObject(evt, INFINITE);
		CloseHandle(evt);
	}

	return rc == WAIT_OBJECT_0;
}

static bool
_ExecuteTransfer(struct NeRenderContext *ctx, ID3D12GraphicsCommandList4 *cmdBuffer)
{
	ID3D12CommandQueue_ExecuteCommandLists(ctx->neDev->copyQueue, 1, (ID3D12CommandList **)&cmdBuffer);
	Rt_ArrayAddPtr(&ctx->closedList.copy[Re_frameId], &cmdBuffer);

	++ctx->executeFence.value;
	ID3D12CommandQueue_Signal(ctx->neDev->copyQueue, ctx->executeFence.fence, ctx->executeFence.value);

	DWORD rc = WAIT_FAILED;
	if (ID3D12Fence_GetCompletedValue(ctx->executeFence.fence) < ctx->executeFence.value) {
		HANDLE evt = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (!evt)
			return false;

		ID3D12Fence_SetEventOnCompletion(ctx->executeFence.fence, ctx->executeFence.value, evt);
		rc = WaitForSingleObject(evt, INFINITE);
		CloseHandle(evt);
	}

	return rc == WAIT_OBJECT_0;
}

void
D3D12_InitContextProcs(struct NeRenderContextProcs *p)
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
