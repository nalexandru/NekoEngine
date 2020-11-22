#version 450

#extension GL_ARB_explicit_uniform_location : require

in vec2 v_uv;
in vec4 v_color;

layout(location = 0) out vec4 o_FragColor;

layout(location = 1) uniform sampler2D u_texture;

void main()
{
	o_FragColor = vec4(v_color.xyz, v_color.w * texture(u_texture, vec2(v_uv.x, -v_uv.y)).r);
}
