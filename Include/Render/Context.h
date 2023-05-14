#ifndef NE_RENDER_DRIVER_CONTEXT_H
#define NE_RENDER_DRIVER_CONTEXT_H

#include <Engine/Job.h>
#include <Render/Types.h>
#include <System/Thread.h>

#ifdef __cplusplus
extern "C" {
#endif

struct NeImageSubresource
{
	enum NeImageAspect aspect;
	uint32_t mipLevel;
	uint32_t baseArrayLayer;
	uint32_t layerCount;
	uint32_t levelCount;
};

struct NeBlitRegion
{
	struct NeImageSubresource srcSubresource;
	struct {
		int32_t x, y, z;
	} srcOffset;
	struct {
		int32_t width, height, depth;
	} srcSize;
	struct NeImageSubresource dstSubresource;
	struct {
		int32_t x, y, z;
	} dstOffset;
	struct {
		int32_t width, height, depth;
	} dstSize;
};

struct NeImageOffset
{
	int32_t x, y, z;
};

struct NeImageSize
{
	int32_t width, height, depth;
};

struct NeBufferImageCopy
{
	uint64_t bufferOffset;
	uint32_t bytesPerRow;
	uint32_t rowLength;
	uint32_t imageHeight;
	struct NeImageSubresource subresource;
	struct NeImageOffset imageOffset;
	struct NeImageSize imageSize;
};

#define RE_NUM_CONTEXTS		E_JobWorkerThreads() + 1

ENGINE_API extern struct NeRenderDevice *Re_device;
extern THREAD_LOCAL struct NeRenderContext *Re_context;
ENGINE_API extern struct NeRenderContext **Re_contexts;

struct NeRenderContext *Re_CreateContext(void);
void Re_ResetContext(struct NeRenderContext *ctx);
void Re_DestroyContext(struct NeRenderContext *ctx);

static inline struct NeRenderContext *
Re_CurrentContext(void)
{
	if (!Re_context)
		Re_context = Re_contexts[E_WorkerId()];
	return Re_context;
}

NeCommandBufferHandle Re_BeginSecondary(struct NeRenderPassDesc *rpd);
void Re_BeginDrawCommandBuffer(struct NeSemaphore *wait);
void Re_BeginComputeCommandBuffer(struct NeSemaphore *wait);
void Re_BeginTransferCommandBuffer(struct NeSemaphore *wait);
NeCommandBufferHandle Re_EndCommandBuffer(void);

void Re_CmdBindVertexBuffer(NeBufferHandle handle, uint64_t offset);
void Re_CmdBindVertexBuffers(uint32_t count, NeBufferHandle *handles, uint64_t *offsets, uint32_t start);
void Re_CmdBindIndexBuffer(NeBufferHandle handle, uint64_t offset, enum NeIndexType type);

void Re_CmdBindPipeline(struct NePipeline *pipeline);
void Re_CmdPushConstants(NeShaderStageFlags stage, uint32_t size, const void *data);

void Re_CmdExecuteSecondary(NeCommandBufferHandle *cmdBuffers, uint32_t count);

void Re_CmdBeginRenderPass(struct NeRenderPassDesc *passDesc, struct NeFramebuffer *fb, enum NeRenderCommandContents contents);
void Re_CmdEndRenderPass(void);

void Re_CmdSetViewport(float x, float y, float width, float height, float minDepth, float maxDepth);
void Re_CmdSetScissor(int32_t x, int32_t y, int32_t width, int32_t height);
void Re_CmdSetLineWidth(float width);

void Re_CmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
void Re_CmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);
void Re_CmdDrawIndirect(struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride);
void Re_CmdDrawIndexedIndirect(struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride);

void Re_CmdDrawMeshTasks(uint32_t taskCount, uint32_t firstTask);
void Re_CmdDrawMeshTasksIndirect(struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride);
void Re_CmdDrawMeshTasksIndirectCount(struct NeBuffer *buff, uint64_t offset, struct NeBuffer *countBuff,
										uint64_t countOffset, uint32_t maxDrawCount, uint32_t stride);

void Re_CmdDispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
void Re_CmdDispatchIndirect(struct NeBuffer *buff, uint64_t offset);

void Re_CmdTraceRays(uint32_t width, uint32_t height, uint32_t depth);
void Re_CmdTraceRaysIndirect(struct NeBuffer *buff, uint64_t offset);

void Re_CmdBlit(const struct NeTexture *src, struct NeTexture *dst, const struct NeBlitRegion *regions,
					uint32_t regionCount, enum NeImageFilter filter);

void Re_CmdLoadBuffer(struct NeBuffer *buff, uint64_t offset, uint64_t size, void *handle, uint64_t sourceOffset);
void Re_CmdLoadTexture(struct NeTexture *tex, uint32_t slice, uint32_t level, uint32_t width, uint32_t height, uint32_t depth,
						uint32_t bytesPerRow, struct NeImageOffset *origin, void *handle, uint64_t sourceOffset);

bool Re_QueueCompute(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *signal);
bool Re_QueueGraphics(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *signal);
bool Re_QueueTransfer(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *signal);

bool Re_ExecuteCompute(NeCommandBufferHandle cmdBuffer);
bool Re_ExecuteGraphics(NeCommandBufferHandle cmdBuffer);
bool Re_ExecuteTransfer(NeCommandBufferHandle cmdBuffer);

void Re_BeginDirectIO(void);
bool Re_SubmitDirectIO(bool *completed);
bool Re_ExecuteDirectIO(void);

void Re_CmdBarrier(enum NePipelineDependency dep,
					uint32_t memBarrierCount, const struct NeMemoryBarrier *memBarriers,
					uint32_t bufferBarrierCount, const struct NeBufferBarrier *bufferBarriers,
					uint32_t imageBarrierCount, const struct NeImageBarrier *imageBarriers);

#ifdef __cplusplus
}
#endif

#endif /* NE_RENDER_DRIVER_CONTEXT_H */

/* NekoEngine
 *
 * Context.h
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
