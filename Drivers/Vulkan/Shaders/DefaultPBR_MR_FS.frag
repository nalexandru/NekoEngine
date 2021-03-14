#version 460 core

layout(location = 0) out vec4 o_fragColor;

layout(location = 0) in vec4 v_color;
layout(location = 1) in vec2 v_uv;

layout(set = 0, binding = 0) uniform texture2D u_textures[];
layout(set = 1, binding = 1) uniform sampler u_sampler;

void
main()
{
	o_fragColor = texture(sampler2D(u_textures[0], u_sampler), v_uv);
}
