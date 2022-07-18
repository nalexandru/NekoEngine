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
