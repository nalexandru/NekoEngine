#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_16bit_storage : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16 : require

layout(location = 0) out vec4 o_fragColor;

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
	mat4 mvp;
	VertexBuffer vertices;
} DrawInfo;

void
main()
{
	o_fragColor = vec4(1.0);
}
