#ifndef _RE_VERTEX_H_
#define _RE_VERTEX_H_

#include "Types.glsl"

vec3 Re_Position()
{
	return vec3(
		DrawInfo.vertices.data[gl_VertexIndex].x,
		DrawInfo.vertices.data[gl_VertexIndex].y,
		DrawInfo.vertices.data[gl_VertexIndex].z
	);
}

vec3 Re_Normal()
{
	return vec3(
		DrawInfo.vertices.data[gl_VertexIndex].nx,
		DrawInfo.vertices.data[gl_VertexIndex].ny,
		DrawInfo.vertices.data[gl_VertexIndex].nz
	);
}

vec3 Re_Tangent()
{
	return vec3(
		DrawInfo.vertices.data[gl_VertexIndex].tx,
		DrawInfo.vertices.data[gl_VertexIndex].ty,
		DrawInfo.vertices.data[gl_VertexIndex].tz
	);
}

vec2 Re_TexCoord()
{
	return vec2(
		DrawInfo.vertices.data[gl_VertexIndex].u,
		DrawInfo.vertices.data[gl_VertexIndex].v
	);
}

vec4 Re_Color()
{
	return vec4(
		DrawInfo.vertices.data[gl_VertexIndex].r,
		DrawInfo.vertices.data[gl_VertexIndex].g,
		DrawInfo.vertices.data[gl_VertexIndex].b,
		DrawInfo.vertices.data[gl_VertexIndex].a
	);
}

#ifndef VTX_NO_INSTANCE

vec3 Re_I_Position()
{
	return vec3(
		DrawInfo.instance.vertices.data[gl_VertexIndex].x,
		DrawInfo.instance.vertices.data[gl_VertexIndex].y,
		DrawInfo.instance.vertices.data[gl_VertexIndex].z
	);
}

vec3 Re_I_Normal()
{
	return vec3(
		DrawInfo.instance.vertices.data[gl_VertexIndex].nx,
		DrawInfo.instance.vertices.data[gl_VertexIndex].ny,
		DrawInfo.instance.vertices.data[gl_VertexIndex].nz
	);
}

vec3 Re_I_Tangent()
{
	return vec3(
		DrawInfo.instance.vertices.data[gl_VertexIndex].tx,
		DrawInfo.instance.vertices.data[gl_VertexIndex].ty,
		DrawInfo.instance.vertices.data[gl_VertexIndex].tz
	);
}

vec2 Re_I_TexCoord()
{
	return vec2(
		DrawInfo.instance.vertices.data[gl_VertexIndex].u,
		DrawInfo.instance.vertices.data[gl_VertexIndex].v
	);
}

vec4 Re_I_Color()
{
	return vec4(
		DrawInfo.instance.vertices.data[gl_VertexIndex].r,
		DrawInfo.instance.vertices.data[gl_VertexIndex].g,
		DrawInfo.instance.vertices.data[gl_VertexIndex].b,
		DrawInfo.instance.vertices.data[gl_VertexIndex].a
	);
}

#endif

#endif /* _RE_VERTEX_H_ */

/* NekoEngine
 *
 * Vertex.glsl
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
