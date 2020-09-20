sampler2D diffuseMap : register(s0);

struct PixelData
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

float4
main(PixelData pd) : SV_TARGET
{
	return tex2D(diffuseMap, pd.texCoord);
}