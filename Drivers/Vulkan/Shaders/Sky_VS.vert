#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_ARB_shader_draw_parameters : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

layout(location = 0) out vec3 v_uv;

struct Vertex
{
	float x, y, z;
};

layout(std430, buffer_reference) readonly buffer VertexBuffer
{
	Vertex data[];
};

layout(push_constant) uniform SkyDrawInfo
{
	uint texture;
	float exposure;
	float gamma;
	float invGamma;
	mat4 mvp;
	VertexBuffer vertices;
} DrawInfo;

void
main()
{
	vec4 pos = vec4(
		DrawInfo.vertices.data[gl_VertexIndex].x,
		DrawInfo.vertices.data[gl_VertexIndex].y,
		DrawInfo.vertices.data[gl_VertexIndex].z,
		1.0
	);
	v_uv = pos.xyz;
	gl_Position = DrawInfo.mvp * pos;
}
