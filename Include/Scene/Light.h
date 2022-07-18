#ifndef _NE_SCENE_LIGHT_H_
#define _NE_SCENE_LIGHT_H_

#include <Math/Math.h>
#include <Engine/Types.h>
#include <Engine/Component.h>
#include <Runtime/Runtime.h>

#define SCN_COLLECT_LIGHTS		"Scn_CollectLights"

enum NeLightType
{
	LT_DIRECTIONAL = 0,
	LT_POINT = 1,
	LT_SPOT = 2,

	LT_FORCE_UINT32 = 0xFFFFFFFF
};

struct NeLight
{
	NE_COMPONENT_BASE;

	struct NeVec3 color;
	float intensity;
	enum NeLightType type;

	float innerRadius, outerRadius;
	float innerCutoff, outerCutoff;
};

#pragma pack(push, 1)
struct NeLightData
{
	float position[3];
	uint32_t type;

	float direction[3];
	uint32_t __padding;

	float color[3];
	float intensity;

	float innerRadius, outerRadius;
	float innerCutoff, outerCutoff;
};
#pragma pack(pop)

struct NeCollectLights
{
	struct NeArray lightData;
};

#endif /* _NE_SCENE_LIGHT_H_ */
