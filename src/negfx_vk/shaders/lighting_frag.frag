/* NekoEngine
 *
 * lighting_frag.frag
 * Author: Alexandru Naiman
 *
 * Lighting Shader
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#version 460 core
#extension GL_GOOGLE_include_directive : require

// Output
layout(location = 0) out vec4 o_FragColor;
//layout(location = 1) out vec4 o_Brightness;

#define SHADOW_MATRICES_BINDING		4

#define SCENE_DATA_SET			0
#define LIGHT_SET			1
#define OBJECT_DATA_SET			2
#define MATERIAL_SET			3

#include "util.glh"
#include "fs_data.glh"
#include "scenedata.glh"
#include "objectdata.glh"
#include "material.glh"

#include "light.glh"

layout(set = LIGHT_SET, binding = 2) uniform sampler2DMS ws_normal_map;

/*#include "lightbuffers.glh"
#include "shadowmatrices.glh"

layout(set = 0, binding = 3) uniform sampler2D ao_map;
layout(set = 0, binding = 5) uniform sampler2DArray shadow_map;
*/

layout(push_constant) uniform push_constants
{
	vec4 color;
	float roughness;
	float metallic;
} mat_data;

const float PI = 3.14159265359;
const uvec2 mask_pos[5] = { { 0xFC000000, 26 }, { 0x03F00000, 20 }, { 0x000FC000, 14 }, { 0x00003F00, 8 }, { 0x000000FC, 2 } };

float
attenuate(
	vec3 dir,
	float radius)
{
	float cutoff = 0.5;
	float attenuation = dot(dir, dir) / (100.0 * radius);
	attenuation = 1.0 / (attenuation * 15.0 + 1.0);
	attenuation = (attenuation - cutoff) / (1.0 - cutoff);

	return clamp(attenuation, 0.0, 1.0);
}

float
linstep(
	float low,
	float high,
	float val)
{
	return clamp((val - low) / (high - low), 0.0, 1.0);
}

float
shadow_map_id(
	uint map_id,
	float v)
{
	uint ret = uint(v) & mask_pos[map_id].x;
	return float(ret >> mask_pos[map_id].y);
}

float
calc_shadow(uint l_idx)
{
	/*float map_id = shadow_map_id(0, light_buffer.data[l_idx].direction.w);

	vec4 lsp = shadowMatrices.data[int(map_id)] * vec4(v_pos, 1.0);
	vec3 coords = (lsp.xyz / lsp.w);

	vec2 moments = texture(shadowMap, vec3(coords.xy, map_id)).rg;

	float p = step(coords.z, moments.x);
	float var = max(moments.y - moments.x * moments.x, 0.000002);

	float d = coords.z - moments.x;
	//float p_max = linstep(0.2, 1.0, var / (var + d * d));
	float p_max = smoothstep(0.2, 1.0, var / (var + d * d));

	return min(max(p, p_max), 1.0);*/

	return 1.0;
}

float
calc_dir_shadow(uint l_idx)
{
	// TODO: Cascaded
	/*float map_id = getShadowMapId(0, lightBuffer.data[l_idx].direction.w);

	vec4 lsp = shadowMatrices.data[int(map_id)] * vec4(v_pos, 1.0);
	vec3 coords = (lsp.xyz / lsp.w);

	vec2 moments = texture(shadowMap, vec3(coords.xy, map_id)).rg;

	float p = step(coords.z, moments.x);
	float var = max(moments.y - moments.x * moments.x, 0.000002);

	float d = coords.z - moments.x;
	//float p_max = linstep(0.2, 1.0, var / (var + d * d));
	float p_max = smoothstep(0.2, 1.0, var / (var + d * d));

	return min(max(p, p_max), 1.0);*/

	return 1.0;
}

float
d_ggx(
	float dot_nh,
	float roughness)
{
	float a = roughness * roughness;
	float a_sq = a * a;
	float d = dot_nh * dot_nh * (a_sq - 1.0) + 1.0;
	return a_sq / (PI * d * d);
}

float
g_schlick_smith_ggx(
	float dot_nl,
	float dot_nv,
	float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	float gl = dot_nl / (dot_nl * (1.0 - k) + k);
	float gv = dot_nv / (dot_nv * (1.0 - k) + k);
	return gl * gv;
}

vec3
f_schlick(
	vec3 f0,
	float cos_theta)
{
	return f0 + (1.0 - f0) * pow(1.0 - cos_theta, 5.0);
}

