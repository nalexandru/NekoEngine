#include <Scene/Systems.h>
#include <Engine/ECSystem.h>
#include <Scene/Transform.h>
#include <Scene/Components.h>

static bool _InitTransform(struct NeTransform *xform, const void **args);
static void _TermTransform(struct NeTransform *xform);

E_REGISTER_COMPONENT(TRANSFORM_COMP, struct NeTransform, 16, _InitTransform, _TermTransform)

static bool
_InitTransform(struct NeTransform *xform, const void **args)
{
	M_Vec3(&xform->position, 0.f, 0.f, 0.f);
	M_Vec3(&xform->scale, 1.f, 1.f, 1.f);
	M_Identity(&xform->rotation);

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "Parent", len)) {
		//	if (!strncmp(*(++args), "true", 4))
		//		Scn_ActiveCamera = cam;
		} else if (!strncmp(arg, "ParentPtr", len)) {
			xform->parent = (struct NeTransform *)*(++args);
		} else if (!strncmp(arg, "Position", len)) {
			char *ptr = (char *)*(++args);
			xform->position.x = strtof(ptr, &ptr);
			xform->position.y = strtof(ptr + 2, &ptr);
			xform->position.z = strtof(ptr + 2, &ptr);
		} else if (!strncmp(arg, "Rotation", len)) {
			struct NeVec3 r;
			char *ptr = (char *)*(++args);
			r.x = strtof(ptr, &ptr);
			r.y = strtof(ptr + 2, &ptr);
			r.z = strtof(ptr + 2, &ptr);
			M_QuatRotationPitchYawRoll(&xform->rotation, r.x, r.y, r.z);
		} else if (!strncmp(arg, "Scale", len)) {
			char *ptr = (char *)*(++args);
			xform->scale.x = strtof(ptr, &ptr);
			xform->scale.y = strtof(ptr + 2, &ptr);
			xform->scale.z = strtof(ptr + 2, &ptr);
		}
	}

	M_Identity(&xform->mat);
	xform_update(xform);

	return true;
}

static void
_TermTransform(struct NeTransform *xform)
{
	//
}

E_SYSTEM(SCN_UPDATE_TRANSFORM, ECSYS_GROUP_PRE_RENDER, ECSYS_PRI_TRANSFORM, false, void, 1, TRANSFORM_COMP)
{
	struct NeTransform *xform = (struct NeTransform *)comp[0];
	
	if (!xform->parent)
		xform_update(xform);
}
