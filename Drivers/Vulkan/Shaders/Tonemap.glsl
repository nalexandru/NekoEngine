#ifndef _RE_TONEMAP_H_
#define _RE_TONEMAP_H_

#include "DrawInfo.glsl"

// https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl

const mat3 ACESInputMat =
{
	{ 0.59719, 0.35458, 0.04823 },
	{ 0.07600, 0.90834, 0.01566 },
	{ 0.02840, 0.13383, 0.83777 }
};

const mat3 ACESOutputMat =
{
	{  1.60475, -0.53108, -0.07367 },
	{ -0.10208,  1.10813, -0.00605 },
	{ -0.00327, -0.07276,  1.07602 }
};

const vec3 a = { 0.0245786, 0.0245786, 0.0245786 };
const vec3 b = { 0.000090537, 0.000090537, 0.000090537 };
const vec3 c = { 0.983729, 0.983729, 0.983729 };
const vec3 d = { 0.4329510, 0.4329510, 0.4329510 };
const vec3 e = { 0.238081, 0.238081, 0.238081 };

vec4
sRGBtoLinear(vec4 color)
{
	return vec4(pow(color.rgb, vec3(DrawInfo.scene.gamma)), color.a);
}

vec3
tonemap(vec3 color)
{
	color = (color * vec3(DrawInfo.scene.exposure)) * ACESInputMat;
	color = (color * (color + a) - b) / (color * (c * color + d) + e);

	return pow(clamp(color * ACESOutputMat, 0.0, 1.0), vec3(DrawInfo.scene.invGamma));
}

#endif /* _RE_TONEMAP_H_ */
