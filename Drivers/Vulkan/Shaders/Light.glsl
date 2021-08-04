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

#endif /* _RE_LIGHT_H_ */