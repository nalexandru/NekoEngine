#include <assert.h>

#include <System/Memory.h>
#include <Runtime/Runtime.h>

#include "D3D12Backend.h"

static inline void Submit(struct VkdRenderQueue *queue, struct NeArray *submitInfo,
							uint32_t waitCount, NeSemaphore *wait,
							uint32_t signalCount, NeSemaphore *signal);

struct NeRenderContext *
Re_CreateContext(void)
{
	NeRenderContext *ctx = (NeRenderContext *)Sys_Alloc(1, sizeof(*ctx), MH_RenderBackend);
	if (!ctx)
		return NULL;

	ID3D12CommandAllocator **allocators = NULL;
	NeArray *arrays = (NeArray *)Sys_Alloc(RE_NUM_FRAMES * 3 * 2, sizeof(*arrays), MH_RenderBackend);
	if (!arrays)
		goto error;

	ctx->closed.gfx = arrays;
	ctx->closed.comp = &arrays[RE_NUM_FRAMES];
	ctx->closed.copy = &arrays[RE_NUM_FRAMES * 2];

	ctx->free.gfx = &arrays[RE_NUM_FRAMES * 3];
	ctx->free.comp = &arrays[RE_NUM_FRAMES * 4];
	ctx->free.copy = &arrays[RE_NUM_FRAMES * 5];

	allocators = (ID3D12CommandAllocator **)Sys_Alloc(RE_NUM_FRAMES * 3, sizeof(*allocators), MH_RenderBackend);
	if (!allocators)
		goto error;

	ctx->gfxAllocators = allocators;
	ctx->compAllocators = &allocators[RE_NUM_FRAMES];
	ctx->copyAllocators = &allocators[RE_NUM_FRAMES * 2];

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		if (FAILED(Re_device->dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&ctx->gfxAllocators[i]))))
			goto error;
		if (FAILED(Re_device->dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&ctx->compAllocators[i]))))
			goto error;
		if (FAILED(Re_device->dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&ctx->copyAllocators[i]))))
			goto error;

		if (!Rt_InitPtrArray(&ctx->closed.gfx[i], 10, MH_RenderBackend))
			goto error;
		if (!Rt_InitPtrArray(&ctx->closed.comp[i], 10, MH_RenderBackend))
			goto error;
		if (!Rt_InitPtrArray(&ctx->closed.copy[i], 10, MH_RenderBackend))
			goto error;

		if (!Rt_InitPtrArray(&ctx->free.gfx[i], 10, MH_RenderBackend))
			goto error;
		if (!Rt_InitPtrArray(&ctx->free.comp[i], 10, MH_RenderBackend))
			goto error;
		if (!Rt_InitPtrArray(&ctx->free.copy[i], 10, MH_RenderBackend))
			goto error;

#ifdef _DEBUG
		/*char name[64];
		snprintf(name, sizeof(name), "Graphics Pool %u", Re_frameId);
		Vkd_SetObjectName(Re_device->dev, ctx->graphicsPools[i], VK_OBJECT_TYPE_COMMAND_POOL, name);

		snprintf(name, sizeof(name), "Compute Pool %u", Re_frameId);
		Vkd_SetObjectName(Re_device->dev, ctx->computePools[i], VK_OBJECT_TYPE_COMMAND_POOL, name);

		snprintf(name, sizeof(name), "Transfer Pool %u", Re_frameId);
		Vkd_SetObjectName(Re_device->dev, ctx->xferPools[i], VK_OBJECT_TYPE_COMMAND_POOL, name);*/
#endif
	}

//	VkFenceCreateInfo fci = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
//	vkCreateFence(Re_device->dev, &fci, Vkd_allocCb, &ctx->executeFence);

	ctx->d3dDev = Re_device->dev;
	ctx->neDev = Re_device;
	//ctx->descriptorSet = Re_device->descriptorSet;

	if (!Rt_InitArray(&ctx->queued.graphics, 10, sizeof(struct D3D12_SubmitInfo), MH_RenderBackend) ||
		!Rt_InitArray(&ctx->queued.compute, 10, sizeof(struct D3D12_SubmitInfo), MH_RenderBackend) ||
		!Rt_InitArray(&ctx->queued.copy, 10, sizeof(struct D3D12_SubmitInfo), MH_RenderBackend))
		goto error;

	return ctx;

error:
/*	if (pools) {
		for (uint32_t i = 0; i < RE_NUM_FRAMES * 3; ++i)
			if (pools[i] != VK_NULL_HANDLE)
				vkDestroyCommandPool(Re_device->dev, pools[i], Vkd_allocCb);
	}*/

	Sys_Free(arrays);
	Sys_Free(allocators);
	Sys_Free(ctx);

	return NULL;
}

