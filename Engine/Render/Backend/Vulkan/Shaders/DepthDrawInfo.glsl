#ifndef _RE_DEPTH_DRAW_INFO_H_
#define _RE_DEPTH_DRAW_INFO_H_

#include "Types.glsl"
#include "Scene.glsl"
#include "Material.glsl"

layout(push_constant) uniform ConstantDrawInfo 
{
	VertexBuffer vertices;
	MaterialBuffer material;
	InstanceBuffer instance;
} DrawInfo;

#endif /* _RE_DEPTH_DRAW_INFO_ */
