#include <Render/Material.h>

#include "Random.hlsli"
#include "RTUtils.hlsli"

struct PathTracerPayload
{
	float4 color;
};

struct ShadowPayload
{
	float visibility;
};

struct Attributes
{
	float2 bary;
};

struct Vertex
{
	float x, y, z;
	float nx, ny, nz;
	float tx, ty, tz;
	float u, v;
};

struct Material
{
	float4 diffuse;
	float4 emissive;
	float roughness;
	float metallic;
	uint textures[RE_MAX_TEXTURES];
};

// Output
RWTexture2D<float4> PT_Output : register(u0);

// Geometry
RaytracingAccelerationStructure PT_TopLevelAS : register(t0, space0);
StructuredBuffer<Vertex> PT_Vertices[] : register(t0, space1);
StructuredBuffer<uint> PT_Indices[] : register(t0, space2);

// Scene Data
cbuffer RenderData : register(b0)
{
	float4x4 RD_inverseView;
	float4x4 RD_inverseProjection;
	float4x4 RD_viewProjection;
	float4x4 RD_inverseViewProjection;

/*	float3 RD_SunDirection;
	float RD_CosSunAngularRadius;
	float3 RD_SunIrradiance;
	float RD_SinSunAngularRadius;
	float3 RD_SunRenderColor;*/

	float RD_aspectRatio;
	uint RD_numSamples;
};

// Resources
StructuredBuffer<Material> PT_Materials : register(t1, space0);
Texture2D Res_Textures[] : register(t0, space3);
SamplerState Res_Sampler : register(s0);

[shader("raygeneration")] 
void RayGen() {
	PathTracerPayload payload;
	payload.color = float4(0.0, 0.0, 0.0, 1);
	
	uint2 pixelCoords = DispatchRaysIndex().xy;
	float2 dims = float2(DispatchRaysDimensions().xy);
	float2 d = (((pixelCoords.xy + 0.5f) / dims.xy) * 2.f - 1.f);

	RayDesc ray;
	ray.Origin = mul(RD_inverseView, float4(0, 0, 0, 1)).xyz;

	float4 target = mul(RD_inverseProjection, float4(d.x, -d.y, 1, 1));
	ray.Direction = mul(RD_inverseView, float4(target.xyz, 0)).xyz;

	ray.TMin = 0.01;
	ray.TMax = 10000.0;

	TraceRay(PT_TopLevelAS, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);
	
	PT_Output[pixelCoords] = float4(payload.color);
}

float2 BarycentricInterpolation(in float2 a0, in float2 a1, in float2 a2, in float3 b) {
	return b.x * a0 + b.y * a1 + b.z * a2;
}

[shader("closesthit")] 
void ClosestHit(inout PathTracerPayload payload, Attributes attrib) 
{
	int prim = PrimitiveIndex();
	int inst = InstanceID();

	Material mtl = PT_Materials[inst];
	uint3 indices = uint3(
		PT_Indices[inst][(prim * 3) + 0],
		PT_Indices[inst][(prim * 3) + 1],
		PT_Indices[inst][(prim * 3) + 2]
	);

	Vertex v0 = PT_Vertices[inst][indices[0]];
	Vertex v1 = PT_Vertices[inst][indices[1]];
	Vertex v2 = PT_Vertices[inst][indices[2]];

	float3 factors = CalculateBarycentricalInterpolationFactors(attrib.bary);

	float2 tc0 = float2(v0.u, v0.v);
	float2 tc1 = float2(v1.u, v1.v);
	float2 tc2 = float2(v2.u, v2.v);

	float2 texCoords = BarycentricInterpolation(tc0, tc1, tc2, factors);

	payload.color += float4(SampleTexture(Res_Textures[mtl.textures[MAP_DIFFUSE]], Res_Sampler, texCoords).xyz, 1.f);
}

[shader("miss")]
void Miss(inout PathTracerPayload payload : SV_RayPayload)
{
	payload.color = float4(0.412f, 0.796f, 1.0f, 1.f);
}

[shader("closesthit")]
void ShadowHit(inout ShadowPayload payload : SV_RayPayload, Attributes attrib)
{
	payload.visibility = 1.0f;
}

[shader("miss")]
void ShadowMiss(inout ShadowPayload payload : SV_RayPayload)
{
	payload.visibility = 0.0f;
}

