#version 460 core

struct Vertex
{
	float x, y, z;
	float nx, ny, nz;
	float tx, ty, tz;
	float u, v;
};

layout(location = 0) out vec4 v_color;
layout(location = 1) out vec2 v_uv;

layout(std430, set = 1, binding = 0) readonly buffer VertexBuffer
{
	Vertex Re_vertices[];
};

void
main()
{
	vec3 v = vec3(Re_vertices[gl_VertexIndex].x, Re_vertices[gl_VertexIndex].y, Re_vertices[gl_VertexIndex].z);
	v_uv = vec2(Re_vertices[gl_VertexIndex].u, Re_vertices[gl_VertexIndex].v);

	v_color = vec4(1.0);

	gl_Position = vec4(v, 1.0);
}
