#ifndef _RE_VERTEX_H_
#define _RE_VERTEX_H_

#include "Types.h"

vec3 Re_Position()
{
	return vec3(
		DrawInfo.vertices.data[gl_VertexIndex].x,
		DrawInfo.vertices.data[gl_VertexIndex].y,
		DrawInfo.vertices.data[gl_VertexIndex].z
		/*Re_vertexBuffers[DrawInfo.vertexBuffer].data[gl_VertexIndex].x,
		Re_vertexBuffers[DrawInfo.vertexBuffer].data[gl_VertexIndex].y,
		Re_vertexBuffers[DrawInfo.vertexBuffer].data[gl_VertexIndex].z*/
	);
}

vec3 Re_Normal()
{
	return vec3(
		DrawInfo.vertices.data[gl_VertexIndex].nx,
		DrawInfo.vertices.data[gl_VertexIndex].ny,
		DrawInfo.vertices.data[gl_VertexIndex].nz
		/*Re_vertexBuffers[DrawInfo.vertexBuffer].data[gl_VertexIndex].nx,
		Re_vertexBuffers[DrawInfo.vertexBuffer].data[gl_VertexIndex].ny,
		Re_vertexBuffers[DrawInfo.vertexBuffer].data[gl_VertexIndex].nz*/
	);
}

vec3 Re_Tangent()
{
	return vec3(
		DrawInfo.vertices.data[gl_VertexIndex].tx,
		DrawInfo.vertices.data[gl_VertexIndex].ty,
		DrawInfo.vertices.data[gl_VertexIndex].tz
		/*Re_vertexBuffers[DrawInfo.vertexBuffer].data[gl_VertexIndex].tx,
		Re_vertexBuffers[DrawInfo.vertexBuffer].data[gl_VertexIndex].ty,
		Re_vertexBuffers[DrawInfo.vertexBuffer].data[gl_VertexIndex].tz*/
	);
}

vec2 Re_TexCoord()
{
	return vec2(
		DrawInfo.vertices.data[gl_VertexIndex].u,
		DrawInfo.vertices.data[gl_VertexIndex].v
		/*Re_vertexBuffers[DrawInfo.vertexBuffer].data[gl_VertexIndex].u,
		Re_vertexBuffers[DrawInfo.vertexBuffer].data[gl_VertexIndex].v*/
	);
}

#endif /* _RE_VERTEX_H_ */
