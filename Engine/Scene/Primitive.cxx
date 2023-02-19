#include <Scene/Primitive.h>

struct PrimitiveInfo
{
	uint32_t indexCount;
	uint32_t indexOffset;
	uint32_t vertexOffset;
};

//static NeBufferHandle _primitiveBuffer;

static struct PrimitiveInfo _info[PRIM_COUNT];

bool
Scn_InitPrimitives(void)
{
	_info[PRIM_PLANE].indexCount = 4;
	_info[PRIM_PLANE].indexOffset = 0;
	_info[PRIM_PLANE].vertexOffset = 0;

	_info[PRIM_CUBE].indexCount = 0;
	_info[PRIM_CUBE].indexOffset = 0;
	_info[PRIM_CUBE].vertexOffset = 0;

	_info[PRIM_SPHERE].indexCount = 0;
	_info[PRIM_SPHERE].indexOffset = 0;
	_info[PRIM_SPHERE].vertexOffset = 0;

	_info[PRIM_CONE].indexCount = 0;
	_info[PRIM_CONE].indexOffset = 0;
	_info[PRIM_CONE].vertexOffset = 0;

	_info[PRIM_CYLINDER].indexCount = 0;
	_info[PRIM_CYLINDER].indexOffset = 0;
	_info[PRIM_CYLINDER].vertexOffset = 0;

	return false;
}

bool
Scn_PrimitiveInfo(enum NePrimitive prim, uint32_t *indexCount, uint32_t *indexOffset, uint32_t *vertexOffset)
{
	if (prim >= PRIM_COUNT)
		return false;

	const struct PrimitiveInfo *i = &_info[PRIM_COUNT];

	if (indexCount)
		*indexCount = i->indexCount;

	if (indexOffset)
		*indexOffset = i->indexOffset;

	if (vertexOffset)
		*vertexOffset = i->vertexOffset;

	return true;
}

void
Scn_TermPrimitives(void)
{
	//
}

/* NekoEngine
 *
 * Primitive.c
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
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
