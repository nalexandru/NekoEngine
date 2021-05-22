#ifndef _RE_DRAW_INFO_H_
#define _RE_DRAW_INFO_H_

#include "Material.h"

layout(push_constant) uniform ConstantDrawInfo 
{
	uint vertexBuffer;
	MaterialBuffer material;
} DrawInfo;

#endif /* _RE_DRAW_INFO_ */