void
Re_ResetContext(struct NeRenderContext *ctx)
{
	ID3D12GraphicsCommandList7 *cmdList;

	ctx->gfxAllocators[Re_frameId]->Reset();
	ctx->compAllocators[Re_frameId]->Reset();
	ctx->copyAllocators[Re_frameId]->Reset();

	Rt_ClearArray(&ctx->free.gfx[Re_frameId], false);
	Rt_ArrayForEachPtr(cmdList, &ctx->closed.gfx[Re_frameId], ID3D12GraphicsCommandList7 *) {
		cmdList->Reset(ctx->gfxAllocators[Re_frameId], NULL);
		Rt_ArrayAdd(&ctx->free.gfx[Re_frameId], cmdList);
	}
	Rt_ClearArray(&ctx->closed.gfx[Re_frameId], false);

	Rt_ClearArray(&ctx->free.comp[Re_frameId], false);
	Rt_ArrayForEachPtr(cmdList, &ctx->closed.comp[Re_frameId], ID3D12GraphicsCommandList7 *) {
		cmdList->Reset(ctx->compAllocators[Re_frameId], NULL);
		Rt_ArrayAdd(&ctx->free.comp[Re_frameId], cmdList);
	}
	Rt_ClearArray(&ctx->closed.comp[Re_frameId], false);

	Rt_ClearArray(&ctx->free.copy[Re_frameId], false);
	Rt_ArrayForEachPtr(cmdList, &ctx->closed.copy[Re_frameId], ID3D12GraphicsCommandList7 *) {
		cmdList->Reset(ctx->copyAllocators[Re_frameId], NULL);
		Rt_ArrayAdd(&ctx->free.copy[Re_frameId], cmdList);
	}
	Rt_ClearArray(&ctx->closed.copy[Re_frameId], false);

	ctx->lastSubmittedXfer = 0;
	ctx->lastSubmittedCompute = 0;

	Rt_ClearArray(&ctx->queued.graphics, false);
	Rt_ClearArray(&ctx->queued.compute, false);
	Rt_ClearArray(&ctx->queued.copy, false);
}

void
Re_DestroyContext(struct NeRenderContext *ctx)
{
	//D3D12Drv_TermFence(&ctx->executeFence);

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		ctx->gfxAllocators[i]->Release();
		ctx->compAllocators[i]->Release();
		ctx->copyAllocators[i]->Release();

		Rt_TermArray(&ctx->closed.gfx[i]);
		Rt_TermArray(&ctx->closed.comp[i]);
		Rt_TermArray(&ctx->closed.copy[i]);

		Rt_TermArray(&ctx->free.gfx[i]);
		Rt_TermArray(&ctx->free.comp[i]);
		Rt_TermArray(&ctx->free.copy[i]);
	}

	Sys_Free(ctx->closed.gfx);
	Sys_Free(ctx->gfxAllocators);

	Sys_Free(ctx);
}

void
Vkd_ExecuteCommands(struct NeRenderDevice *dev, struct NeRenderContext *ctx, struct NeSwapchain *sw, struct NeSemaphore *waitSemaphore)
{
	/*dev->frameValues[Re_frameId] = ++dev->semaphoreValue;

	uint32_t waitCount = 0;
	VkSemaphoreSubmitInfoKHR waitInfo[] =
		{
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR,
				.stageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR,
				.deviceIndex = 0
			},
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR,
				.stageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR,
				.deviceIndex = 0
			},
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR,
				.stageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR,
				.deviceIndex = 0
			},
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR,
				.stageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR,
				.deviceIndex = 0
			}
		};
	VkSemaphoreSubmitInfoKHR signalInfo[] =
		{
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR,
				.semaphore = dev->frameSemaphore,
				.value = dev->frameValues[Re_frameId],
				.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT_KHR,
				.deviceIndex = 0
			},
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR,
				.semaphore = sw->frameEnd[Re_frameId],
				.value = 0,
				.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT_KHR,
				.deviceIndex = 0
			}
		};

#define ADD_WAIT(s, v) 					\
	waitInfo[waitCount].semaphore = s;	\
	waitInfo[waitCount].value = v;		\
	++waitCount

	if (!Re_deviceInfo.features.coherentMemory) {
		ADD_WAIT(Vkd_stagingSignal.sem, Vkd_stagingSignal.value);
	}

	if (waitSemaphore) {
		ADD_WAIT(waitSemaphore->sem, waitSemaphore->value);
	}

	if (ctx->queued.xfer.count) {
		Submit(&dev->transfer, &ctx->queued.xfer, waitCount, waitInfo, 1, signalInfo);
		ADD_WAIT(dev->frameSemaphore, dev->frameValues[Re_frameId]);

		dev->frameValues[Re_frameId] = ++dev->semaphoreValue;
		signalInfo[0].value = dev->frameValues[Re_frameId];
	}

	Submit(&dev->compute, &ctx->queued.compute, waitCount, waitInfo, 0, NULL);

	if (Re_deviceInfo.features.coherentMemory) {
		ADD_WAIT(sw->frameStart[Re_frameId], 0);
	}

	Submit(&dev->graphics, &ctx->queued.graphics, waitCount, waitInfo, 2, signalInfo);*/
}

NeCommandBufferHandle
Re_BeginSecondary(struct NeRenderPassDesc *passDesc)
{
	/*struct NeRenderContext *ctx = Re_CurrentContext();
	ctx->cmdBuffer = Vkd_AllocateCmdBuffer(ctx->vkDev, VK_COMMAND_BUFFER_LEVEL_SECONDARY,
										   ctx->graphicsPools[Re_frameId]);
	assert(ctx->cmdBuffer);

#ifdef _DEBUG
	char name[64];
	snprintf(name, sizeof(name), "Secondary CmdBuffer %u", Re_frameId);
	Vkd_SetObjectName(ctx->vkDev, ctx->cmdBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, name);
#endif

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

	return ctx->cmdBuffer;*/ return NULL;
}

