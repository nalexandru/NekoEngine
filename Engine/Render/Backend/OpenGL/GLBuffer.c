#include <stdlib.h>

#include "GLBackend.h"

bool
GLBk_CreateBuffer(const struct NeBufferDesc *desc, struct NeBuffer *buff)
{
	if (desc->usage == (BU_TRANSFER_SRC | BU_TRANSFER_DST) && desc->memoryType == MT_CPU_WRITE) {
		buff->staging = Sys_Alloc(desc->size, 1, MH_RenderBackend);
		return buff->staging != NULL;
	}

	if ((desc->usage & BU_VERTEX_BUFFER) == BU_VERTEX_BUFFER)
		buff->bindPoint = GL_ARRAY_BUFFER;
	else if ((desc->usage & BU_INDEX_BUFFER) == BU_INDEX_BUFFER)
		buff->bindPoint = GL_ELEMENT_ARRAY_BUFFER;
	else if ((desc->usage & BU_UNIFORM_BUFFER) == BU_UNIFORM_BUFFER)
		buff->bindPoint = GL_UNIFORM_BUFFER;
	else if ((desc->usage & BU_STORAGE_BUFFER))
		buff->bindPoint = GL_SHADER_STORAGE_BUFFER;
	else
		buff->bindPoint = GL_ARRAY_BUFFER;

	switch (desc->memoryType) {
	case MT_GPU_LOCAL:
		buff->flags = 0;
		buff->usage = GL_STATIC_DRAW;
	break;
	case MT_CPU_WRITE:
		buff->flags = GL_MAP_WRITE_BIT;
		buff->flags = GL_DYNAMIC_DRAW;
	break;
	case MT_CPU_READ:
		buff->flags = GL_MAP_READ_BIT;
		buff->usage = GL_DYNAMIC_READ;
	break;
	case MT_CPU_COHERENT:
		buff->flags = GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		buff->usage = GL_DYNAMIC_COPY;
	break;
	}

	if (GLAD_GL_ARB_direct_state_access) {
		GL_TRACE(glCreateBuffers(1, &buff->id));
		if (!buff->id)
			return false;

		GL_TRACE(glNamedBufferStorage(buff->id, desc->size, NULL, buff->flags));
		if (desc->memoryType == MT_CPU_COHERENT) {
			buff->persistent = GL_TRACE(glMapNamedBufferRange(buff->id, 0, desc->size, buff->flags));
		}
	} else {
		GL_TRACE(glGenBuffers(1, &buff->id));
		if (!buff->id)
			return false;

		GL_TRACE(glBindBuffer(buff->bindPoint, buff->id));

		if (glBufferStorage) {
			GL_TRACE(glBufferStorage(buff->bindPoint, desc->size, NULL, buff->flags));
			if (desc->memoryType == MT_CPU_COHERENT) {
				buff->persistent = GL_TRACE(glMapBufferRange(buff->bindPoint, 0, desc->size, buff->flags));
			}
		} else {
			GL_TRACE(glBufferData(buff->bindPoint, desc->size, NULL, buff->usage));
		}

		GL_TRACE(glBindBuffer(buff->bindPoint, 0));
	}

	/*if (glMakeBufferResidentNV) {
		glMakeBufferResidentNV()
	}
	glGetNamedBufferParameterui64vNV(buff->id, GL_BUFFER_GPU_ADDRESS_NV, &buff->handle);
	glGetBufferParameterui64vNV(buff->bindPoint, GL_BUFFER_GPU_ADDRESS_NV, &buff->handle);
	 */
	/*
	 * 	if (glGetTextureHandleARB) {
		tex->handle = GL_TRACE(glGetTextureHandleARB(tex->id));
		GL_TRACE(glMakeTextureHandleResidentARB(tex->handle));
	}
	 */

#ifdef _DEBUG
	//if (desc->name)
	//	Vkd_SetObjectName(Re_device->dev, buff->buff, VK_OBJECT_TYPE_BUFFER, desc->name);
#endif

	return true;
}

