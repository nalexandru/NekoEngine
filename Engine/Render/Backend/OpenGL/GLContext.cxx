#include <assert.h>

#include <Math/Math.h>
#include <System/Memory.h>
#include <Runtime/Runtime.h>

#include "GLBackend.h"

static inline void BeginCommandBuffer();

struct NeRenderContext *
Re_CreateContext(void)
{
	NeRenderContext *ctx = (NeRenderContext *)Sys_Alloc(1, sizeof(*ctx), MH_RenderBackend);
	if (!ctx)
		return NULL;

	if (!Rt_InitPtrArray(&ctx->queued, 10, MH_RenderBackend))
		goto error;

	if (!Rt_InitPtrQueue(&ctx->free, 10, MH_RenderBackend))
		goto error;

	// This will return NULL if the implementation doesn't support shared contexts; It is not an error.
	ctx->glContext = GLBk_CreateShareContext();

	return ctx;

error:
	Rt_TermArray(&ctx->queued);
	Rt_TermQueue(&ctx->free);

	GLBk_DestroyShareContext(ctx->glContext);

	return NULL;
}

void
Re_ResetContext(struct NeRenderContext *ctx)
{
}

void
GLBk_ExecuteCommands()
{
	GLCommandBuffer *cmdBuff;
	for (uint32_t i = 0; i < RE_NUM_CONTEXTS; ++i) {
		Rt_ArrayForEachPtr(cmdBuff, &Re_contexts[i]->queued, GLCommandBuffer *) {
			ExecuteCommands(cmdBuff);
			Rt_QueuePushPtr(&Re_contexts[i]->free, cmdBuff);
		}
	}
}

void
Re_DestroyContext(struct NeRenderContext *ctx)
{
	Rt_TermArray(&ctx->queued);
	Rt_TermQueue(&ctx->free);

	GLBk_DestroyShareContext(ctx->glContext);

	Sys_Free(ctx);
}

NeCommandBufferHandle
Re_BeginSecondary(struct NeRenderPassDesc *passDesc)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	ctx->cmdBuffer = (GLCommandBuffer *)Sys_Alloc(sizeof(*ctx->cmdBuffer), 1, MH_RenderBackend);
	new (ctx->cmdBuffer)GLCommandBuffer();

	InitCommandBuffer(ctx->cmdBuffer);
	return ctx->cmdBuffer;
}

void
BeginCommandBuffer()
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	if (ctx->free.count) {
		ctx->cmdBuffer = (GLCommandBuffer *)Rt_QueuePopPtr(&ctx->free);
		ResetCommandBuffer(ctx->cmdBuffer);
	} else {
		ctx->cmdBuffer = (GLCommandBuffer *)Sys_Alloc(sizeof(*ctx->cmdBuffer), 1, MH_RenderBackend);
		new (ctx->cmdBuffer)GLCommandBuffer();
		InitCommandBuffer(ctx->cmdBuffer);
	}
}

void Re_BeginDrawCommandBuffer(struct NeSemaphore *wait) { BeginCommandBuffer(); }
void Re_BeginComputeCommandBuffer(struct NeSemaphore *wait) { BeginCommandBuffer(); }
void Re_BeginTransferCommandBuffer(struct NeSemaphore *wait) { BeginCommandBuffer(); }

NeCommandBufferHandle
Re_EndCommandBuffer(void)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	EndCommandBuffer(ctx->cmdBuffer);

	NeCommandBufferHandle cb = ctx->cmdBuffer;
	ctx->cmdBuffer = NULL;

	return cb;
}

void
Re_CmdBindPipeline(struct NePipeline *pipeline)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
		const NePipeline *p = (const NePipeline *)cb->arg<void *>(0);

		GL_TRACE(glUseProgram(p->program));
	}, (void *)pipeline);

	ctx->boundPipeline = pipeline;
}

