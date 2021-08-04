#ifndef _RE_SHADER_TYPES_H_
#define _RE_SHADER_TYPES_H_

struct Vertex
{
	float x, y, z;
	float nx, ny, nz;
	float tx, ty, tz;
	float u, v;
};

struct UIVertex
{
	float x, y, u, v;
	vec4 color;
};

layout(std430, buffer_reference) readonly buffer VertexBuffer
{
	Vertex data[];
};

layout(std430, buffer_reference) readonly buffer UIVertexBuffer
{
	UIVertex data[];
};

#define NE_THREADS_X	0
#define NE_THREADS_Y	1
#define NE_THREADS_Z	2

#endif /* _RE_SHADER_TYPES_H_ */
