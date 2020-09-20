struct Material
{
	float4 diffuseColor;
	float4 emissiveColor;
	float roughness;
	float metallic;
	uint textures[10];
};

struct Light
{
	float4 position;
};

Texture2D Res_Textures[] : register(t0, space0);
TextureCube Res_CubeTextures[] : register(t0, space1);

StructuredBuffer<Material> Res_Materials[] : register(t0, space2);
StructuredBuffer<Light> Res_Lights[] : register(t0, space3);