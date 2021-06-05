#ifndef _RE_UI_DRAW_INFO_H_
#define _RE_UI_DRAW_INFO_H_

#include "Types.h"

layout(push_constant) uniform ConstantDrawInfo
{
	UIVertexBuffer vertices;
	uint texture;
	mat4 mvp;
} DrawInfo;

#endif /* _RE_UI_DRAW_INFO_H_ */
