#version 110

invariant gl_Position;

attribute vec3 a_pos;
attribute vec2 a_uv;
attribute vec4 a_color;

varying vec3 v_pos;
varying vec2 v_uv;
varying vec4 v_color;

/*
	VertexBuffer vertices;
	SceneBuffer scene;
	VisibleLightIndicesBufferRO visibleIndices;
	InstanceBuffer instance;
	MaterialBuffer material;
*/

uniform mat4 u_mvp;
uniform mat4 u_model;

void main()
{
	vec4 v = vec4(a_pos, 1.0);
	v_uv = a_uv;

	const mat4 mvp = u_mvp;
	const vec4 pos = u_model * v;
	v_pos = pos.xyz / pos.w;
	v_color = a_color;

	gl_Position = mvp * v;
}
