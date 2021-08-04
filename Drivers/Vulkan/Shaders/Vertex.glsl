#ifndef _RE_VERTEX_H_
#define _RE_VERTEX_H_

#include "Types.glsl"

vec3 Re_Position()
{
	return vec3(
		DrawInfo.vertices.data[gl_VertexIndex].x,
		DrawInfo.vertices.data[gl_VertexIndex].y,
		DrawInfo.vertices.data[gl_VertexIndex].z
	);
}

vec3 Re_Normal()
{
	return vec3(
		DrawInfo.vertices.data[gl_VertexIndex].nx,
		DrawInfo.vertices.data[gl_VertexIndex].ny,
		DrawInfo.vertices.data[gl_VertexIndex].nz
	);
}

vec3 Re_Tangent()
{
	return vec3(
		DrawInfo.vertices.data[gl_VertexIndex].tx,
		DrawInfo.vertices.data[gl_VertexIndex].ty,
		DrawInfo.vertices.data[gl_VertexIndex].tz
	);
}

vec2 Re_TexCoord()
{
	return vec2(
		DrawInfo.vertices.data[gl_VertexIndex].u,
		DrawInfo.vertices.data[gl_VertexIndex].v
	);
}

vec3 Re_I_Position()
{
	return vec3(
		DrawInfo.instance.vertices.data[gl_VertexIndex].x,
		DrawInfo.instance.vertices.data[gl_VertexIndex].y,
		DrawInfo.instance.vertices.data[gl_VertexIndex].z
	);
}

vec3 Re_I_Normal()
{
	return vec3(
		DrawInfo.instance.vertices.data[gl_VertexIndex].nx,
		DrawInfo.instance.vertices.data[gl_VertexIndex].ny,
		DrawInfo.instance.vertices.data[gl_VertexIndex].nz
	);
}

vec3 Re_I_Tangent()
{
	return vec3(
		DrawInfo.instance.vertices.data[gl_VertexIndex].tx,
		DrawInfo.instance.vertices.data[gl_VertexIndex].ty,
		DrawInfo.instance.vertices.data[gl_VertexIndex].tz
	);
}

vec2 Re_I_TexCoord()
{
	return vec2(
		DrawInfo.instance.vertices.data[gl_VertexIndex].u,
		DrawInfo.instance.vertices.data[gl_VertexIndex].v
	);
}

#endif /* _RE_VERTEX_H_ */
