#include "PBR.hlsli"
#include "Random.hlsli"
#include "RTUtils.hlsli"
#include "Tonemap.hlsli"
#include "Resources.hlsli"

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

// Output
RWTexture2D<float4> PT_Output : register(u0);

// Geometry
RaytracingAccelerationStructure PT_TopLevelAS : register(t0, space0);
StructuredBuffer<Vertex> PT_Vertices[] : register(t0, space1);
StructuredBuffer<uint> PT_Indices[] : register(t0, space2);

[shader("raygeneration")] 
void RayGen() {
	PathTracerPayload payload;

	const uint2 idx = DispatchRaysIndex().xy;
	const float2 pixelCoords = float2(idx);
	const float2 dims = float2(DispatchRaysDimensions().xy);
	const float2 d = (((pixelCoords.xy + 0.5f) / dims.xy) * 2.0f - 1.0f);

	RayDesc ray;
	ray.Origin = mul(RD_inverseView, float4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;

	float4 target = mul(RD_inverseProjection, float4(d.x, -d.y, 1.0f, 1.0f));
	ray.Direction = mul(RD_inverseView, float4(target.xyz, 0.0f)).xyz;

	ray.TMin = 0.01f;
	ray.TMax = 10000.0f;

	TraceRay(PT_TopLevelAS, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);
	
	PT_Output[idx] = Tonemap(payload.color, 1.0);
}

[shader("raygeneration")] 
void RayGenMS() {
	PathTracerPayload payload;
	float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);

	const uint2 idx = DispatchRaysIndex().xy;
	const float2 dims = float2(DispatchRaysDimensions().xy);

	uint pxRand = RD_seed;
	uint rayRand = RandSeed(RandSeed(idx.x, idx.y), RD_numSamples);

	for (uint i = 0; i < RD_numSamples; ++i) {
		const float2 pixelCoords = float2(idx) + float2(RandF(pxRand), RandF(pxRand));
		const float2 d = (((pixelCoords.xy + 0.5f) / dims.xy) * 2.0f - 1.0f);

		const float2 offset = RD_aperture / 2.0f * RandDisk(rayRand);

		RayDesc ray;
		ray.Origin = mul(RD_inverseView, float4(offset, 0.0f, 1.0f)).xyz;

		float4 target = mul(RD_inverseProjection, float4(d.x, -d.y, 1.0f, 1.0f));
		ray.Direction = mul(RD_inverseView, float4(normalize(target.xyz - float3(offset, 0.0f)), 0.0f)).xyz;

		ray.TMin = 0.01f;
		ray.TMax = 10000.0f;

		TraceRay(PT_TopLevelAS, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);
		color += payload.color;
	}

	color /= RD_numSamples;
	
	PT_Output[idx] = Tonemap(color, 1.0);
}

[shader("closesthit")] 
void ClosestHit(inout PathTracerPayload payload, Attributes attrib) 
{
	int prim = PrimitiveIndex();
	int inst = InstanceID();

	Material mtl = Res_Materials[inst];
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

	ShadingInfo si = BuildShadingInfo(mtl, texCoords);

	payload.color = EvaluateShading(si);
}

[shader("miss")]
void Miss(inout PathTracerPayload payload : SV_RayPayload)
{
	float3 p = normalize(WorldRayDirection());
	float u = (1.f + atan2(p.x, -p.z) * M_1_PI) * 0.5f;
	float v = acos(p.y) * M_1_PI;
	
	payload.color = sRGBToLinear(SampleTexture(Res_Textures[RD_environmentMap], Res_Sampler, float2(u, v)));
	
	//float2 d;
	//Res_Textures[RD_environmentMap].GetDimensions(d.x, d.y);



//	payload.color = Res_Textures[uint2(u * d.x, v * d.y)];
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
