#include <assert.h>

#include <System/Memory.h>
#include <Runtime/Runtime.h>

#include "VulkanBackend.h"

static inline uint32_t _queueFamilyIndex(struct NeRenderDevice *dev, enum NeRenderQueue queue);
static inline void _Submit(struct VkdRenderQueue *queue, struct NeArray *submitInfo,
	uint32_t waitCount, VkSemaphoreSubmitInfoKHR *wait,
	uint32_t signalCount, VkSemaphoreSubmitInfoKHR *signal);

struct NeRenderContext *
Re_CreateContext(void)
{
	struct NeRenderContext *ctx = Sys_Alloc(1, sizeof(*ctx), MH_RenderDriver);
	if (!ctx)
		return NULL;

	VkCommandPoolCreateInfo poolInfo =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
	};

	struct NeArray *arrays = Sys_Alloc(RE_NUM_FRAMES * 4, sizeof(*arrays), MH_RenderDriver);
	if (!arrays)
		goto error;

	ctx->graphicsCmdBuffers = arrays;
	ctx->secondaryCmdBuffers = &arrays[RE_NUM_FRAMES];
	ctx->computeCmdBuffers = &arrays[RE_NUM_FRAMES * 2];
	ctx->xferCmdBuffers = &arrays[RE_NUM_FRAMES * 3];

	VkCommandPool *pools = Sys_Alloc(RE_NUM_FRAMES * 3, sizeof(*pools), MH_RenderDriver);
	if (!pools)
		goto error;

	ctx->graphicsPools = pools;
	ctx->computePools = &pools[RE_NUM_FRAMES];
	ctx->xferPools = &pools[RE_NUM_FRAMES * 2];

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		poolInfo.queueFamilyIndex = Re_device->graphics.family;
		if (vkCreateCommandPool(Re_device->dev, &poolInfo, Vkd_allocCb, &ctx->graphicsPools[i]) != VK_SUCCESS)
			goto error;

		poolInfo.queueFamilyIndex = Re_device->compute.family;
		if (vkCreateCommandPool(Re_device->dev, &poolInfo, Vkd_allocCb, &ctx->computePools[i]) != VK_SUCCESS)
			goto error;

		poolInfo.queueFamilyIndex = Re_device->transfer.family;
		if (vkCreateCommandPool(Re_device->dev, &poolInfo, Vkd_allocCb, &ctx->xferPools[i]) != VK_SUCCESS)
			goto error;

		if (!Rt_InitPtrArray(&ctx->graphicsCmdBuffers[i], 10, MH_RenderDriver))
			goto error;

		if (!Rt_InitPtrArray(&ctx->xferCmdBuffers[i], 10, MH_RenderDriver))
			goto error;

		if (!Rt_InitPtrArray(&ctx->computeCmdBuffers[i], 10, MH_RenderDriver))
			goto error;

		if (!Rt_InitPtrArray(&ctx->secondaryCmdBuffers[i], 10, MH_RenderDriver))
			goto error;

	#ifdef _DEBUG
		char name[64];
		snprintf(name, sizeof(name), "Graphics Pool %u", Re_frameId);
		Vkd_SetObjectName(Re_device->dev, ctx->graphicsPools[i], VK_OBJECT_TYPE_COMMAND_POOL, name);

		snprintf(name, sizeof(name), "Compute Pool %u", Re_frameId);
		Vkd_SetObjectName(Re_device->dev, ctx->computePools[i], VK_OBJECT_TYPE_COMMAND_POOL, name);

		snprintf(name, sizeof(name), "Transfer Pool %u", Re_frameId);
		Vkd_SetObjectName(Re_device->dev, ctx->xferPools[i], VK_OBJECT_TYPE_COMMAND_POOL, name);
	#endif
	}

	VkFenceCreateInfo fci = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	vkCreateFence(Re_device->dev, &fci, Vkd_allocCb, &ctx->executeFence);

	ctx->vkDev = Re_device->dev;
	ctx->neDev = Re_device;
	ctx->descriptorSet = Re_device->descriptorSet;

	if (!Rt_InitArray(&ctx->queued.graphics, 10, sizeof(struct Vkd_SubmitInfo), MH_RenderDriver) ||
		!Rt_InitArray(&ctx->queued.compute, 10, sizeof(struct Vkd_SubmitInfo), MH_RenderDriver) ||
		!Rt_InitArray(&ctx->queued.xfer, 10, sizeof(struct Vkd_SubmitInfo), MH_RenderDriver))
		goto error;

	return ctx;

