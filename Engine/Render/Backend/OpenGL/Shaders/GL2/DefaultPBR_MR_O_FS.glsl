#version 110

layout(location = 0) out vec4 o_fragColor;

varying vec3 v_pos;
varying vec2 v_uv;
varying vec4 v_color;

//layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput in_wsNormals;

uniform sampler2D ia_wsNormals;

uniform float u_exposure;
uniform float u_invGamma;

void
main()
{
//	const vec4 color = PBR_MR(v_color, v_pos, subpassLoad(ia_wsNormals).xyz, v_uv);
	o_fragColor = v_color;//vec4(tonemap(color.rgb, u_exposure, u_invGamma), color.a);
}
