#ifndef _RE_SCENE_H_
#define _RE_SCENE_H_

#include "Types.glsl"
#include "Light.glsl"
#include "Material.glsl"

layout(std430, buffer_reference, buffer_reference_align = 16) readonly buffer InstanceBuffer
{
	mat4 mvp;
	mat4 model;
	mat4 normal;
	VertexBuffer vertices;
	MaterialBuffer material;
};

layout(std430, buffer_reference, buffer_reference_align = 16) readonly buffer SceneBuffer
{
	mat4 viewProjection;

	mat4 projection;
	mat4 inverseProjection;

	vec4 cameraPosition;

	vec4 sunPosition;
	uint enviornmentMap;
	uint irradianceMap;
	uint aoMap;
	uint reserved;

	uint lightCount;
	uint xTileCount;

	InstanceBuffer instances;

	float exposure;
	float gamma;
	float invGamma;
	uint sampleCount;

	Light lights[];
};

#endif /* _RE_SCENE_H_ */

/* NekoEngine
 *
 * Scene.glsl
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