error:
	if (pools) {
		for (uint32_t i = 0; i < RE_NUM_FRAMES * 3; ++i)
			if (pools[i] != VK_NULL_HANDLE)
				vkDestroyCommandPool(Re_device->dev, pools[i], Vkd_allocCb);
	}

	Sys_Free(arrays);
	Sys_Free(pools);
	Sys_Free(ctx);

	return NULL;
}

void
Re_ResetContext(struct NeRenderContext *ctx)
{
	if (ctx->secondaryCmdBuffers[Re_frameId].count) {
		vkFreeCommandBuffers(Re_device->dev, ctx->graphicsPools[Re_frameId],
			(uint32_t)ctx->secondaryCmdBuffers[Re_frameId].count, (const VkCommandBuffer *)ctx->secondaryCmdBuffers[Re_frameId].data);
		Rt_ClearArray(&ctx->secondaryCmdBuffers[Re_frameId], false);
	}

	if (ctx->graphicsCmdBuffers[Re_frameId].count) {
		vkFreeCommandBuffers(Re_device->dev, ctx->graphicsPools[Re_frameId],
			(uint32_t)ctx->graphicsCmdBuffers[Re_frameId].count, (const VkCommandBuffer *)ctx->graphicsCmdBuffers[Re_frameId].data);
		vkResetCommandPool(Re_device->dev, ctx->graphicsPools[Re_frameId], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		Rt_ClearArray(&ctx->graphicsCmdBuffers[Re_frameId], false);
	}

	if (ctx->xferCmdBuffers[Re_frameId].count) {
		vkFreeCommandBuffers(Re_device->dev, ctx->xferPools[Re_frameId],
			(uint32_t)ctx->xferCmdBuffers[Re_frameId].count, (const VkCommandBuffer *)ctx->xferCmdBuffers[Re_frameId].data);
		vkResetCommandPool(Re_device->dev, ctx->xferPools[Re_frameId], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		Rt_ClearArray(&ctx->xferCmdBuffers[Re_frameId], false);
	}
	ctx->lastSubmittedXfer = 0;

	if (ctx->computeCmdBuffers[Re_frameId].count) {
		vkFreeCommandBuffers(Re_device->dev, ctx->computePools[Re_frameId],
			(uint32_t)ctx->computeCmdBuffers[Re_frameId].count, (const VkCommandBuffer *)ctx->computeCmdBuffers[Re_frameId].data);
		vkResetCommandPool(Re_device->dev, ctx->computePools[Re_frameId], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		Rt_ClearArray(&ctx->computeCmdBuffers[Re_frameId], false);
	}
	ctx->lastSubmittedCompute = 0;

	Rt_ClearArray(&ctx->queued.graphics, false);
	Rt_ClearArray(&ctx->queued.compute, false);
	Rt_ClearArray(&ctx->queued.xfer, false);
}

void
Re_DestroyContext(struct NeRenderContext *ctx)
{
	vkDestroyFence(Re_device->dev, ctx->executeFence, Vkd_allocCb);

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		vkDestroyCommandPool(Re_device->dev, ctx->graphicsPools[i], Vkd_allocCb);
		vkDestroyCommandPool(Re_device->dev, ctx->computePools[i], Vkd_allocCb);
		vkDestroyCommandPool(Re_device->dev, ctx->xferPools[i], Vkd_allocCb);
		Rt_TermArray(&ctx->graphicsCmdBuffers[i]);
		Rt_TermArray(&ctx->secondaryCmdBuffers[i]);
		Rt_TermArray(&ctx->xferCmdBuffers[i]);
		Rt_TermArray(&ctx->computeCmdBuffers[i]);
	}

	Rt_TermArray(&ctx->queued.graphics);
	Rt_TermArray(&ctx->queued.compute);
	Rt_TermArray(&ctx->queued.xfer);

	Sys_Free(ctx->graphicsCmdBuffers);
	Sys_Free(ctx->graphicsPools);
	Sys_Free(ctx);
}

void
Vkd_ExecuteCommands(struct NeRenderDevice *dev, struct NeRenderContext *ctx, struct NeSwapchain *sw, struct NeSemaphore *waitSemaphore)
{
	dev->frameValues[Re_frameId] = ++dev->semaphoreValue;

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
		_Submit(&dev->transfer, &ctx->queued.xfer, waitCount, waitInfo, 1, signalInfo);
		ADD_WAIT(dev->frameSemaphore, dev->frameValues[Re_frameId]);

		dev->frameValues[Re_frameId] = ++dev->semaphoreValue;
		signalInfo[0].value = dev->frameValues[Re_frameId];
	}

	_Submit(&dev->compute, &ctx->queued.compute, waitCount, waitInfo, 0, NULL);

	if (Re_deviceInfo.features.coherentMemory) {
		ADD_WAIT(sw->frameStart[Re_frameId], 0);
	}
	
	_Submit(&dev->graphics, &ctx->queued.graphics, waitCount, waitInfo, 2, signalInfo);
}

NeCommandBufferHandle
Re_BeginSecondary(struct NeRenderPassDesc *passDesc)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
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

	return ctx->cmdBuffer;
}

