struct PSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	float2 texCoord : TEXCOORD0;
};

uint UI_TextureID : register(b0);
Texture2D UI_Texture[] : register(t0);
SamplerState UI_Sampler : register(s0);

float4
main(PSInput input) : SV_TARGET
{
	return float4(input.color.xyz, input.color.w * UI_Texture[UI_TextureID].Sample(UI_Sampler, input.texCoord).r);
}