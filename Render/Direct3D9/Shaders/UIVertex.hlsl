struct Vertex
{
	float2 position : POSITION;
	float2 texCoord : TEXCOORD;
	float4 color : COLOR;
};

struct PixelData
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float4 color : COLOR;
};

float4x4 SD_MVP : register(c0);

PixelData
main(Vertex v)
{
	PixelData ret;

	ret.position = mul(SD_MVP, float4(v.position, 0.0, 1.0));
	ret.texCoord = v.texCoord;
	ret.color = v.color;

	return ret;
}