void
Re_BeginDrawCommandBuffer(void)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	ctx->cmdBuffer = Vkd_AllocateCmdBuffer(ctx->vkDev, VK_COMMAND_BUFFER_LEVEL_PRIMARY, ctx->graphicsPools[Re_frameId]);
	assert(ctx->cmdBuffer);

#ifdef _DEBUG
	char name[64];
	snprintf(name, sizeof(name), "Draw CmdBuffer %u", Re_frameId);
	Vkd_SetObjectName(ctx->vkDev, ctx->cmdBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, name);
#endif

	VkCommandBufferBeginInfo beginInfo =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	vkBeginCommandBuffer(ctx->cmdBuffer, &beginInfo);
}

void
Re_BeginComputeCommandBuffer(void)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	ctx->cmdBuffer = Vkd_AllocateCmdBuffer(ctx->vkDev, VK_COMMAND_BUFFER_LEVEL_PRIMARY, ctx->computePools[Re_frameId]);
	assert(ctx->cmdBuffer);

#ifdef _DEBUG
	char name[64];
	snprintf(name, sizeof(name), "Compute CmdBuffer %u", Re_frameId);
	Vkd_SetObjectName(ctx->vkDev, ctx->cmdBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, name);
#endif

	VkCommandBufferBeginInfo beginInfo =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	vkBeginCommandBuffer(ctx->cmdBuffer, &beginInfo);
}

void
Re_BeginTransferCommandBuffer(void)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	ctx->cmdBuffer = Vkd_AllocateCmdBuffer(ctx->vkDev, VK_COMMAND_BUFFER_LEVEL_PRIMARY, ctx->xferPools[Re_frameId]);
	assert(ctx->cmdBuffer);

#ifdef _DEBUG
	char name[64];
	snprintf(name, sizeof(name), "Xfer CmdBuffer %u", Re_frameId);
	Vkd_SetObjectName(ctx->vkDev, ctx->cmdBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, name);
#endif

	VkCommandBufferBeginInfo beginInfo =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	vkBeginCommandBuffer(ctx->cmdBuffer, &beginInfo);
}

NeCommandBufferHandle
Re_EndCommandBuffer(void)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	VkCommandBuffer cb = ctx->cmdBuffer;

	vkEndCommandBuffer(ctx->cmdBuffer);
	ctx->cmdBuffer = VK_NULL_HANDLE;

	return cb;
}

void
Re_CmdBindPipeline(struct NePipeline *pipeline)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	VkDescriptorSet sets[] = { ctx->descriptorSet, ctx->iaSet};

	vkCmdBindPipeline(ctx->cmdBuffer, pipeline->bindPoint, pipeline->pipeline);
	vkCmdBindDescriptorSets(ctx->cmdBuffer, pipeline->bindPoint, pipeline->layout, 0, ctx->iaSet ? 2 : 1, sets, 0, NULL);

	ctx->boundPipeline = pipeline;
}

void
Re_CmdPushConstants(enum NeShaderStage stage, uint32_t size, const void *data)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdPushConstants(ctx->cmdBuffer, ctx->boundPipeline->layout, stage, 0, size, data);
}

void
Re_BkCmdBindVertexBuffer(struct NeBuffer *buff, uint64_t offset)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdBindVertexBuffers(ctx->cmdBuffer, 0, 1, &buff->buff, &offset);
}

void
Re_BkCmdBindVertexBuffers(uint32_t count, struct NeBuffer **buffers, uint64_t *offsets, uint32_t start)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	VkBuffer *vbuffers = Sys_Alloc(sizeof(*vbuffers), count, MH_Frame);
	for (uint32_t i = 0; i < count; ++i)
		vbuffers[i] = buffers[i]->buff;

	vkCmdBindVertexBuffers(ctx->cmdBuffer, start, count, vbuffers, offsets);
}

