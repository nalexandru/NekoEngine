#ifndef _RE_MATERIAL_H_
#define _RE_MATERIAL_H_

struct Material
{
	float a;
};

layout(std430, set = 0, binding = 2) readonly buffer MaterialBuffer
{
	Material data[];
} Re_materialBuffers[];


#endif /* _RE_MATERIAL_H_ */