void
Re_BeginDrawCommandBuffer(struct NeSemaphore *wait)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	if (ctx->free.gfx[Re_frameId].count)
		ctx->cmdList = (ID3D12GraphicsCommandList7 *)Rt_ArrayGetPtr(&ctx->free.gfx[Re_frameId], --ctx->free.gfx[Re_frameId].count);
	else
		ctx->d3dDev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, ctx->gfxAllocators[Re_frameId], NULL, IID_PPV_ARGS(&ctx->cmdList));

	ctx->wait = wait;
}

void
Re_BeginComputeCommandBuffer(struct NeSemaphore *wait)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	if (ctx->free.comp[Re_frameId].count)
		ctx->cmdList = (ID3D12GraphicsCommandList7 *)Rt_ArrayGetPtr(&ctx->free.comp[Re_frameId], --ctx->free.comp[Re_frameId].count);
	else
		ctx->d3dDev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, ctx->compAllocators[Re_frameId], NULL, IID_PPV_ARGS(&ctx->cmdList));

	ctx->wait = wait;
}

void
Re_BeginTransferCommandBuffer(struct NeSemaphore *wait)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	if (ctx->free.copy[Re_frameId].count)
		ctx->cmdList = (ID3D12GraphicsCommandList7 *)Rt_ArrayGetPtr(&ctx->free.copy[Re_frameId], --ctx->free.copy[Re_frameId].count);
	else
		ctx->d3dDev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, ctx->copyAllocators[Re_frameId], NULL, IID_PPV_ARGS(&ctx->cmdList));

	ctx->wait = wait;
}

NeCommandBufferHandle
Re_EndCommandBuffer(void)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	NeCommandBufferHandle cb = ctx->cmdList;

	if (HRESULT hr = ctx->cmdList->Close(); FAILED(hr)) {
		D3D12BK_LOG_ERR("ID3D12GraphicsCommandList7::Close", hr);
	}
	ctx->cmdList = NULL;

	return cb;
}

void
Re_CmdBindPipeline(struct NePipeline *pipeline)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	ID3D12DescriptorHeap *heaps[] = { ctx->neDev->gpuDescriptorHeap[Re_frameId] };
	ctx->cmdList->SetDescriptorHeaps(_countof(heaps), heaps);

	ctx->cmdList->SetGraphicsRootSignature(pipeline->rs);
	ctx->cmdList->SetGraphicsRootDescriptorTable(0, heaps[0]->GetGPUDescriptorHandleForHeapStart());

	ctx->cmdList->IASetPrimitiveTopology(pipeline->topology);
	ctx->cmdList->SetPipelineState(pipeline->ps);
}

void
Re_CmdPushConstants(NeShaderStageFlags stage, uint32_t size, const void *data)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

/*	if (stage == SS_COMPUTE)
		ctx->cmdList->SetComputeRoot32BitConstants(0, size / 4, data, 0);
	else
		ctx->cmdList->SetGraphicsRoot32BitConstants(0, size / 4, data, 0);*/
}

void
Re_BkCmdBindVertexBuffer(struct NeBuffer *buff, uint64_t offset)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	const D3D12_VERTEX_BUFFER_VIEW view =
	{
		.BufferLocation = buff->res->GetGPUVirtualAddress() + offset,
		.SizeInBytes = (UINT)(buff->size - offset),
		.StrideInBytes = ctx->boundPipeline->vertexBufferStride[0]
	};
	ctx->cmdList->IASetVertexBuffers(0, 1, &view);
}

void
Re_BkCmdBindVertexBuffers(uint32_t count, struct NeBuffer **buffers, uint64_t *offsets, uint32_t start)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	D3D12_VERTEX_BUFFER_VIEW *views = (D3D12_VERTEX_BUFFER_VIEW *)Sys_Alloc(sizeof(*views), count, MH_Frame);
	for (uint32_t i = 0; i < count; ++i) {
		views[i].BufferLocation = buffers[i]->res->GetGPUVirtualAddress() + offsets[i];
		views[i].SizeInBytes = (UINT)(buffers[i]->size - offsets[i]);
		views[i].StrideInBytes = ctx->boundPipeline->vertexBufferStride[i];
	}
	ctx->cmdList->IASetVertexBuffers(start, count, views);
}

void
Re_BkCmdBindIndexBuffer(struct NeBuffer *buff, uint64_t offset, enum NeIndexType type)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	D3D12_INDEX_BUFFER_VIEW view =
	{
		.BufferLocation = buff->res->GetGPUVirtualAddress() + offset,
		.SizeInBytes = (UINT)(buff->size - offset),
		.Format = type == IT_UINT_16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT
	};
	ctx->cmdList->IASetIndexBuffer(&view);
}

void
Re_CmdExecuteSecondary(NeCommandBufferHandle *cmdBuffers, uint32_t count)
{
//	struct NeRenderContext *ctx = Re_CurrentContext();
//	vkCmdExecuteCommands(ctx->cmdBuffer, count, (VkCommandBuffer *)cmdBuffers);
}

