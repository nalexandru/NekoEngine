#ifndef _NE_SCENE_PRIMITIVE_H_
#define _NE_SCENE_PRIMITIVE_H_

#include <Render/Types.h>

enum NePrimitive
{
	PRIM_PLANE,
	PRIM_CUBE,
	PRIM_SPHERE,
	PRIM_CONE,
	PRIM_CYLINDER,

	PRIM_COUNT
};

bool Scn_PrimitiveInfo(enum NePrimitive prim, uint32_t *indexCount, uint32_t *indexOffset, uint32_t *vertexOffset);
void Scn_GeneratePrimitive(enum NePrimitive prim);

void Scn_BindPrimitieBuffers(void);
void Scn_DrawPrimitive(enum NePrimitive prim);

bool Scn_InitPrimitives(void);
void Scn_TermPrimitives(void);

#endif /* _NE_SCENE_PRIMITIVE_H_ */

/* NekoEngine
 *
 * Primitive.h
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
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
