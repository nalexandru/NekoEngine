#include <Scene/Scene.h>
#include <Scene/Systems.h>
#include <Engine/Entity.h>
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

	xform->parent = E_INVALID_HANDLE;
	xform->dirty = true;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "Parent", len)) {
			struct NeScene *s = Scn_GetScene((uint8_t)xform->_sceneId);

			NeEntityHandle parent = E_FindEntityS(s, (char *)*(++args));
			xform->parent = E_GetComponentHandleS(s, parent, E_ComponentTypeId(TRANSFORM_COMP));

			struct NeTransform *parentPtr = E_ComponentPtrS(s, xform->parent);
			NeCompHandle self = xform->_handleId;
			Rt_ArrayAdd(&parentPtr->children, &self);
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

	Rt_InitArray(&xform->children, 2, sizeof(NeCompHandle), MH_Scene);

	M_Identity(&xform->mat);

	return true;
}

static void
_TermTransform(struct NeTransform *xform)
{
	Rt_TermArray(&xform->children);
}

E_SYSTEM(SCN_UPDATE_TRANSFORM, ECSYS_GROUP_PRE_RENDER, ECSYS_PRI_TRANSFORM, false, void, 1, TRANSFORM_COMP)
{
	struct NeTransform *xform = (struct NeTransform *)comp[0];
	
	if (xform->parent == E_INVALID_HANDLE)
		xform_update(xform);
}
