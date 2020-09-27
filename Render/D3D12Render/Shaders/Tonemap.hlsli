#ifndef _TONEMAP_H_
#define _TONEMAP_H_

static const float3 GAMMA = float3(2.2, 2.2, 2.2);
static const float3 INV_GAMMA = float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2);

float4
sRGBToLinear(float4 color)
{
	return float4(pow(color.xyz, GAMMA), color.w);
}

// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
float4
Tonemap(float4 color, float exposure)
{
	const float3 a = float3(2.51f, 2.51f, 2.51f);
	const float3 b = float3(0.03f, 0.03f, 0.03f);
	const float3 c = float3(2.43f, 2.43f, 2.43f);
	const float3 d = float3(0.59f, 0.59f, 0.59f);
	const float3 e = float3(0.14f, 0.14f, 0.14f);

	color.xyz *= float3(exposure, exposure, exposure);

	return float4(pow(saturate((color.xyz * (a * color.xyz + b)) / (color.xyz * (c * color.xyz + d) + e)), INV_GAMMA), color.w);
}

#endif /* _TONEMAP_H_ */