void
Re_CmdBeginRenderPass(struct NeRenderPassDesc *passDesc, struct NeFramebuffer *fb, enum NeRenderCommandContents contents)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	D3D12_CPU_DESCRIPTOR_HANDLE targetHandle = ctx->neDev->rtvHeap->GetCPUDescriptorHandleForHeapStart();

	uint32_t barrierCount = 0;
	D3D12_RESOURCE_BARRIER barriers[9];

	D3D12_CPU_DESCRIPTOR_HANDLE handles[8];
	for (uint32_t i = 0; i < passDesc->attachmentCount; ++i) {
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc =
		{
			.Format = passDesc->rtvFormats[i],
			.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D
		};
		ctx->d3dDev->CreateRenderTargetView(fb->attachments[i], &rtvDesc, targetHandle);


		if (passDesc->loadOp[i] == ATL_CLEAR)
			ctx->cmdList->ClearRenderTargetView(targetHandle, passDesc->clearValues[i], 0, NULL);

		handles[i] = targetHandle;
		targetHandle.ptr += ctx->neDev->rtvIncrement;

		D3D12_RESOURCE_BARRIER *barrier = &barriers[barrierCount++];
		barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier->Transition.pResource = fb->attachments[i];
		barrier->Transition.StateBefore = passDesc->rtvInitialState[i];
		barrier->Transition.StateAfter = passDesc->rtvState[i];
		barrier->Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = ctx->neDev->dsvHeap->GetCPUDescriptorHandleForHeapStart();
	if (passDesc->depthFormat != DXGI_FORMAT_UNKNOWN) {
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc =
		{
			.Format = passDesc->depthFormat,
			.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
			//.Flags = 0 // TODO: read only depth
		};
		ctx->d3dDev->CreateDepthStencilView(fb->depthAttachment, &dsvDesc, targetHandle);

		if (passDesc->depthLoadOp == ATL_CLEAR)
			ctx->cmdList->ClearDepthStencilView(targetHandle, D3D12_CLEAR_FLAG_DEPTH, passDesc->depthClearValue, 0, 0, NULL);

		D3D12_RESOURCE_BARRIER *barrier = &barriers[barrierCount++];
		barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier->Transition.pResource = fb->depthAttachment;
		barrier->Transition.StateBefore = passDesc->depthInitialState;
		barrier->Transition.StateAfter = passDesc->depthState;
		barrier->Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		ctx->cmdList->ResourceBarrier(barrierCount, barriers);
		ctx->cmdList->OMSetRenderTargets(passDesc->attachmentCount, handles, TRUE, &dsvHandle);
	} else {
		ctx->cmdList->ResourceBarrier(barrierCount, barriers);
		ctx->cmdList->OMSetRenderTargets(passDesc->attachmentCount, handles, TRUE, NULL);
	}

	ctx->boundRenderPass = passDesc;
	ctx->boundFramebuffer = fb;
}

void
Re_CmdEndRenderPass(void)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

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

	ctx->cmdList->ResourceBarrier(barrierCount, barriers);

	ctx->boundRenderPass = NULL;
	ctx->boundFramebuffer = NULL;
}

void
Re_CmdSetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	D3D12_VIEWPORT vp = { x, height + y, width, -height, minDepth, maxDepth };
	ctx->cmdList->RSSetViewports(1, &vp);
}

void
Re_CmdSetScissor(int32_t x, int32_t y, int32_t width, int32_t height)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	D3D12_RECT scissor = { x, y, width, height };
	ctx->cmdList->RSSetScissorRects(1, &scissor);
}

void
Re_CmdSetLineWidth(float width)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	//ctx->cmdList->L
	//vkCmdSetLineWidth(ctx->cmdBuffer, width);
}

void
Re_CmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	ctx->cmdList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
}

void
Re_CmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	ctx->cmdList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void
Re_CmdDrawIndirect(struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	//ctx->cmdList->ExecuteIndirect()

	//vkCmdDrawIndirect(ctx->cmdBuffer, buff->buff, offset, count, stride);
}

void
Re_CmdDrawIndexedIndirect(struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	//vkCmdDrawIndexedIndirect(ctx->cmdBuffer, buff->buff, offset, count, stride);
}

void
Re_CmdDrawMeshTasks(uint32_t taskCount, uint32_t firstTask)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
//	ctx->cmdList->DispatchMesh(thx, thy, thz);
	//vkCmdDrawMeshTasksNV(ctx->cmdBuffer, taskCount, firstTask);
}

void
Re_CmdDrawMeshTasksIndirect(struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
//	vkCmdDrawMeshTasksIndirectNV(ctx->cmdBuffer, buff->buff, offset, count, stride);
}

void
Re_CmdDrawMeshTasksIndirectCount(struct NeBuffer *buff, uint64_t offset, struct NeBuffer *countBuff, uint64_t countOffset, uint32_t maxDrawCount, uint32_t stride)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
//	vkCmdDrawMeshTasksIndirectCountNV(ctx->cmdBuffer, buff->buff, offset, countBuff->buff, countOffset, maxDrawCount, stride);
}

