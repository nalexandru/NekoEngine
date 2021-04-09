#ifndef _RE_OBJECT_H_
#define _RE_OBJECT_H_

struct Object
{
	uint16_t vertexBuffer;
	uint16_t indexBuffer;
	uint16_t material;
};

layout(std430, set = 0, binding = 2) readonly buffer ObjectBuffer
{
	Object data[];
}; Re_objectBuffers[];

#endif /* _RE_OBJECT_H_ */