struct NeBuffer *
Re_BkCreateBuffer(const struct NeBufferDesc *desc, uint16_t location)
{
	struct NeBuffer *buff = Sys_Alloc(1, sizeof(*buff), MH_Frame);
	if (!buff)
		return NULL;

	buff->transient = false;

	if (!GLBk_CreateBuffer(desc, buff)) {
		Sys_Free(buff);
		return NULL;
	}

	return buff;
}

void
Re_BkUpdateBuffer(struct NeBuffer *buff, uint64_t offset, void *data, uint64_t size)
{
	if (buff->staging) {
		memcpy(buff->staging, data, size);
		return;
	}

	if (buff->persistent) {
		while (buff->fences[Re_frameId]) {
			GLenum rc = GL_TRACE(glClientWaitSync(buff->fences[Re_frameId], GL_SYNC_FLUSH_COMMANDS_BIT, UINT64_MAX));
			if (rc != GL_ALREADY_SIGNALED && rc != GL_CONDITION_SATISFIED)
				continue;

			glDeleteSync(buff->fences[Re_frameId]);
			buff->fences[Re_frameId] = 0;
		}

		memcpy((uint8_t *)buff->persistent + offset, data, size);

		GL_TRACE(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));

		return;
	}

	if (GLAD_GL_ARB_direct_state_access) {
		GL_TRACE(glNamedBufferSubData(buff->id, offset, size, data));
	} else {
		GL_TRACE(glBindBuffer(buff->bindPoint, buff->id));
		GL_TRACE(glBufferSubData(buff->bindPoint, offset, size, data));
		GL_TRACE(glBindBuffer(buff->bindPoint, buff->id));
	}
}

void *
Re_BkMapBuffer(struct NeBuffer *buff)
{
	if (buff->staging)
		return buff->staging;

	if (buff->persistent)
		return buff->persistent;

	void *mem = NULL;
	if (GLAD_GL_ARB_direct_state_access) {
		mem = GL_TRACE(glMapNamedBuffer(buff->id, GL_READ_WRITE));
	} else {
		GL_TRACE(glBindBuffer(buff->bindPoint, buff->id));
		mem = GL_TRACE(glMapBuffer(buff->bindPoint, GL_READ_WRITE));
		GL_TRACE(glBindBuffer(buff->bindPoint, 0));
	}

	return mem;
}

void
Re_BkFlushBuffer(struct NeBuffer *buff, uint64_t offset, uint64_t size)
{
	if (buff->staging)
		return;

	if (GLAD_GL_ARB_direct_state_access) {
		GL_TRACE(glFlushMappedNamedBufferRange(buff->id, offset, size));
	} else {
		GL_TRACE(glBindBuffer(buff->bindPoint, buff->id));
		GL_TRACE(glFlushMappedBufferRange(buff->bindPoint, offset, size));
		GL_TRACE(glBindBuffer(buff->bindPoint, 0));
	}
}

void
Re_BkUnmapBuffer(struct NeBuffer *buff)
{
	if (buff->staging)
		return;

	if (GLAD_GL_ARB_direct_state_access) {
		GL_TRACE(glUnmapNamedBuffer(buff->id));
	} else {
		GL_TRACE(glBindBuffer(buff->bindPoint, buff->id));
		GL_TRACE(glUnmapBuffer(buff->bindPoint));
		GL_TRACE(glBindBuffer(buff->bindPoint, 0));
	}
}

uint64_t
Re_BkBufferAddress(const struct NeBuffer *buff, uint64_t offset)
{
	return 0; // TODO: GL_NV_shader_buffer_load
}

uint64_t
Re_OffsetAddress(uint64_t addr, uint64_t offset)
{
	return addr + offset;
}

void
Re_BkDestroyBuffer(struct NeBuffer *buff)
{
	if (buff->staging) {
		Sys_Free(buff->staging);
	} else {
		if (buff->transient) {
			GLBk_ReleaseTransientBuffer(buff->id, buff->hash);
			return;
		}

		GL_TRACE(glDeleteBuffers(1, &buff->id));
	}

	Sys_Free(buff);
}

/* NekoEngine
 *
 * GLBuffer.c
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
