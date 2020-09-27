#version 450

#extension GL_ARB_explicit_uniform_location : require

in vec2 v_uv;

layout(location = 0) out vec4 o_FragColor;

layout(location = 1) uniform sampler2D u_texture;

const float GAMMA = 2.2;
const float INV_GAMMA = 1.0 / GAMMA;

vec4
sRGBToLinear(vec4 color)
{
	return vec4(pow(color.xyz, vec3(GAMMA)), color.w);
}

// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec4
Tonemap(vec4 color, float exposure)
{
	const vec3 a = vec3(2.51f);
	const vec3 b = vec3(0.03f);
	const vec3 c = vec3(2.43f);
	const vec3 d = vec3(0.59f);
	const vec3 e = vec3(0.14f);

	color.xyz *= vec3(exposure);

	return vec4(pow(clamp((color.xyz * (a * color.xyz + b)) / (color.xyz * (c * color.xyz + d) + e), 0.0, 1.0), vec3(INV_GAMMA)), color.w);
}

void main()
{
	vec4 color = sRGBToLinear(texture(u_texture, v_uv));

	o_FragColor = Tonemap(color, 1.0);
}
