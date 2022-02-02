#include <stdlib.h>

#include "NullGraphicsDriver.h"

struct NeBuffer *
NG_CreateBuffer(struct NeRenderDevice *dev, const struct NeBufferDesc *desc, uint16_t location)
{
	struct NeBuffer *buff = Sys_Alloc(1, sizeof(*buff), MH_RenderDriver);
	if (buff)
		buff->size = desc->size;
	return buff;
}

void NG_UpdateBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff, uint64_t offset, void *data, uint64_t size) { }

void *
NG_MapBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff)
{
	if (buff->ptr) {
		Sys_LogEntry(NGDRV_MOD, LOG_WARNING, "Attempt to map already mapped memory");
		return buff->ptr;
	}

	buff->ptr = Sys_Alloc(buff->size, 1, MH_RenderDriver);

	return buff->ptr;
}

void NG_FlushBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff, uint64_t offset, uint64_t size) { }

void
NG_UnmapBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff)
{
	Sys_Free(buff->ptr);
	buff->ptr = NULL;
}

uint64_t
NG_BufferAddress(struct NeRenderDevice *dev, const struct NeBuffer *buff, uint64_t offset)
{
	return (uintptr_t)buff;
}

uint64_t
NG_OffsetAddress(uint64_t addr, uint64_t offset)
{
	return addr + offset;
}

void
NG_DestroyBuffer(struct NeRenderDevice *dev, struct NeBuffer *buff)
{
	Sys_Free(buff->ptr);
	Sys_Free(buff);
}
