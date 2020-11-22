#ifndef _RT_UTILS_H_
#define _RT_UTILS_H_

#define M_PI	 	3.14159265358979323846
#define M_1_PI		0.318309886183790671538

float2 BarycentricInterpolation(in float2 a0, in float2 a1, in float2 a2, in float3 b) {
	return b.x * a0 + b.y * a1 + b.z * a2;
}

float3 CalculateBarycentricalInterpolationFactors(in float2 bary)
{
	return float3(1.0 - bary.x - bary.y, bary.x, bary.y);
}

float4 SampleTexture(Texture2D tex, SamplerState s, float2 texCoords)
{
	return tex.SampleLevel(s, texCoords, 0);
}

#endif /* _RT_UTILS_H_ */