void
Re_CmdPushConstants(NeShaderStageFlags stage, uint32_t size, const void *data)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	assert(size <= sizeof(ctx->cmdBuffer->pushConstants));
	memcpy((void *)&ctx->cmdBuffer->pushConstants, data, size);

	RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
		const NePipeline *p = (const NePipeline *)cb->arg<void *>(0);

		uint32_t tu = 0;
		uint8_t *pc = ((struct GLCommandBuffer *)cb)->pushConstants;
		struct GLBkPipelineUniform *pu;
		Rt_ArrayForEach(pu, &p->uniforms, struct GLBkPipelineUniform *) {
			switch (pu->type) {
			case UT_VEC3: {
				GL_TRACE(glUniform3fv(pu->location, 1, (GLfloat *)pc));
				pc += pu->size;
			} break;
			case UT_FLOAT: {
				GL_TRACE(glUniform1fv(pu->location, 1, (GLfloat *)pc));
				pc += pu->size;
			} break;
			case UT_TEX2D: {
				GL_TRACE(glActiveTexture(GL_TEXTURE0 + tu));
				GL_TRACE(glBindTexture(GL_TEXTURE_2D, GLBk_textures[*(uint32_t *)pc]));
				GL_TRACE(glUniform1i(pu->location, tu++));

				pc += pu->size;
			} break;
			case UT_VEC4: {
				GL_TRACE(glUniform4fv(pu->location, 1, (GLfloat *)pc));
				pc += pu->size;
			} break;
			case UT_MAT4: {
				GL_TRACE(glUniformMatrix4fv(pu->location, 1, false, (GLfloat *)pc));
				pc += pu->size;
			} break;
			case UT_NONE: pc += pu->size; break;
			case UT_INT: {
				GL_TRACE(glUniform1iv(pu->location, 1, (GLint *)pc));
				pc += pu->size;
			} break;
			case UT_UINT: {
				GL_TRACE(glUniform1uivEXT(pu->location, 1, (GLuint *)pc));
				pc += pu->size;
			} break;
			case UT_TEX3D: {
				GL_TRACE(glActiveTexture(GL_TEXTURE0 + tu));
				GL_TRACE(glBindTexture(GL_TEXTURE_3D, GLBk_textures[*(uint32_t *)pc]));
				GL_TRACE(glUniform1i(pu->location, tu++));

				pc += pu->size;
			} break;
			case UT_MAT3: {
				GL_TRACE(glUniformMatrix3fv(pu->location, 1, false, (GLfloat *)pc));
				pc += pu->size;
			} break;
			}
		}
	}, (void *)ctx->boundPipeline);
}

void
Re_BkCmdBindVertexBuffer(struct NeBuffer *buff, uint64_t offset)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
		GL_TRACE(glBindBuffer(GL_ARRAY_BUFFER, cb->arg<GLuint>(0)));
	}, buff->id);
}

void
Re_BkCmdBindVertexBuffers(uint32_t count, struct NeBuffer **buffers, uint64_t *offsets, uint32_t start)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	for (uint32_t i = 0; i < count; ++i)
		RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
			GL_TRACE(glBindBuffer(GL_ARRAY_BUFFER, cb->arg<GLuint>(0)));
		}, buffers[i]->id);
}

void
Re_BkCmdBindIndexBuffer(struct NeBuffer *buff, uint64_t offset, enum NeIndexType type)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
		GL_TRACE(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cb->arg<GLuint>(0)));
	}, buff->id);
}

void
Re_CmdExecuteSecondary(NeCommandBufferHandle *cmdBuffers, uint32_t count)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	for (uint32_t i = 0; i < count; ++i)
		RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
			ExecuteCommands((NeCommandBuffer *)cb->arg<void *>(0));
		}, (void *)cmdBuffers[i]);
}

