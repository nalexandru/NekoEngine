#ifndef _NE_SCENE_LIGHT_H_
#define _NE_SCENE_LIGHT_H_

#include <Math/Math.h>
#include <Engine/Types.h>
#include <Engine/Component.h>
#include <Runtime/Runtime.h>

#define SCN_COLLECT_LIGHTS	L"Scn_CollectLights"

enum LightType
{
	LT_DIRECTIONAL = 0,
	LT_POINT = 1,
	LT_SPOT = 2,

	LT_FORCE_UINT32 = 0xFFFFFFFF
};

struct Light
{
	COMPONENT_BASE;

	struct vec3 color;
	float intensity;
	enum LightType type;

	float innerRadius, outerRadius;
	float innerCutoff, outerCutoff;
};

#pragma pack(push, 1)
struct LightData
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

struct CollectLights
{
	struct Array lightData;
};

bool Scn_InitLight(struct Light *cam, const void **args);
void Scn_TermLight(struct Light *cam);

void Scn_CollectLights(void **comp, struct CollectLights *args);

#endif /* _NE_SCENE_LIGHT_H_ */
