#version 450

layout(location = 0) in vec4 a_posUv;
layout(location = 1) in vec4 a_color;

layout(location = 0) uniform mat4 u_mvp;

out vec2 v_uv;
out vec4 v_color;

void main()
{
	v_uv = vec2(a_posUv.z, -a_posUv.w);
	v_color = a_color;

	gl_Position = u_mvp * vec4(a_posUv.xy, 0.0, 1.0);
}
