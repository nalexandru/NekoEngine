#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Engine/Component.h>
#include <Scene/Components.h>
#include <Scene/Transform.h>
#include <Scene/Light.h>

#define MOVING_LIGHT	"NativeMovingLight"
#define MOVE_LIGHTS		"NativeMoveLights"

struct MovingLight
{
	NE_COMPONENT_BASE;

	float speed, direction, maxHeight;
};

static bool InitMovingLight(struct MovingLight *ml, const char **args);
static void TermMovingLight(struct MovingLight *ml);

NE_REGISTER_COMPONENT(MOVING_LIGHT, struct MovingLight, 1, InitMovingLight, NULL, TermMovingLight);

static bool
InitMovingLight(struct MovingLight *ml, const char **args)
{
	float maxHeight = 0.f, maxWidth = 0.f, maxDepth = 0.f;

	ml->speed = 0.f;
	ml->maxHeight = 0.f;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "Speed", len)) {
			ml->speed = (float)atof(*(++args));
		} else if (!strncmp(arg, "MaxWidth", len)) {
			maxWidth = (float)atof(*(++args));
		} else if (!strncmp(arg, "MaxHeight", len)) {
			maxHeight = (float)atof(*(++args));
		} else if (!strncmp(arg, "MaxDepth", len)) {
			maxDepth = (float)atof(*(++args));
		}
	}

	struct NeTransform *xform = (struct NeTransform *)E_GetComponent(ml->_owner, NE_TRANSFORM_ID);
	struct NeLight *light = (struct NeLight *)E_GetComponent(ml->_owner, NE_LIGHT_ID);

	struct NeVec3 pos =
	{
		(((float)(rand() % 100) / 100.f) * 2.f - 1.f) * maxWidth,
		((float)(rand() % 100) / 100.f) * maxHeight,
		(((float)(rand() % 100) / 100.f) * 2.f - 1.f) * maxDepth
	};
	Xform_SetPosition(xform, &pos);

	light->color.x = (float)(rand() % 100) / 100.f;
	light->color.y = (float)(rand() % 100) / 100.f;
	light->color.z = (float)(rand() % 100) / 100.f;

	ml->maxHeight = maxHeight;
	ml->direction = pos.y > maxHeight / 2.f ? 1.f : -1.f;

	return true;
}

static void TermMovingLight(struct MovingLight *ml) { }

NE_SYSTEM(MOVE_LIGHTS, ECSYS_GROUP_LOGIC, 0, false, void, 2, NE_TRANSFORM, MOVING_LIGHT)
{
	struct NeTransform *xform = (struct NeTransform *)comp[0];
	struct MovingLight *ml = (struct MovingLight *)comp[1];

	struct NeVec3 pos;
	Xform_Position(xform, &pos);

	if (pos.y > ml->maxHeight || pos.y < 0.0)
		ml->direction *= -1.f;

	Xform_MoveUp(xform, ml->speed * ml->direction * (float)E_deltaTime);
}