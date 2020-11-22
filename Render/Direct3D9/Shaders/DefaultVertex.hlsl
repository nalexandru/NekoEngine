struct Vertex
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 texCoord : TEXCOORD;
};

struct PixelData
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

float4x4 SD_MVP : register(c0);

PixelData
main(Vertex v)
{
	PixelData ret;

	ret.position = mul(SD_MVP, float4(v.position, 1.0));
	ret.texCoord = v.texCoord;

	return ret;
}