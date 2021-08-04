#ifndef Tonemap_h
#define Tonemap_h

#include <metal_stdlib>
using namespace metal;

constant float3x3 ACESInputMat =
{
	{ 0.59719, 0.35458, 0.04823 },
	{ 0.07600, 0.90834, 0.01566 },
	{ 0.02840, 0.13383, 0.83777 }
};

constant float3x3 ACESOutputMat =
{
	{  1.60475, -0.53108, -0.07367 },
	{ -0.10208,  1.10813, -0.00605 },
	{ -0.00327, -0.07276,  1.07602 }
};

constant float3 a = { 0.0245786, 0.0245786, 0.0245786 };
constant float3 b = { 0.000090537, 0.000090537, 0.000090537 };
constant float3 c = { 0.983729, 0.983729, 0.983729 };
constant float3 d = { 0.4329510, 0.4329510, 0.4329510 };
constant float3 e = { 0.238081, 0.238081, 0.238081 };

inline float4
sRGBtoLinear(float4 color, float gamma)
{
	return float4(pow(color.rgb, float3(gamma)), color.a);
}

inline float3
tonemap(float3 color, float exposure, float invGamma)
{
	color = (color * exposure) * ACESInputMat;
	color = (color * (color + a) - b) / (color * (c * color + d) + e);

	return pow(clamp(color * ACESOutputMat, 0.0, 1.0), invGamma);
}

#endif /* Tonemap_h */