void main()
{
	ivec2 coord	= ivec2(gl_FragCoord.xy);
	ivec2 tile	= coord / ivec2(16, 16);
	uint tile_id	= tile.y * int(scene_data.x_tile_count) + tile.x;

	// material
	vec4 diffuse	= texture(mat_diffuse_map, v_uv) * mat_data.color;
	diffuse.rgb	= pow(diffuse.rgb, vec3(2.2));

	vec3 normal	= normalize(textureAverage(ws_normal_map, coord, scene_data.samples).xyz);
	float metallic	= texture(mat_metallic_map, v_uv).r * mat_data.metallic;
	float roughness	= texture(mat_roughness_map, v_uv).r * mat_data.roughness;
	float ao	= texture(mat_ao_map, v_uv).r;

	// lighting precalculations
	vec3 f0		= mix(vec3(0.04), diffuse.rgb, metallic);
	vec3 v_dir	= -normalize(scene_data.camera_pos.xyz - v_pos);
	float dot_nv	= clamp(dot(normal, v_dir), 0.0, 1.0);

	// lighting
	vec3 l_accum = vec3(0.0);
	/*for (uint i = 0;
	i < 1024 && visible_indices_buffer.data[offset + i] != -1; ++i) {
		uint l_idx = visible_indices_buffer.data[offset + i];*/
	for (uint i = 0; i < scene_data.light_count; ++i) {
		uint l_idx = i;
		vec3 l_dir = vec3(0.0);
		vec3 radiance = light_buffer.data[l_idx].color.rgb *
					light_buffer.data[l_idx].color.a;
		float attenuation = 1.0,
			spot_attenuation = 1.0,
			shadow = 1.0;

		uint l_type = uint(light_buffer.data[l_idx].position.w);
		if (l_type == LT_DIRECTIONAL) {

			l_dir = normalize(light_buffer.data[l_idx].direction.xyz);
			if (light_buffer.data[l_idx].direction.w > -1.0)
				shadow = calc_dir_shadow(l_idx);

		} else if (l_type == LT_POINT) {

			l_dir = light_buffer.data[l_idx].position.xyz - v_pos;
			float d = length(l_dir);
			attenuation = smoothstep(light_buffer.data[l_idx].data.y,
					light_buffer.data[l_idx].data.x, d);

			l_dir = normalize(l_dir);

			if (light_buffer.data[l_idx].direction.w > -1.0)
				shadow = calc_shadow(l_idx);

		} else if (l_type == LT_SPOT) {

			l_dir = light_buffer.data[l_idx].position.xyz - v_pos;
			float d = length(l_dir);
			attenuation = smoothstep(light_buffer.data[l_idx].data.y,
					light_buffer.data[l_idx].data.x, d);
			l_dir = normalize(l_dir);

			float inner_cutoff = light_buffer.data[l_idx].data.z;
			float outer_cutoff = light_buffer.data[l_idx].data.w;

			float e = inner_cutoff - outer_cutoff;
			float theta = dot(l_dir,
				normalize(-light_buffer.data[l_idx].direction.xyz));
			spot_attenuation = smoothstep(0.0, 1.0,
						(theta - outer_cutoff) / e);

			if (theta < light_buffer.data[l_idx].data.w)
				continue;

			if (light_buffer.data[l_idx].direction.w > -1.0)
				shadow = calc_shadow(l_idx);
		}

		l_dir *= -1;
		radiance *= attenuation * spot_attenuation;

		vec3 h		= normalize(v_dir + l_dir);
		float dot_nl	= clamp(dot(normal, l_dir), 0.0, 1.0);
		float dot_nh	= clamp(dot(normal, h), 0.0, 1.0);
		float dot_hv	= clamp(dot(h, v_dir), 0.0, 1.0);

		if (dot_nl > 0.0) {
			float d = d_ggx(dot_nh, roughness);
			float g = g_schlick_smith_ggx(dot_nl, dot_nv, roughness);
			vec3 f = f_schlick(f0, dot_hv);

			vec3 spec = d * f * g / (4.0 * dot_nl * dot_nv);

			l_accum += spec * dot_nl * radiance;
		}
	};

	vec3 color = 0.03 * diffuse.rgb * ao;
	color += l_accum;

	color = color / (color + vec3(1.0));
	color = pow(color, vec3(0.4545));

	o_FragColor = vec4(color, diffuse.a);
}