void
Re_BkCmdBindIndexBuffer(struct NeBuffer *buff, uint64_t offset, enum NeIndexType type)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdBindIndexBuffer(ctx->cmdBuffer, buff->buff, offset, type);
}

void
Re_CmdExecuteSecondary(NeCommandBufferHandle *cmdBuffers, uint32_t count)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdExecuteCommands(ctx->cmdBuffer, count, (VkCommandBuffer *)cmdBuffers);
}

void
Re_CmdBeginRenderPass(struct NeRenderPassDesc *passDesc, struct NeFramebuffer *fb, enum NeRenderCommandContents contents)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	//if (!Re_deviceInfo.features.coherentMemory)
	//	Vkd_StagingBarrier(ctx->cmdBuffer);

	if (passDesc->inputAttachments) {
		ctx->iaSet = Vk_AllocateIADescriptorSet(ctx->neDev);
		for (uint32_t i = 0; i < passDesc->inputAttachments; ++i)
			Vk_SetInputAttachment(ctx->neDev, ctx->iaSet, i, fb->attachments[fb->attachmentCount - i - 1]);
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
	vkCmdBeginRenderPass(ctx->cmdBuffer, &rpbi, contents);
}

void
Re_CmdEndRenderPass(void)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdEndRenderPass(ctx->cmdBuffer);
	ctx->iaSet = VK_NULL_HANDLE;
}

void
Re_CmdSetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	VkViewport vp = { x, height + y, width, -height, minDepth, maxDepth };
	vkCmdSetViewport(ctx->cmdBuffer, 0, 1, &vp);
}

void
Re_CmdSetScissor(int32_t x, int32_t y, int32_t width, int32_t height)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	VkRect2D scissor = { { x, y }, { width, height } };
	vkCmdSetScissor(ctx->cmdBuffer, 0, 1, &scissor);
}

void
Re_CmdSetLineWidth(float width)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdSetLineWidth(ctx->cmdBuffer, width);
}

void
Re_CmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdDraw(ctx->cmdBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void
Re_CmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdDrawIndexed(ctx->cmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void
Re_CmdDrawIndirect(struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdDrawIndirect(ctx->cmdBuffer, buff->buff, offset, count, stride);
}

void
Re_CmdDrawIndexedIndirect(struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdDrawIndexedIndirect(ctx->cmdBuffer, buff->buff, offset, count, stride);
}

void
Re_CmdDrawMeshTasks(uint32_t taskCount, uint32_t firstTask)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdDrawMeshTasksNV(ctx->cmdBuffer, taskCount, firstTask);
}

void
Re_CmdDrawMeshTasksIndirect(struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdDrawMeshTasksIndirectNV(ctx->cmdBuffer, buff->buff, offset, count, stride);
}

void
Re_CmdDrawMeshTasksIndirectCount(struct NeBuffer *buff, uint64_t offset, struct NeBuffer *countBuff, uint64_t countOffset, uint32_t maxDrawCount, uint32_t stride)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdDrawMeshTasksIndirectCountNV(ctx->cmdBuffer, buff->buff, offset, countBuff->buff, countOffset, maxDrawCount, stride);
}

void
Re_CmdDispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdDispatch(ctx->cmdBuffer, groupCountX, groupCountY, groupCountZ);
}

void
Re_CmdDispatchIndirect(struct NeBuffer *buff, uint64_t offset)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdDispatchIndirect(ctx->cmdBuffer, buff->buff, offset);
}

void
Re_CmdTraceRays(uint32_t width, uint32_t height, uint32_t depth)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdTraceRaysKHR(ctx->cmdBuffer, NULL, NULL, NULL, NULL, width, height, depth);
}

void
Re_CmdTraceRaysIndirect(struct NeBuffer *buff, uint64_t offset)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdTraceRaysIndirectKHR(ctx->cmdBuffer, NULL, NULL, NULL, NULL, Re_BkBufferAddress(buff, offset));
}

void
Re_CmdBuildAccelerationStructures(uint32_t count, struct NeAccelerationStructureBuildInfo *buildInfo, const struct NeAccelerationStructureRangeInfo **rangeInfo)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
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

	vkCmdBuildAccelerationStructuresKHR(ctx->cmdBuffer, count, geomInfo, (const VkAccelerationStructureBuildRangeInfoKHR * const *)rangeInfo);
}

