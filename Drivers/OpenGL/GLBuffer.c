#include <stdlib.h>

#include "OpenGLDriver.h"

struct NeBuffer *
GL_CreateBuffer(struct NeRenderDevice *dev, const struct NeBufferDesc *desc, uint16_t location)
{
	struct NeBuffer *buff = Sys_Alloc(1, sizeof(*buff), MH_RenderDriver);
	if (!buff)
		return NULL;

	glCreateBuffers(1, &buff->id);

	GLbitfield flags = 0;
	// TODO: flags

	glNamedBufferStorage(buff->id, desc->size, NULL, flags);

	glGetBufferParameterui64vNV(buff->id, GL_BUFFER_GPU_ADDRESS_NV, &buff->addr);
	glMakeNamedBufferResidentNV(buff->id, GL_READ_ONLY); // TODO: set ro/rw based on usage flags

	return buff;
}

void
GL_UpdateBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff, uint64_t offset, void *data, uint64_t size)
{
	if (buff->cpuVisible) {
		glNamedBufferSubData(buff->id, offset, size, data);
	} else {
		GLuint staging;

		glCreateBuffers(1, &staging);
		glNamedBufferStorage(staging, size, data, 0);

		glCopyNamedBufferSubData(staging, buff->id, 0, offset, size);

		// wait for fence ?

		glDeleteBuffers(1, &staging);
	}
}

void *
GL_MapBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff)
{
	return glMapNamedBuffer(buff->id, GL_READ_WRITE);
}

void
GL_FlushBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff, uint64_t offset, uint64_t size)
{
	glFlushMappedNamedBufferRange(buff->id, offset, size);
}

void
GL_UnmapBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff)
{
	glUnmapNamedBuffer(buff->id);
}

uint64_t
GL_BufferAddress(struct NeRenderDevice *dev, const struct NeBuffer *buff, uint64_t offset)
{
	return buff->addr + offset;
}

uint64_t
GL_OffsetAddress(uint64_t addr, uint64_t offset)
{
	return addr + offset;
}

void
GL_DestroyBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff)
{
	glMakeNamedBufferNonResidentNV(buff->id);
	glDeleteBuffers(1, &buff->id);
	Sys_Free(buff);
}
