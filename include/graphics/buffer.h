/* NekoEngine
 *
 * buffer.h
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

#ifndef _NE_GRAPHICS_BUFFER_H_
#define _NE_GRAPHICS_BUFFER_H_

#include <stdint.h>
#include <stdbool.h>

#include <engine/status.h>

typedef enum ne_buffer_usage
{
	NE_VERTEX_BUFFER		= 0x00000001,
	NE_INDEX_BUFFER			= 0x00000002,
	NE_UNIFORM_BUFFER		= 0x00000004,
	NE_STORAGE_BUFFER		= 0x00000008,
	NE_UNIFORM_TEXEL_BUFFER		= 0x00000010,
	NE_STORAGE_TEXEL_BUFFER		= 0x00000020,
	NE_INDIRECT_BUFFER		= 0x00000040,
	NE_TRANSFORM_FEEDBACK		= 0x00000080,
	NE_TRANSFORM_FEEDBACK_COUNTER	= 0x00000100,
	NE_CONDITIONAL_RENDERING	= 0x00000200,
	NE_RAY_TRACING			= 0x00000400,
	NE_SHADER_DEVICE_ADDRESS	= 0x00000800,
	NE_TRANSFER_SRC			= 0x00001000,
	NE_TRANSFER_DST			= 0x00002000
} ne_buffer_usage;

typedef enum ne_buffer_access
{
	NE_GPU_LOCAL			= 0x00000001,
	NE_CPU_VISIBLE			= 0x00000002,
	NE_CPU_COHERENT			= 0x00000004,
	NE_CPU_CACHED			= 0x00000008
} ne_buffer_access;

struct ne_buffer_create_info
{
	struct ne_buffer *master;
	uint64_t size;
	uint64_t offset;
	ne_buffer_usage usage;
	ne_buffer_access access;
};

struct ne_buffer;

#endif /* _NE_GRAPHICS_BUFFER_H_ */