void
Re_Barrier(enum NePipelineDependency dep,
	uint32_t memBarrierCount, const struct NeMemoryBarrier *memBarriers,
	uint32_t bufferBarrierCount, const struct NeBufferBarrier *bufferBarriers,
	uint32_t imageBarrierCount, const struct NeImageBarrier *imageBarriers)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
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
			vkImageBarriers[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR;
			vkImageBarriers[i].pNext = NULL;

			vkImageBarriers[i].srcStageMask = imageBarriers[i].srcStage;
			vkImageBarriers[i].dstStageMask = imageBarriers[i].dstStage;

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
	vkCmdPipelineBarrier2KHR(ctx->cmdBuffer, &di);
}

void
Re_CmdTransition(struct NeTexture *tex, enum NeTextureLayout newLayout)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	Vkd_TransitionImageLayout(ctx->cmdBuffer, tex->image, tex->layout, NeToVkImageLayout(newLayout));
	tex->layout = NeToVkImageLayout(newLayout);
}

void
Re_BkCmdCopyBuffer(const struct NeBuffer *src, uint64_t srcOffset, struct NeBuffer *dst, uint64_t dstOffset, uint64_t size)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	VkBufferCopy r =
	{
		.srcOffset = srcOffset,
		.dstOffset = dstOffset,
		.size = size
	};
	vkCmdCopyBuffer(ctx->cmdBuffer, src->buff, dst->buff, 1, &r);
}

void
Re_CmdCopyImage(const struct NeTexture *src, struct NeTexture *dst)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdCopyImage(ctx->cmdBuffer, src->image, src->layout, dst->image, dst->layout, 1, NULL);
}

void
Re_BkCmdCopyBufferToTexture(const struct NeBuffer *src, struct NeTexture *dst, const struct NeBufferImageCopy *bic)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
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
	Vkd_TransitionImageLayoutRange(ctx->cmdBuffer, dst->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, dst->layout, &range);
}

void
Re_BkCmdCopyTextureToBuffer(const struct NeTexture *src, struct NeBuffer *dst, const struct NeBufferImageCopy *bic)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
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

void
Re_CmdBlit(const struct NeTexture *src, struct NeTexture *dst, const struct NeBlitRegion *regions, uint32_t regionCount, enum NeImageFilter filter)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	VkFilter f = VK_FILTER_NEAREST;

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

	vkCmdBlitImage(ctx->cmdBuffer, src->image, src->layout, dst->image, dst->layout, regionCount, r, f);
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
Re_QueueGraphics(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *wait, struct NeSemaphore *signal)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	struct Vkd_SubmitInfo si =
	{
		.wait = wait ? wait->sem : VK_NULL_HANDLE,
		.waitValue = wait ? wait->value : 0,
		.signal = signal ? signal->sem : VK_NULL_HANDLE,
		.signalValue = signal ? ++signal->value : 0,
		.cmdBuffer = (VkCommandBuffer)cmdBuffer
	};
	Rt_ArrayAddPtr(&ctx->graphicsCmdBuffers[Re_frameId], cmdBuffer);
	return Rt_ArrayAdd(&ctx->queued.graphics, &si);
}

bool
Re_QueueCompute(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *wait, struct NeSemaphore *signal)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	struct Vkd_SubmitInfo si =
	{
		.wait = wait ? wait->sem : VK_NULL_HANDLE,
		.waitValue = wait ? wait->value : 0,
		.signal = signal ? signal->sem : VK_NULL_HANDLE,
		.signalValue = signal ? ++signal->value : 0,
		.cmdBuffer = (VkCommandBuffer)cmdBuffer
	};
	Rt_ArrayAddPtr(&ctx->computeCmdBuffers[Re_frameId], cmdBuffer);
	return Rt_ArrayAdd(&ctx->queued.compute, &si);
}

bool
Re_QueueTransfer(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *wait, struct NeSemaphore *signal)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	struct Vkd_SubmitInfo si =
	{
		.wait = wait ? wait->sem : VK_NULL_HANDLE,
		.waitValue = wait ? wait->value : 0,
		.signal = signal ? signal->sem : VK_NULL_HANDLE,
		.signalValue = signal ? ++signal->value : 0,
		.cmdBuffer = (VkCommandBuffer)cmdBuffer
	};
	Rt_ArrayAddPtr(&ctx->xferCmdBuffers[Re_frameId], cmdBuffer);
	return Rt_ArrayAdd(&ctx->queued.xfer, &si);
}

