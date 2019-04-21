/* NekoEngine
 *
 * primitive.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Primitives
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

#ifndef _NE_GRAPHICS_PRIMITIVE_H_
#define _NE_GRAPHICS_PRIMITIVE_H_

#include <engine/status.h>
#include <graphics/buffer.h>

enum ne_primitive
{
	PRIM_TRIANGLE = 0,
	PRIM_QUAD,
	PRIM_CUBE,
	PRIM_PYRAMID,
	PRIM_SPHERE,
	PRIM_CONE,
	PRIM_CYLINDER,

	PRIM_COUNT
};


struct ne_mesh *gfx_primitive(enum ne_primitive p);

#ifdef _NE_ENGINE_INTERNAL_

ne_status gfx_init_primitive(void);
void gfx_release_primitive(void);

#endif /* _NE_ENGINE_INTERNAL_ */

#endif /* _NE_GRAPHICS_PRIMITIVE_H_ */

