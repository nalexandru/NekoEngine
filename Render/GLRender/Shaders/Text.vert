#version 450

layout(location = 0) in vec4 a_posUv;

layout(location = 0) uniform mat4 u_mvp;

out vec2 v_uv;

void main()
{
	v_uv = vec2(a_posUv.z, -a_posUv.w);
	gl_Position = u_mvp * vec4(a_posUv.xy, 0.0, 1.0);
}
