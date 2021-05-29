#ifndef _RE_DRAW_INFO_H_
#define _RE_DRAW_INFO_H_

#include "Types.h"
#include "Material.h"

layout(push_constant) uniform ConstantDrawInfo 
{
	VertexBuffer vertices;
	MaterialBuffer material;
	mat4 mvp;
} DrawInfo;

#endif /* _RE_DRAW_INFO_ */
