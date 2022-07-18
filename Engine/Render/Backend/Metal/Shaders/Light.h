#ifndef Light_h
#define Light_h

#include <metal_stdlib>
using namespace metal;

inline float
attenuate(float range, float dist)
{
	return clamp(1.0 - dist * dist / (range * range), 0.0, 1.0);
}

inline float
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

#endif /* Light_h */
