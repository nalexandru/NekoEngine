#include <Scene/Transform.h>

bool
Scn_InitTransform(struct Transform *xform, const void **args)
{
	v3(&xform->position, 0.f, 0.f, 0.f);
	v3(&xform->scale, 1.f, 1.f, 1.f);
	quat_ident(&xform->rotation);

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "Parent", len)) {
		//	if (!strncmp(*(++args), "true", 4))
		//		Scn_ActiveCamera = cam;
		} else if (!strncmp(arg, "ParentPtr", len)) {
			xform->parent = (struct Transform *)*(++args);
		} else if (!strncmp(arg, "Position", len)) {
			char *ptr = (char *)*(++args);
			xform->position.x = strtof(ptr, &ptr);
			xform->position.y = strtof(ptr + 2, &ptr);
			xform->position.z = strtof(ptr + 2, &ptr);
		} else if (!strncmp(arg, "Rotation", len)) {
			struct vec3 r;
			char *ptr = (char *)*(++args);
			r.x = strtof(ptr, &ptr);
			r.y = strtof(ptr + 2, &ptr);
			r.z = strtof(ptr + 2, &ptr);
			quat_rot_pitch_yaw_roll(&xform->rotation, r.x, r.y, r.z);
		} else if (!strncmp(arg, "Scale", len)) {
			char *ptr = (char *)*(++args);
			xform->scale.x = strtof(ptr, &ptr);
			xform->scale.y = strtof(ptr + 2, &ptr);
			xform->scale.z = strtof(ptr + 2, &ptr);
		}
	}

	m4_ident(&xform->mat);
	xform_update(xform);

	return true;
}

void
Scn_TermTransform(struct Transform *xform)
{
	//
}

void
Scn_UpdateTransform(void **comp, void *args)
{
	struct Transform *xform = (struct Transform *)comp[0];
	
	if (!xform->parent)
		xform_update(xform);
}