void
Re_CmdDispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	ctx->cmdList->Dispatch(groupCountX, groupCountY, groupCountZ);
}

void
Re_CmdDispatchIndirect(struct NeBuffer *buff, uint64_t offset)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	//vkCmdDispatchIndirect(ctx->cmdBuffer, buff->buff, offset);
}

void
Re_CmdTraceRays(uint32_t width, uint32_t height, uint32_t depth)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	D3D12_DISPATCH_RAYS_DESC desc
	{
	//	D3D12_GPU_VIRTUAL_ADDRESS_RANGE RayGenerationShaderRecord;
	//D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE MissShaderTable;
	//D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE HitGroupTable;
	//D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE CallableShaderTable;
		//.Width = width,
		//.Height = height,
		//.Depth = depth
	};
	ctx->cmdList->DispatchRays(&desc);
}

void
Re_CmdTraceRaysIndirect(struct NeBuffer *buff, uint64_t offset)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	//vkCmdTraceRaysIndirectKHR(ctx->cmdBuffer, NULL, NULL, NULL, NULL, Re_BkBufferAddress(buff, offset));
}

void
Re_CmdBuildAccelerationStructures(uint32_t count, struct NeAccelerationStructureBuildInfo *buildInfo, const struct NeAccelerationStructureRangeInfo **rangeInfo)
{
	/*struct NeRenderContext *ctx = Re_CurrentContext();
	VkAccelerationStructureBuildGeometryInfoKHR *geomInfo = Sys_Alloc(count, sizeof(*geomInfo), MH_Transient);

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

void
Re_CmdBarrier(enum NePipelineDependency dep,
	uint32_t memBarrierCount, const struct NeMemoryBarrier *memBarriers,
	uint32_t bufferBarrierCount, const struct NeBufferBarrier *bufferBarriers,
	uint32_t imageBarrierCount, const struct NeImageBarrier *imageBarriers)
{
	/*struct NeRenderContext *ctx = Re_CurrentContext();
	VkMemoryBarrier2KHR *vkMemBarriers = NULL;
	VkBufferMemoryBarrier2KHR *vkBufferBarriers = NULL;
	VkImageMemoryBarrier2KHR *vkImageBarriers = NULL;

	if (memBarrierCount) {
		vkMemBarriers = Sys_Alloc(sizeof(*vkMemBarriers), memBarrierCount, MH_Transient);

		for (uint32_t i = 0; i < memBarrierCount; ++i) {
			vkMemBarriers[i].sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR;
			vkMemBarriers[i].pNext = NULL;

			vkMemBarriers[i].srcStageMask = memBarriers[i].srcStage;
			vkMemBarriers[i].dstStageMask = memBarriers[i].dstStage;

			vkMemBarriers[i].srcAccessMask = memBarriers[i].srcAccess;
			vkMemBarriers[i].dstAccessMask = memBarriers[i].dstAccess;
		}
	}

	if (bufferBarrierCount) {
		vkBufferBarriers = Sys_Alloc(sizeof(*vkBufferBarriers), bufferBarrierCount, MH_Transient);

		for (uint32_t i = 0; i < bufferBarrierCount; ++i) {
			vkBufferBarriers[i].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2_KHR;
			vkBufferBarriers[i].pNext = NULL;

			vkBufferBarriers[i].srcStageMask = bufferBarriers[i].srcStage;
			vkBufferBarriers[i].dstStageMask = bufferBarriers[i].dstStage;

			vkBufferBarriers[i].srcAccessMask = bufferBarriers[i].srcAccess;
			vkBufferBarriers[i].dstAccessMask = bufferBarriers[i].dstAccess;

			vkBufferBarriers[i].srcQueueFamilyIndex = QueueFamilyIndex(ctx->neDev, bufferBarriers[i].srcQueue);
			vkBufferBarriers[i].dstQueueFamilyIndex = QueueFamilyIndex(ctx->neDev, bufferBarriers[i].dstQueue);

			vkBufferBarriers[i].buffer = bufferBarriers[i].buffer->buff;
			vkBufferBarriers[i].offset = bufferBarriers[i].offset;
			vkBufferBarriers[i].size = bufferBarriers[i].size;
		}
	}

	if (imageBarrierCount) {
		vkImageBarriers = Sys_Alloc(sizeof(*vkImageBarriers), imageBarrierCount, MH_Transient);

		for (uint32_t i = 0; i < imageBarrierCount; ++i) {
			vkImageBarriers[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR;
			vkImageBarriers[i].pNext = NULL;

			vkImageBarriers[i].srcStageMask = imageBarriers[i].srcStage;
			vkImageBarriers[i].dstStageMask = imageBarriers[i].dstStage;

			vkImageBarriers[i].srcAccessMask = imageBarriers[i].srcAccess;
			vkImageBarriers[i].dstAccessMask = imageBarriers[i].dstAccess;

			vkImageBarriers[i].oldLayout = NeToVkImageLayout(imageBarriers[i].oldLayout);
			vkImageBarriers[i].newLayout = NeToVkImageLayout(imageBarriers[i].newLayout);

			vkImageBarriers[i].srcQueueFamilyIndex = QueueFamilyIndex(ctx->neDev, imageBarriers[i].srcQueue);
			vkImageBarriers[i].dstQueueFamilyIndex = QueueFamilyIndex(ctx->neDev, imageBarriers[i].dstQueue);

			vkImageBarriers[i].image = imageBarriers[i].texture->image;
			vkImageBarriers[i].subresourceRange.aspectMask = imageBarriers[i].subresource.aspect;
			vkImageBarriers[i].subresourceRange.baseMipLevel = imageBarriers[i].subresource.mipLevel;
			vkImageBarriers[i].subresourceRange.baseArrayLayer = imageBarriers[i].subresource.baseArrayLayer;
			vkImageBarriers[i].subresourceRange.layerCount = imageBarriers[i].subresource.layerCount;
			vkImageBarriers[i].subresourceRange.levelCount = imageBarriers[i].subresource.levelCount;
		}
	}

	VkDependencyInfoKHR di =
		{
			.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR,
			.bufferMemoryBarrierCount = bufferBarrierCount,
			.pBufferMemoryBarriers = vkBufferBarriers,
			.imageMemoryBarrierCount = imageBarrierCount,
			.pImageMemoryBarriers = vkImageBarriers,
			.memoryBarrierCount = memBarrierCount,
			.pMemoryBarriers = vkMemBarriers,
			.dependencyFlags = dep
		};
	vkCmdPipelineBarrier2KHR(ctx->cmdBuffer, &di);*/
}

