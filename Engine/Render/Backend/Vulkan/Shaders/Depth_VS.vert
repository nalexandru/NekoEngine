#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_ARB_shader_draw_parameters : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

#include "DepthDrawInfo.glsl"
#include "Vertex.glsl"

invariant gl_Position;

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

layout(location = 0) out vec3 v_pos;
layout(location = 1) out vec2 v_uv;
layout(location = 2) out vec3 v_normal;

void
main()
{
	const vec4 v = vec4(a_pos, 1.0);
	const vec3 n = a_normal;// / vec3(127.0) - vec3(1.0);
//	const vec3 t = a_tangent;// / vec3(127.0) - vec3(1.0);
	v_uv = a_uv;
	const mat4 mvp = DrawInfo.instance.mvp;
	const mat3 normalMat = mat3(DrawInfo.instance.normal);

	const vec4 pos = DrawInfo.instance.model * v;


	v_normal = normalize(normalMat * n);
	//v_tangent = normalize(normalMat * t);
	//v_b = normalize(normalMat * cross(v_t, v_n));
	v_pos = pos.xyz / pos.w;

	gl_Position = mvp * v;
}

/* NekoEngine
 *
 * Depth_VS.vert
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
