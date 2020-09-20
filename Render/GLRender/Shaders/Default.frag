#version 450

#extension GL_ARB_explicit_uniform_location : require

in vec2 v_uv;

layout(location = 0) out vec4 o_FragColor;

layout(location = 1) uniform sampler2D u_texture;

void main()
{
	o_FragColor = vec4(texture(u_texture, v_uv).rgb, 1.0);
}