void
Re_CmdTransition(struct NeTexture *tex, enum NeTextureLayout newLayout)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	D3D12_RESOURCE_BARRIER b = CD3DX12_RESOURCE_BARRIER::Transition(tex->res, tex->state,NeImageLayoutToD3DResourceState(newLayout));
	ctx->cmdList->ResourceBarrier(1, &b);

	tex->state = NeImageLayoutToD3DResourceState(newLayout);
}

void
Re_BkCmdUpdateBuffer(const struct NeBuffer *buff, uint64_t offset, void *data, uint64_t size)
{
	D3D12_RESOURCE_DESC rd = buff->res->GetDesc();
	D3D12_RESOURCE_ALLOCATION_INFO rai = Re_device->dev->GetResourceAllocationInfo(0, 1, &rd);

	D3D12_HEAP_PROPERTIES hp{ D3D12_HEAP_TYPE_UPLOAD };
	rd = CD3DX12_RESOURCE_DESC::Buffer(size, D3D12_RESOURCE_FLAG_NONE, rai.Alignment);

	ID3D12Resource *staging = nullptr;
	Re_device->dev->CreateCommittedResource(&hp, D3D12_HEAP_FLAG_NONE, &rd, D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_PPV_ARGS(&staging));

	Re_device->copyCmdList->Reset(Re_device->copyAllocator, NULL);

	D3D12_SUBRESOURCE_DATA srd = { data, (LONG_PTR)size, (LONG_PTR)size };
	UpdateSubresources(Re_device->copyCmdList, buff->res, staging, offset, 0, 1, &srd);

	Re_device->copyCmdList->Close();

	Re_device->copy->ExecuteCommandLists(1, (ID3D12CommandList **)&Re_device->copyCmdList);
	D3D12Bk_SignalFenceGPU(&Re_device->copyFence, Re_device->copy);

	D3D12Bk_WaitForFenceCPU(&Re_device->copyFence);

	staging->Release();
}

void
Re_BkCmdCopyBuffer(const struct NeBuffer *src, uint64_t srcOffset, struct NeBuffer *dst, uint64_t dstOffset, uint64_t size)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	uint32_t bCount = 0;
	D3D12_RESOURCE_BARRIER b[2];

	if (src->state != D3D12_RESOURCE_STATE_COPY_SOURCE)
		b[bCount++] = CD3DX12_RESOURCE_BARRIER::Transition(src->res, src->state, D3D12_RESOURCE_STATE_COPY_SOURCE);

	if (dst->state != D3D12_RESOURCE_STATE_COPY_DEST)
		b[bCount++] = CD3DX12_RESOURCE_BARRIER::Transition(dst->res, dst->state, D3D12_RESOURCE_STATE_COPY_DEST);

	ctx->cmdList->ResourceBarrier(bCount, b);
	ctx->cmdList->CopyBufferRegion(dst->res, dstOffset, src->res, srcOffset, size);

	bCount = 0;

	if (src->state != D3D12_RESOURCE_STATE_COPY_SOURCE)
		b[bCount++] = CD3DX12_RESOURCE_BARRIER::Transition(src->res, D3D12_RESOURCE_STATE_COPY_SOURCE, src->state);

	if (dst->state != D3D12_RESOURCE_STATE_COPY_DEST)
		b[bCount++] = CD3DX12_RESOURCE_BARRIER::Transition(dst->res, D3D12_RESOURCE_STATE_COPY_DEST, dst->state);

	ctx->cmdList->ResourceBarrier(bCount, b);
}

void
Re_CmdCopyImage(const struct NeTexture *src, struct NeTexture *dst)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	//ctx->cmdList->CopyTextureRegion()

	//vkCmdCopyImage(ctx->cmdBuffer, src->image, src->layout, dst->image, dst->layout, 1, NULL);

}

