struct VSInput
{
	float4 posUv : POSITION;
	float4 color : COLOR0;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float2 texCoord : TEXCOORD0;
};

cbuffer UIRenderData : register(b0)
{
	float4x4 RD_Projection;
};

PSInput
main(VSInput input)
{
	PSInput output;

	output.position = mul(RD_Projection, float4(input.posUv.xy, 0.0, 1.0));
	output.color = input.color;
	output.texCoord = input.posUv.zw;

	return output;
}
