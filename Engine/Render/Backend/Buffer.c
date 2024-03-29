#include <stdatomic.h>

#include <Render/Render.h>
#include <Render/Backend.h>
#include <Runtime/Runtime.h>

struct NeBufferInfo
{
	struct NeBuffer *buff;
	struct NeBufferDesc desc;
};

static atomic_uint_fast32_t f_nextId;
static struct NeBufferInfo f_buffers[UINT16_MAX];
static struct NeQueue f_freeList;

bool
Re_CreateBuffer(const struct NeBufferCreateInfo *bci, NeBufferHandle *handle)
{
	if (!Re_ReserveBufferId(handle))
		return false;

	struct NeBufferInfo *info = &f_buffers[*handle];

	info->buff = Re_BkCreateBuffer(&bci->desc, *handle);
	if (!info->buff)
		return false;

	memcpy(&info->desc, &bci->desc, sizeof(info->desc));
	
	if (bci->data) {
		Re_UpdateBuffer(*handle, 0, bci->data, bci->dataSize);
		if (!bci->keepData)
			Sys_Free(bci->data);
	}
	
	return true;
}

void
Re_UpdateBuffer(NeBufferHandle handle, uint64_t offset, void *data, uint64_t size)
{
	Re_BkUpdateBuffer(f_buffers[handle].buff, offset, data, size);
}

void
Re_CmdUpdateBuffer(NeBufferHandle handle, uint64_t offset, void *data, uint64_t size)
{
	Re_BkCmdUpdateBuffer(f_buffers[handle].buff, offset, data, size);
}

void
Re_CmdCopyBuffer(NeBufferHandle src, uint64_t srcOffset, NeBufferHandle dst, uint64_t dstOffset, uint64_t size)
{
	Re_BkCmdCopyBuffer(f_buffers[src].buff, srcOffset, f_buffers[dst].buff, dstOffset, size);
}

void *
Re_MapBuffer(NeBufferHandle handle)
{
	return Re_BkMapBuffer(f_buffers[handle].buff);
}

void
Re_FlushBuffer(NeBufferHandle handle, uint64_t offset, uint64_t size)
{
	Re_BkFlushBuffer(f_buffers[handle].buff, offset, size);
}

void
Re_UnmapBuffer(NeBufferHandle handle)
{
	Re_BkUnmapBuffer(f_buffers[handle].buff);
}

uint64_t
Re_BufferAddress(NeBufferHandle handle, uint64_t offset)
{
	return Re_BkBufferAddress(f_buffers[handle].buff, offset);
}

const struct NeBufferDesc *
Re_BufferDesc(NeBufferHandle handle)
{
	return &f_buffers[handle].desc;
}

void
Re_DestroyBuffer(NeBufferHandle handle)
{
	Re_BkDestroyBuffer(f_buffers[handle].buff);
	Re_ReleaseBufferId(handle);
}

bool
Re_ReserveBufferId(NeBufferHandle *handle)
{
	if (f_freeList.count) {
		*handle = *(NeBufferHandle*)Rt_QueuePop(&f_freeList);
	} else {
		const uint32_t nextId = atomic_fetch_add(&f_nextId, 1);
		if (nextId < UINT16_MAX) {
			*handle = nextId;
		} else {
			atomic_fetch_add(&f_nextId, -1);
			*handle = (uint16_t)-1;
			return false;
		}
	}

	return true;
}

void
Re_ReleaseBufferId(NeBufferHandle handle)
{
	Rt_QueuePush(&f_freeList, &handle);
}

void
Re_CmdBindVertexBuffer(NeBufferHandle handle, uint64_t offset)
{
	Re_BkCmdBindVertexBuffer(f_buffers[handle].buff, offset);
}

void
Re_CmdBindVertexBuffers(uint32_t count, NeBufferHandle *handles, uint64_t *offsets, uint32_t start)
{
	struct NeBuffer **buffers = Sys_Alloc(sizeof(*buffers), count, MH_Frame);

	for (uint32_t i = 0; i < count; ++i)
		buffers[i] = f_buffers[handles[i]].buff;

	Re_BkCmdBindVertexBuffers(count, buffers, offsets, start);
}

void
Re_CmdBindIndexBuffer(NeBufferHandle handle, uint64_t offset, enum NeIndexType type)
{
	Re_BkCmdBindIndexBuffer(f_buffers[handle].buff, offset, type);
}

struct NeBuffer *
ReP_CreateTransientBuffer(const struct NeBufferDesc *desc, NeBufferHandle location, uint64_t offset, uint64_t *size)
{
	f_buffers[location].desc = *desc;
	f_buffers[location].buff = Re_BkCreateTransientBuffer(desc, location, offset, size);

	return f_buffers[location].buff;
}

bool
Re_InitBufferSystem(void)
{
	return Rt_InitQueue(&f_freeList, UINT16_MAX, sizeof(NeBufferHandle), MH_Render);
}

void
Re_TermBufferSystem(void)
{
	Rt_TermQueue(&f_freeList);
}

void
Re_CmdCopyBufferToTexture(NeBufferHandle src, struct NeTexture *dst, const struct NeBufferImageCopy *bic)
{
	Re_BkCmdCopyBufferToTexture(f_buffers[src].buff, dst, bic);
}

void
Re_CmdCopyTextureToBuffer(struct NeTexture *src, NeBufferHandle dst, const struct NeBufferImageCopy *bic)
{
	Re_BkCmdCopyTextureToBuffer(src, f_buffers[dst].buff, bic);
}

/* NekoEngine
 *
 * Buffer.c
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