void
Re_CmdBeginRenderPass(struct NeRenderPassDesc *passDesc, struct NeFramebuffer *fb, enum NeRenderCommandContents contents)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	/*if (passDesc->clearValueCount) {

	}*/

	// RecordCommand(ctx->cmdBuffer, glClear, 1, 'i', GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
		struct NeRenderPassDesc *pd = (struct NeRenderPassDesc *)cb->arg<void *>(0);

		GL_TRACE(glBindFramebuffer(GL_FRAMEBUFFER, cb->arg<GLuint>(1)));
		const GLenum rc = GL_TRACE(glCheckFramebufferStatus(GL_FRAMEBUFFER));

		if (rc != GL_FRAMEBUFFER_COMPLETE) {
			// TODO
		}

		XMVECTOR devClearColor = XMLoadFloat4((XMFLOAT4 *)Re_device->clearColor);
		for (uint32_t i = 0; i < pd->count; ++i) {
			const struct GLBkRenderPassAttachment *at = &pd->color[i];

			if (false) {
				// TODO: glClearBufferiv, glClearTexImage
			} else {
				if (at->clear) {
					XMVECTOR clearColor = XMLoadFloat4((XMFLOAT4 *)at->clearColor);
					if (!XMVector4Equal(devClearColor, clearColor)) {
						GL_TRACE(glClearColor(at->clearColor[0], at->clearColor[1], at->clearColor[2], at->clearColor[3]));
						memcpy(Re_device->clearColor, at->clearColor, sizeof(Re_device->clearColor));
					}

					GL_TRACE(glDrawBuffer(GL_COLOR_ATTACHMENT0 + i));
					GL_TRACE(glClear(GL_COLOR_BUFFER_BIT));
				}
			}

			// TODO: glDiscardFramebufferEXT
		}
		GL_TRACE(glDrawBuffers(pd->count, pd->drawBuffers));

		if (pd->hasDepth) {
			if (pd->depth.clear) {
				if (!XMScalarNearEqual(Re_device->clearDepth, pd->depth.clearDepth, FLT_EPSILON)) {
					GL_TRACE(glClearDepthf(pd->depth.clearDepth));
					Re_device->clearDepth = pd->depth.clearDepth;
				}

				GL_TRACE(glClear(GL_DEPTH_BUFFER_BIT));
			}

			// TODO: glDiscardFramebufferEXT
		}

		// TODO: read
	}, (void *)passDesc, fb->id);

	ctx->boundRenderPass = passDesc;
}

void
Re_CmdEndRenderPass(void)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void { glBindFramebuffer(GL_FRAMEBUFFER, 0); });
}

void
Re_CmdSetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
		GL_TRACE(glViewport(cb->arg<int32_t>(0), cb->arg<int32_t>(1), cb->arg<int32_t>(2), cb->arg<int32_t>(3)));
		GL_TRACE(glDepthRangef(cb->arg<float>(4), cb->arg<float>(5)));
	}, (int32_t)x, (int32_t)y, (int32_t)width, (int32_t)height, minDepth, maxDepth);
}

void
Re_CmdSetScissor(int32_t x, int32_t y, int32_t width, int32_t height)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
		GL_TRACE(glScissor(cb->arg<int32_t>(0), cb->arg<int32_t>(1), cb->arg<int32_t>(2), cb->arg<int32_t>(3)));
	}, x, y, width, height);
}

void
Re_CmdSetLineWidth(float width)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
		GL_TRACE(glLineWidth(cb->arg<float>(0)));
	}, width);
}

void
Re_CmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	//RecordCommand(ctx->cmdBuffer, (void(*)())glLineWidth, 1, 'f', width);

	/*struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdDraw(ctx->cmdBuffer, vertexCount, instanceCount, firstVertex, firstInstance);*/
	GL_TRACE(glDrawArrays(GL_TRIANGLES, 0, 3));							// 3 int
	//glDrawArraysInstancedARB(GL_TRIANGLES, )

	/*RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cb->arg<GLuint>(0));
	}, buff->id);*/
}

void
Re_CmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	//RecordCommand(ctx->cmdBuffer, (void(*)())glLineWidth, 1, 'f', width);

	/*struct NeRenderContext *ctx = Re_CurrentContext();
	vkCmdDrawIndexed(ctx->cmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);*/
//	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void *)firstIndex);
//	glDrawElementsBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void *)firstIndex, vertexOffset);
//	glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void *)firstIndex, instanceCount, vertexOffset, firstInstance);

	// 4-7 int

	/*RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cb->arg<GLuint>(0));
	}, buff->id);*/
}

void
Re_CmdDrawIndirect(struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	//RecordCommand(ctx->cmdBuffer, (void(*)())glLineWidth, 1, 'f', width);
	//struct NeRenderContext *ctx = Re_CurrentContext();
	//vkCmdDrawIndirect(ctx->cmdBuffer, buff->buff, offset, count, stride);

	/*RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cb->arg<GLuint>(0));
	}, buff->id);*/
}

