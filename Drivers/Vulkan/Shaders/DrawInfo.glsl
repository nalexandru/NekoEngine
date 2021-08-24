#ifndef _RE_DRAW_INFO_H_
#define _RE_DRAW_INFO_H_

#include "Types.glsl"
#include "Scene.glsl"
#include "Light.glsl"
#include "Material.glsl"

layout(push_constant) uniform ConstantDrawInfo 
{
	VertexBuffer vertices;
	SceneBuffer scene;
	LightBuffer lights;
	InstanceBuffer instance;
	MaterialBuffer material;
} DrawInfo;

#endif /* _RE_DRAW_INFO_ */