void
Re_BkCmdCopyBufferToTexture(const struct NeBuffer *src, struct NeTexture *dst, const struct NeBufferImageCopy *bic)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	D3D12_RESOURCE_DESC rd = dst->res->GetDesc();
	D3D12_SUBRESOURCE_FOOTPRINT fp
	{
		.Format = rd.Format,
		.Width = (UINT)rd.Width,
		.Height = rd.Height,
		.Depth = rd.DepthOrArraySize,
		.RowPitch = 0// Align(bitmapWidth * sizeof(DWORD), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	};

	uint32_t bCount = 0;
	D3D12_RESOURCE_BARRIER b[2];

	if (src->state != D3D12_RESOURCE_STATE_COPY_SOURCE)
		b[bCount++] = CD3DX12_RESOURCE_BARRIER::Transition(src->res, src->state, D3D12_RESOURCE_STATE_COPY_SOURCE);

	if (dst->state != D3D12_RESOURCE_STATE_COPY_DEST)
		b[bCount++] = CD3DX12_RESOURCE_BARRIER::Transition(dst->res, dst->state, D3D12_RESOURCE_STATE_COPY_DEST);

	if (bCount)
		ctx->cmdList->ResourceBarrier(bCount, b);

	D3D12_TEXTURE_COPY_LOCATION dstLoc = CD3DX12_TEXTURE_COPY_LOCATION(dst->res, 0);
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint
	{
		bic->bufferOffset,
		rd.Format,
		(UINT)bic->imageSize.width,
		(UINT)bic->imageSize.height,
		(UINT)bic->imageSize.depth,
		bic->bytesPerRow
	};
	D3D12_TEXTURE_COPY_LOCATION srcLoc = CD3DX12_TEXTURE_COPY_LOCATION(src->res, footprint);
	ctx->cmdList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);

	bCount = 0;

	if (src->state != D3D12_RESOURCE_STATE_COPY_SOURCE)
		b[bCount++] = CD3DX12_RESOURCE_BARRIER::Transition(src->res, D3D12_RESOURCE_STATE_COPY_SOURCE, src->state);

	if (dst->state != D3D12_RESOURCE_STATE_COPY_DEST)
		b[bCount++] = CD3DX12_RESOURCE_BARRIER::Transition(dst->res, D3D12_RESOURCE_STATE_COPY_DEST, dst->state);

	if (bCount)
		ctx->cmdList->ResourceBarrier(bCount, b);

	/*VkBufferImageCopy b =
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

void
Re_BkCmdCopyTextureToBuffer(const struct NeTexture *src, struct NeBuffer *dst, const struct NeBufferImageCopy *bic)
{
	/*struct NeRenderContext *ctx = Re_CurrentContext();
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
	vkCmdCopyImageToBuffer(ctx->cmdBuffer, src->image, src->layout, dst->buff, 1, &b);*/
}