void
Re_CmdDrawIndexedIndirect(struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	//RecordCommand(ctx->cmdBuffer, (void(*)())glLineWidth, 1, 'f', width);
	//struct NeRenderContext *ctx = Re_CurrentContext();
	//vkCmdDrawIndexedIndirect(ctx->cmdBuffer, buff->buff, offset, count, stride);

	//glMultiDrawElementsIndirect()

	/*RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cb->arg<GLuint>(0));
	}, buff->id);*/
}

void
Re_CmdDrawMeshTasks(uint32_t taskCount, uint32_t firstTask)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
		GL_TRACE(glDrawMeshTasksNV(cb->arg<GLuint>(0), cb->arg<GLuint>(1)));
	}, taskCount, firstTask);
}

void
Re_CmdDrawMeshTasksIndirect(struct NeBuffer *buff, uint64_t offset, uint32_t count, uint32_t stride)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
		GL_TRACE(glMultiDrawMeshTasksIndirectNV(cb->arg<GLuint>(0), cb->arg<GLsizei>(1), cb->arg<GLuint>(2)));
	}, buff->id, (GLsizei)count, stride);
}

void
Re_CmdDrawMeshTasksIndirectCount(struct NeBuffer *buff, uint64_t offset, struct NeBuffer *countBuff, uint64_t countOffset, uint32_t maxDrawCount, uint32_t stride)
{
	// TODO

	//struct NeRenderContext *ctx = Re_CurrentContext();
	//RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
	//	glMultiDrawMeshTasksIndirectNV(cb->arg<GLuint>(0), cb->arg<GLsizei>(1), cb->arg<GLuint>(2));
	//}, buff->id, (GLsizei)count, stride);
}

void
Re_CmdDispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
		GL_TRACE(glDispatchCompute(cb->arg<GLuint>(0), cb->arg<GLuint>(1), cb->arg<GLuint>(2)));
	}, groupCountX, groupCountY, groupCountZ);
}

void
Re_CmdDispatchIndirect(struct NeBuffer *buff, uint64_t offset)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
		GL_TRACE(glDispatchComputeIndirect(cb->arg<GLuint>(0)));
	}, buff->id);
}

void
Re_BkCmdUpdateBuffer(const struct NeBuffer *buff, uint64_t offset, void *data, uint64_t size)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	if (GLAD_GL_ARB_direct_state_access)
		RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
			GL_TRACE(glNamedBufferSubData(cb->arg<GLuint>(0), cb->arg<GLintptr>(1), cb->arg<GLsizeiptr>(2), cb->arg<void *>(3)));
		}, buff->id, (GLintptr)offset, (GLsizeiptr)size, data);
	else
		RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
			const GLenum bindPoint = cb->arg<GLenum>(4);
			GL_TRACE(glBindBuffer(bindPoint, cb->arg<GLuint>(0)));
			GL_TRACE(glBufferSubData(bindPoint, cb->arg<GLintptr>(1), cb->arg<GLsizeiptr>(2), cb->arg<void *>(3)));
			GL_TRACE(glBindBuffer(bindPoint, 0));
		}, buff->id, (GLintptr)offset, (GLsizeiptr)size, data, buff->bindPoint);
}

void
Re_BkCmdCopyBuffer(const struct NeBuffer *src, uint64_t srcOffset, struct NeBuffer *dst, uint64_t dstOffset, uint64_t size)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	/*
	 * if (glNamedBufferSubData)
		RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
			glNamedBufferSubData(cb->arg<GLuint>(0), cb->arg<GLintptr>(1), cb->arg<GLsizeiptr>(2), cb->arg<void *>(3));
		}, buff->id, (GLintptr)offset, (GLsizeiptr)size, data);
	else
		RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
			const GLenum bindPoint = cb->arg<GLenum>(4);
			glBindBuffer(bindPoint, cb->arg<GLuint>(0));
			glBufferSubData(bindPoint, cb->arg<GLintptr>(1), cb->arg<GLsizeiptr>(2), cb->arg<void *>(3));
			glBindBuffer(bindPoint, 0);
		}, buff->id, (GLintptr)offset, (GLsizeiptr)size, data, buff->bindPoint);
	 */
}

