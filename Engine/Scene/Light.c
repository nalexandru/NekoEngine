#include <Math/Math.h>
#include <Scene/Light.h>
#include <Scene/Transform.h>
#include <Scene/Components.h>
#include <Engine/Engine.h>
#include <Engine/Event.h>
#include <Engine/Events.h>
#include <Engine/ECSystem.h>

static bool _InitLight(struct NeLight *cam, const void **args);
static void _TermLight(struct NeLight *cam);

E_REGISTER_COMPONENT(LIGHT_COMP, struct NeLight, 16, _InitLight, _TermLight)

static bool
_InitLight(struct NeLight *l, const void **args)
{
	l->type = LT_DIRECTIONAL;
	M_Fill(&l->color, 1.f);
	l->intensity = 1.f;

	l->innerRadius = l->outerRadius = l->innerCutoff = l->outerCutoff = 0.f;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "Type", len)) {
			const char *val = *(++args);
			size_t vLen = strlen(val);

			if (!strncmp(val, "Directional", vLen))
				l->type = LT_DIRECTIONAL;
			else if (!strncmp(val, "Point", vLen))
				l->type = LT_POINT;
			else if (!strncmp(val, "Spot", vLen))
				l->type = LT_SPOT;
		} else if (!strncmp(arg, "Color", len)) {
			char *ptr = (char *)*(++args);
			l->color.x = strtof(ptr, &ptr);
			l->color.y = strtof(ptr + 2, &ptr);
			l->color.z = strtof(ptr + 2, &ptr);
		} else if (!strncmp(arg, "Intensity", len)) {
			l->intensity = (float)atof(*(++args));
		} else if (!strncmp(arg, "InnerRadius", len)) {
			l->innerRadius = (float)atof(*(++args));
		} else if (!strncmp(arg, "OuterRadius", len)) {
			l->outerRadius = (float)atof(*(++args));
		} else if (!strncmp(arg, "InnerCutoff", len)) {
			l->innerCutoff = cosf(M_DegToRad((float)atof(*(++args))));
		} else if (!strncmp(arg, "OuterCutoff", len)) {
			l->outerCutoff = cosf(M_DegToRad((float)atof(*(++args))));
		}
	}

	return true;
}

E_SYSTEM(SCN_COLLECT_LIGHTS, ECSYS_GROUP_MANUAL, 0, true, struct NeCollectLights, 2, TRANSFORM_COMP, LIGHT_COMP)
{
	struct NeTransform *xform = comp[0];
	struct NeLight *l = comp[1];

	float x = M_DegToRad(M_QuatRoll(&xform->rotation));
	float y = M_DegToRad(M_QuatPitch(&xform->rotation));

	const float cosy = cosf(y);

	const float yPos = l->type != LT_DIRECTIONAL ? xform->position.y : -xform->position.y;

	struct NeLightData ld =
	{
		.position = { xform->position.x, yPos, xform->position.z },
		.type = l->type,
		.direction =
		{
			sinf(x) * cosy,
			sinf(y),
			cosf(x) * cosy 
		},
		.color = { l->color.x, l->color.y, l->color.z },
		.intensity = l->intensity,
		.innerRadius = l->innerRadius,
		.outerRadius = l->outerRadius,
		.innerCutoff = l->innerCutoff,
		.outerCutoff = l->outerCutoff
	};

	Rt_ArrayAdd(&args->lightData, &ld);
}

static void
_TermLight(struct NeLight *l)
{
}
