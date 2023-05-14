#ifndef _RE_LIGHT_H_
#define _RE_LIGHT_H_

#define LT_DIRECTIONAL	0
#define LT_POINT		1
#define LT_SPOT			2

struct Light
{
	vec3 position;
	uint type;

	vec3 direction;
	uint __padding;

	vec3 color;
	float intensity;

	float innerRadius, outerRadius;
	float innerCutoff, outerCutoff;
};

layout(std430, buffer_reference) readonly buffer LightBuffer
{
	Light data[];
};

layout(std430, buffer_reference) writeonly buffer VisibleLightIndicesBuffer
{
	uint data[];
};

layout(std430, buffer_reference) readonly buffer VisibleLightIndicesBufferRO
{
	uint data[];
};

float
attenuate(float range, float dist)
{
	return clamp(1.0 - dist * dist / (range * range), 0.0, 1.0);
}

float
attenuateSpot(vec3 l, vec3 dir, float outer, float inner)
{
	const float theta = dot(normalize(-dir), l);
	if (theta > outer) {
		if (theta < inner)
			return smoothstep(outer, inner, theta);
		else
			return 1.0;
	} else {
		return 0.0;
	}
}

#endif /* _RE_LIGHT_H_ */

/* NekoEngine
 *
 * Light.glsl
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