void
Re_CmdCopyImage(const struct NeTexture *src, struct NeTexture *dst)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	/*
	 * if (glNamedBufferSubData)
		RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
			glNamedBufferSubData(cb->arg<GLuint>(0), cb->arg<GLintptr>(1), cb->arg<GLsizeiptr>(2), cb->arg<void *>(3));
		}, buff->id, (GLintptr)offset, (GLsizeiptr)size, data);
	else
		RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
			const GLenum bindPoint = cb->arg<GLenum>(4);
			glBindBuffer(bindPoint, cb->arg<GLuint>(0));
			glBufferSubData(bindPoint, cb->arg<GLintptr>(1), cb->arg<GLsizeiptr>(2), cb->arg<void *>(3));
			glBindBuffer(bindPoint, 0);
		}, buff->id, (GLintptr)offset, (GLsizeiptr)size, data, buff->bindPoint);
	 */
}

void
Re_BkCmdCopyBufferToTexture(const struct NeBuffer *src, struct NeTexture *dst, const struct NeBufferImageCopy *bic)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
		struct NeBufferImageCopy *bic = (struct NeBufferImageCopy *)cb->arg<void *>(2);
		GLBk_UploadImage((struct NeTexture *)cb->arg<void *>(1), (const struct NeBuffer *)cb->arg<void *>(0), bic);
	}, (void *)src, (void *)dst, Rt_MemDup(bic, sizeof(*bic), MH_Frame));
}

void
Re_BkCmdCopyTextureToBuffer(const struct NeTexture *src, struct NeBuffer *dst, const struct NeBufferImageCopy *bic)
{
	struct NeRenderContext *ctx = Re_CurrentContext();

	/*
	 * if (glNamedBufferSubData)
		RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
			glNamedBufferSubData(cb->arg<GLuint>(0), cb->arg<GLintptr>(1), cb->arg<GLsizeiptr>(2), cb->arg<void *>(3));
		}, buff->id, (GLintptr)offset, (GLsizeiptr)size, data);
	else
		RecordCommand(ctx->cmdBuffer, [](struct NeCommandBuffer *cb) -> void {
			const GLenum bindPoint = cb->arg<GLenum>(4);
			glBindBuffer(bindPoint, cb->arg<GLuint>(0));
			glBufferSubData(bindPoint, cb->arg<GLintptr>(1), cb->arg<GLsizeiptr>(2), cb->arg<void *>(3));
			glBindBuffer(bindPoint, 0);
		}, buff->id, (GLintptr)offset, (GLsizeiptr)size, data, buff->bindPoint);
	 */
}

void
Re_CmdBlit(const struct NeTexture *src, struct NeTexture *dst, const struct NeBlitRegion *regions, uint32_t regionCount, enum NeImageFilter filter)
{
	//glBlitFramebuffer()	// 10 int
}

bool
Re_QueueGraphics(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *signal)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	return Rt_ArrayAddPtr(&ctx->queued, cmdBuffer);
}

bool
Re_QueueCompute(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *signal)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	return Rt_ArrayAddPtr(&ctx->queued, cmdBuffer);
}

bool
Re_QueueTransfer(NeCommandBufferHandle cmdBuffer, struct NeSemaphore *signal)
{
	struct NeRenderContext *ctx = Re_CurrentContext();
	return Rt_ArrayInsertPtr(&ctx->queued, cmdBuffer, 0);
}

bool
Re_ExecuteGraphics(NeCommandBufferHandle cmdBuffer)
{
	ExecuteCommands((NeCommandBuffer *)cmdBuffer);
	Re_WaitIdle();

	struct NeRenderContext *ctx = Re_CurrentContext();
	Rt_QueuePushPtr(&ctx->free, cmdBuffer);

	return true;
}

bool
Re_ExecuteCompute(NeCommandBufferHandle cmdBuffer)
{
	ExecuteCommands((NeCommandBuffer *)cmdBuffer);
	Re_WaitIdle();

	struct NeRenderContext *ctx = Re_CurrentContext();
	Rt_QueuePushPtr(&ctx->free, cmdBuffer);

	return true;
}

bool
Re_ExecuteTransfer(NeCommandBufferHandle cmdBuffer)
{
	ExecuteCommands((NeCommandBuffer *)cmdBuffer);
	Re_WaitIdle();

	struct NeRenderContext *ctx = Re_CurrentContext();
	Rt_QueuePushPtr(&ctx->free, cmdBuffer);

	return true;
}

/* NekoEngine
 *
 * GLContext.c
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
