#ifndef LIGHT_H
#define LIGHT_H

#define LT_DIRECTIONAL	0
#define LT_POINT		1
#define LT_SPOT			2

struct Light
{
	float3 position;
	uint type;

	float3 direction;
	uint __padding;

	float3 color;
	float intensity;

	float innerRadius, outerRadius;
	float innerCutoff, outerCutoff;
};

StructuredBuffer<Light> Re_lights : register(t1, space1);
RWStructuredBuffer<uint> Re_visibleLightIndices : register(t1, space2);
StructuredBuffer<uint> Re_visibleLightIndicesRead : register(t1, space3);

float
attenuate(float range, float dist)
{
	return clamp(1.0 - dist * dist / (range * range), 0.0, 1.0);
}

float
attenuateSpot(float3 l, float3 dir, float outer, float inner)
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

#endif /* LIGHT_H */

/* NekoEngine
 *
 * Light.hlsli
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