void
Re_CmdBlit(const struct NeTexture *src, struct NeTexture *dst, const struct NeBlitRegion *regions, uint32_t regionCount, enum NeImageFilter filter)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	/*VkFilter f = VK_FILTER_NEAREST;

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

void
Re_CmdLoadBuffer(struct NeBuffer *buff, uint64_t offset, uint64_t size, void *handle, uint64_t sourceOffset)
{
	//
}

void
Re_CmdLoadTexture(struct NeTexture *tex, uint32_t slice, uint32_t level, uint32_t width, uint32_t height, uint32_t depth,
					uint32_t bytesPerRow, struct NeImageOffset *origin, void *handle, uint64_t sourceOffset)
{
	//
}

bool
Re_QueueGraphics(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *signal)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	struct D3D12_SubmitInfo si =
	{
		.wait = ctx->wait ? &ctx->wait->df : nullptr,
		.signal = signal ? &signal->df : nullptr,
		.waitValue = ctx->wait ? ctx->wait->df.value : 0,
		.signalValue = signal ? ++signal->df.value : 0,
		.cmdList = (ID3D12GraphicsCommandList7 *)cmdBuffer
	};
	Rt_ArrayAddPtr(&ctx->closed.gfx[Re_frameId], cmdBuffer);
	ctx->wait = nullptr;
	return Rt_ArrayAdd(&ctx->queued.graphics, &si);
}

bool
Re_QueueCompute(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *signal)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	struct D3D12_SubmitInfo si =
	{
		.wait = ctx->wait ? &ctx->wait->df : nullptr,
		.signal = signal ? &signal->df : nullptr,
		.waitValue = ctx->wait ? ctx->wait->df.value : 0,
		.signalValue = signal ? ++signal->df.value : 0,
		.cmdList = (ID3D12GraphicsCommandList7 *)cmdBuffer
	};
	Rt_ArrayAddPtr(&ctx->closed.comp[Re_frameId], cmdBuffer);
	ctx->wait = nullptr;
	return Rt_ArrayAdd(&ctx->queued.compute, &si);
}

bool
Re_QueueTransfer(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *signal)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	struct D3D12_SubmitInfo si =
	{
		.wait = ctx->wait ? &ctx->wait->df : nullptr,
		.signal = signal ? &signal->df : nullptr,
		.waitValue = ctx->wait ? ctx->wait->df.value : 0,
		.signalValue = signal ? ++signal->df.value : 0,
		.cmdList = (ID3D12GraphicsCommandList7 *)cmdBuffer
	};
	Rt_ArrayAddPtr(&ctx->closed.copy[Re_frameId], cmdBuffer);
	ctx->wait = nullptr;
	return Rt_ArrayAdd(&ctx->queued.copy, &si);
}

bool
Re_ExecuteGraphics(NeCommandBufferHandle cmdBuffer)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	D3D12_Fence f{};
	D3D12Bk_InitFence(&f, false);

	ctx->neDev->direct->ExecuteCommandLists(1, (ID3D12CommandList **)&cmdBuffer);
	D3D12Bk_SignalFenceGPU(&f, ctx->neDev->direct);

	Rt_ArrayAddPtr(&ctx->closed.gfx[Re_frameId], &cmdBuffer);

	const bool rc = D3D12Bk_WaitForFenceCPU(&f);
	D3D12Bk_TermFence(&f);

	return rc;
}

bool
Re_ExecuteCompute(NeCommandBufferHandle cmdBuffer)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	D3D12_Fence f{};
	D3D12Bk_InitFence(&f, false);

	ctx->neDev->compute->ExecuteCommandLists(1, (ID3D12CommandList **)&cmdBuffer);
	D3D12Bk_SignalFenceGPU(&f, ctx->neDev->compute);

	Rt_ArrayAddPtr(&ctx->closed.comp[Re_frameId], &cmdBuffer);

	const bool rc = D3D12Bk_WaitForFenceCPU(&f);
	D3D12Bk_TermFence(&f);

	return rc;
}

bool
Re_ExecuteTransfer(NeCommandBufferHandle cmdBuffer)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	D3D12_Fence f{};
	D3D12Bk_InitFence(&f, false);

	ctx->neDev->copy->ExecuteCommandLists(1, (ID3D12CommandList **)&cmdBuffer);
	D3D12Bk_SignalFenceGPU(&f, ctx->neDev->copy);

	Rt_ArrayAddPtr(&ctx->closed.copy[Re_frameId], &cmdBuffer);

	const bool rc = D3D12Bk_WaitForFenceCPU(&f);
	D3D12Bk_TermFence(&f);

	return rc;
}

void
Re_BeginDirectIO(void)
{
	//
}

bool
Re_SubmitDirectIO(bool *completed)
{
	return false;
}

bool
Re_ExecuteDirectIO(void)
{
	return false;
}

void
Submit(ID3D12CommandQueue *queue, struct NeArray *submitInfo,
		uint32_t waitCount, NeSemaphore *wait,
		uint32_t signalCount, NeSemaphore *signal)
{
	if (!submitInfo->count)
		return;

	struct NeArray ;//, cbInfo, sInfo;
	ID3D12CommandList **cmdLists = (ID3D12CommandList **)Sys_Alloc(sizeof(*cmdLists), submitInfo->count, MH_Transient);

//	Rt_InitArray(&cbInfo, submitInfo->count, sizeof(VkCommandBufferSubmitInfoKHR), MH_Transient);
//	Rt_InitArray(&sInfo, submitInfo->count * 2 /* wait & signal */, sizeof(VkSemaphoreSubmitInfoKHR), MH_Transient);

	D3D12_SubmitInfo *esi;
	Rt_ArrayForEach(esi, submitInfo, D3D12_SubmitInfo *) {
		cmdLists[miwa_rtafei] = esi->cmdList;

		/*VkSubmitInfo2 *si = Rt_ArrayAllocate(&info);
		si->sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;

		VkCommandBufferSubmitInfoKHR *cbsi = Rt_ArrayAllocate(&cbInfo);
		cbsi->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR;
		cbsi->commandBuffer = esi->cmdBuffer;

		si->commandBufferInfoCount = 1;
		si->pCommandBufferInfos = cbsi;

		if (esi->wait) {
			VkSemaphoreSubmitInfoKHR *ssi = Rt_ArrayAllocate(&sInfo);
			ssi->sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR;

			ssi->semaphore = esi->wait;
			ssi->value = esi->waitValue;
			ssi->stageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR;
			ssi->deviceIndex = 0;

			si->pWaitSemaphoreInfos = ssi;
			si->waitSemaphoreInfoCount = 1;
		} else {
			si->pWaitSemaphoreInfos = wait;
			si->waitSemaphoreInfoCount = waitCount;
		}

		if (esi->signal) {
			VkSemaphoreSubmitInfoKHR *ssi = Rt_ArrayAllocate(&sInfo);
			ssi->sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR;

			ssi->semaphore = esi->signal;
			ssi->value = esi->signalValue;
			ssi->stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT_KHR;
			ssi->deviceIndex = 0;

			si->pSignalSemaphoreInfos = ssi;
			si->signalSemaphoreInfoCount = 1;
		} else {
			si->pSignalSemaphoreInfos = signal;
			si->signalSemaphoreInfoCount = signalCount;
		}*/
	}

	queue->ExecuteCommandLists((UINT)submitInfo->count, cmdLists);
}

/* NekoEngine
 *
 * D3D12Context.cxx
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
