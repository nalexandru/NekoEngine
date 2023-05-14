#version 460 core

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_shader_8bit_storage : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require

layout(location = 0) in vec2 v_uv;
layout(location = 0) out vec4 o_fragColor;

layout(std430, buffer_reference) readonly buffer UIColorBuffer
{
	u8vec4 data[];
};

layout(push_constant) uniform ConstantArgs
{
	UIColorBuffer uiColor;
	uint width;
	uint height;
} args;

void
main()
{
	const uvec2 location = uvec2(v_uv.x * args.width, (1.f - v_uv.y) * args.height);
	const u8vec4 ui = args.uiColor.data[location.x + location.y * args.width];
	o_fragColor = vec4(ui.bgra) / 255.f;
}
