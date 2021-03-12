#version 460 core

layout(location = 0) out vec4 o_fragColor;

layout(location = 0) in vec4 v_color;

void
main()
{
	o_fragColor = v_color;
}
