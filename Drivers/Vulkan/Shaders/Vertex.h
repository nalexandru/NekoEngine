#ifndef _RE_VERTEX_H_
#define _RE_VERTEX_H_

struct Vertex
{
	float x, y, z;
	float nx, ny, nz;
	float tx, ty, tz;
	float u, v;
};

layout(std430, set = 0, binding = 2) readonly buffer VertexBuffer
{
	Vertex data[];
} Re_vertexBuffers[];

vec3 Re_Position()
{
	return vec3(
		Re_vertexBuffers[0].data[gl_VertexIndex].x,
		Re_vertexBuffers[0].data[gl_VertexIndex].y,
		Re_vertexBuffers[0].data[gl_VertexIndex].z
	);
}

vec3 Re_Normal()
{
	return vec3(
		Re_vertexBuffers[0].data[gl_VertexIndex].nx,
		Re_vertexBuffers[0].data[gl_VertexIndex].ny,
		Re_vertexBuffers[0].data[gl_VertexIndex].nz
	);
}

vec3 Re_Tangent()
{
	return vec3(
		Re_vertexBuffers[0].data[gl_VertexIndex].tx,
		Re_vertexBuffers[0].data[gl_VertexIndex].ty,
		Re_vertexBuffers[0].data[gl_VertexIndex].tz
	);
}

vec2 Re_TexCoord()
{
	return vec2(
		Re_vertexBuffers[0].data[gl_VertexIndex].u,
		Re_vertexBuffers[0].data[gl_VertexIndex].v
	);
}

#endif /* _RE_VERTEX_H_ */

