/* NekoEngine
 *
 * buffer.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Buffer
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>

#include "glad.h"
#include <graphics/buffer.h>

typedef struct ne_buff_sync
{
	uint64_t offset;
	GLsync sync;
} ne_buff_sync;

struct ne_buffer
{
	GLuint id;
	GLenum target;
	uint8_t *data;
	ne_buff_sync sync[3];
	int32_t cur_buff;
	uint64_t size, total_size;
	bool dynamic;
	bool persistent;
};

struct ne_buffer *
buffer_create(bool dynamic,
	bool persistent)
{
	ne_buffer *buff = calloc(1, sizeof(*buff));
	if (!buff)
		return NULL;

	buff->dynamic = dynamic;
	buff->persistent = persistent;

	glCreateBuffers(1, &buff->id);

	return buff;
}

void
buffer_bind(struct ne_buffer *buff,
	int32_t loc)
{
	if (!buff)
		return;

	buff->target = loc;

	if (!buff->persistent || loc != GL_UNIFORM_BUFFER)
		glBindBuffer(loc, buff->id);
	else
		glBindBufferRange(loc, 0, buff->id,
			buff->persistent ? buff->sync[buff->cur_buff].offset : 0,
			buff->size);
}

uint8_t *
buffer_data(struct ne_buffer *buff)
{
	if (!buff || !buff->persistent)
		return NULL;

	return buff->data + buff->size * buff->cur_buff;
}

int32_t
buffer_current(ne_buffer *buff)
{
	return buff ? buff->cur_buff : 0;
}

uint64_t
buffer_offset(ne_buffer *buff)
{
	return buff ? buff->sync[buff->cur_buff].offset : 0;
}

void
buffer_set_storage(ne_buffer *buff,
	uint64_t size,
	void *data)
{
	int flags = 0;

	if (!buff)
		return;

	buff->size = size;

	if (buff->persistent)
		flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	else if (buff->dynamic)
		flags = GL_DYNAMIC_STORAGE_BIT;

	if (!buff->persistent) {
		buff->total_size = size;
		glNamedBufferStorage(buff->id, buff->size, data, flags);
		return;
	}

	for (uint8_t i = 0; i < 3; ++i) {
		buff->sync[i].offset = buff->size * i;
		buff->sync[i].sync = 0;
	}

	buff->total_size = buff->size * 3;
	glNamedBufferStorage(buff->id, buff->total_size, data, flags);
	buff->data = glMapNamedBufferRange(buff->id, 0, buff->total_size, flags);

	buff->cur_buff = 0;
}

void
buffer_begin_update(ne_buffer *buff)
{
	GLenum wait;
	GLsync sync;

	if (!buff || !buff->persistent)
		return;

	if ((sync = buff->sync[buff->cur_buff].sync) == 0)
		return;

	while (true) {
		wait = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
		if (wait == GL_ALREADY_SIGNALED || wait == GL_CONDITION_SATISFIED)
			return;
	}
}

void
buffer_update(ne_buffer *buff,
	uint64_t offset,
	uint64_t size,
	void *data)
{
	if (buff->persistent)
		memcpy(buffer_data(buff) + offset, data, size);
	else
		glNamedBufferSubData(buff->id, offset, size, data);
}

void
buffer_end_update(ne_buffer *buff)
{
	GLsync sync;

	if (!buff || !buff->persistent)
		return;

	if ((sync = buff->sync[buff->cur_buff].sync) != 0)
		glDeleteSync(sync);

	buff->sync[buff->cur_buff].sync =
		glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

void
buffer_next(ne_buffer *buff)
{
	if (!buff || !buff->persistent)
		return;

	buff->cur_buff = (buff->cur_buff + 1) % 3;
}

void
buffer_bind_uniform(ne_buffer *buff,
	int idx,
	uint64_t offset,
	uint64_t size)
{
	glBindBufferRange(buff->target, idx, buff->id,
		buff->persistent ?
			buff->sync[buff->cur_buff].offset + offset : offset,
		size);
}

void
buffer_destroy(ne_buffer *buff)
{
	if (!buff)
		return;

	glUnmapNamedBuffer(buff->id);
	glDeleteBuffers(1, &buff->id);

	free(buff);
}
