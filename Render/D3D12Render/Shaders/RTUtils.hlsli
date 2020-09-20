#ifndef _RT_UTILS_H_
#define _RT_UTILS_H_

float3 CalculateBarycentricalInterpolationFactors(in float2 bary)
{
	return float3(1.0 - bary.x - bary.y, bary.x, bary.y);
}

float4 SampleTexture(Texture2D tex, SamplerState s, float2 texCoords)
{
	return tex.SampleLevel(s, texCoords, 0);
}

#endif /* _RT_UTILS_H_ */