bool
Re_ExecuteGraphics(NeCommandBufferHandle cmdBuffer)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	bool rc = false;
	VkFence f;
	VkFenceCreateInfo fci = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	vkCreateFence(ctx->vkDev, &fci, Vkd_allocCb, &f);

	VkCommandBufferSubmitInfoKHR cbsi =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR,
		.commandBuffer = (VkCommandBuffer)cmdBuffer
	};
	VkSubmitInfo2KHR si =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cbsi
	};
	if (Vkd_QueueSubmit(&ctx->neDev->graphics, 1, &si, f) != VK_SUCCESS)
		goto exit;

	rc = vkWaitForFences(ctx->vkDev, 1, &f, VK_TRUE, UINT64_MAX) == VK_SUCCESS;
	vkFreeCommandBuffers(ctx->vkDev, ctx->graphicsPools[Re_frameId], 1, (VkCommandBuffer *)&cmdBuffer);

exit:
	vkDestroyFence(ctx->vkDev, f, Vkd_allocCb);

	return rc;
}

bool
Re_ExecuteCompute(NeCommandBufferHandle cmdBuffer)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	bool rc = false;
	VkFence f;
	VkFenceCreateInfo fci = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	vkCreateFence(ctx->vkDev, &fci, Vkd_allocCb, &f);

	VkCommandBufferSubmitInfoKHR cbsi =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR,
		.commandBuffer = (VkCommandBuffer)cmdBuffer
	};
	VkSubmitInfo2KHR si =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cbsi
	};
	if (Vkd_QueueSubmit(&ctx->neDev->compute, 1, &si, f) != VK_SUCCESS)
		goto exit;

	rc = vkWaitForFences(ctx->vkDev, 1, &f, VK_TRUE, UINT64_MAX) == VK_SUCCESS;
	vkFreeCommandBuffers(ctx->vkDev, ctx->computePools[Re_frameId], 1, (VkCommandBuffer *)&cmdBuffer);

exit:
	vkDestroyFence(ctx->vkDev, f, Vkd_allocCb);

	return rc;
}

bool
Re_ExecuteTransfer(NeCommandBufferHandle cmdBuffer)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	bool rc = false;
	VkFence f;
	VkFenceCreateInfo fci = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	vkCreateFence(ctx->vkDev, &fci, Vkd_allocCb, &f);

	VkCommandBufferSubmitInfoKHR cbsi =
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR,
		.commandBuffer = (VkCommandBuffer)cmdBuffer
	};
	VkSubmitInfo2KHR si =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cbsi
	};
	if (Vkd_QueueSubmit(&ctx->neDev->transfer, 1, &si, f) != VK_SUCCESS)
		goto exit;

	rc = vkWaitForFences(ctx->vkDev, 1, &f, VK_TRUE, UINT64_MAX) == VK_SUCCESS;
	vkFreeCommandBuffers(ctx->vkDev, ctx->xferPools[Re_frameId], 1, (VkCommandBuffer *)&cmdBuffer);

exit:
	vkDestroyFence(ctx->vkDev, f, Vkd_allocCb);

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

static inline uint32_t
_queueFamilyIndex(struct NeRenderDevice *dev, enum NeRenderQueue queue)
{
	switch (queue) {
	case RE_QUEUE_GRAPHICS: return dev->graphics.family;
	case RE_QUEUE_TRANSFER: return dev->transfer.family;
	case RE_QUEUE_COMPUTE: return dev->compute.family;
	}

	return 0;
}

void
_Submit(struct VkdRenderQueue *queue, struct NeArray *submitInfo,
		uint32_t waitCount, VkSemaphoreSubmitInfoKHR *wait,
		uint32_t signalCount, VkSemaphoreSubmitInfoKHR *signal)
{
	if (!submitInfo->count)
		return;

	struct NeArray info, cbInfo, sInfo;
	Rt_InitArray(&info, submitInfo->count, sizeof(VkSubmitInfo2), MH_Transient);
	Rt_InitArray(&cbInfo, submitInfo->count, sizeof(VkCommandBufferSubmitInfoKHR), MH_Transient);
	Rt_InitArray(&sInfo, submitInfo->count * 2 /* wait & signal */, sizeof(VkSemaphoreSubmitInfoKHR), MH_Transient);

	struct Vkd_SubmitInfo *esi;
	Rt_ArrayForEach(esi, submitInfo) {
		VkSubmitInfo2 *si = Rt_ArrayAllocate(&info);
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
		}
	}

	Vkd_QueueSubmit(queue, (uint32_t)info.count, (const VkSubmitInfo2KHR *)info.data, VK_NULL_HANDLE);
}
