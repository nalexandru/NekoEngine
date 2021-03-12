#ifndef _SCN_TRANSFORM_H_
#define _SCN_TRANSFORM_H_

#include <Math/Math.h>

#include <Runtime/Runtime.h>
#include <Engine/Component.h>

struct Transform
{
	COMPONENT_BASE;

	struct mat4 mat;
	bool dirty;
	struct vec3 position, scale;
	struct quat rotation;
	
	struct vec3 forward, right, up;

	struct Transform *parent;
	struct Array children;
};

bool Scn_InitTransform(struct Transform *xform, const void **args);
void Scn_TermTransform(struct Transform *xform);
void Scn_UpdateTransform(void **comp, void *args);

static inline void
xform_move(struct Transform *t, struct vec3 *movement)
{
	v3_add(&t->position, &t->position, movement);
	t->dirty = true;
}

static inline void
xform_rotate(struct Transform *t, float angle, const struct vec3 *axis)
{
	quat_rot_axis_angle(&t->rotation, axis, angle);
	t->dirty = true;
}

static inline void
xform_scale(struct Transform *t, const struct vec3 *scale)
{
	v3_mul(&t->scale, &t->scale, scale);
	t->dirty = true;
}

static inline void
xform_set_pos(struct Transform *t, const struct vec3 *pos)
{
	v3_copy(&t->position, pos);
	t->dirty = true;
}

static inline void
xform_set_scale(struct Transform *t, const struct vec3 *scale)
{
	v3_copy(&t->scale, scale);
	t->scale.x = fmaxf(t->scale.x, .0000001f);
	t->scale.y = fmaxf(t->scale.y, .0000001f);
	t->scale.z = fmaxf(t->scale.z, .0000001f);
	t->dirty = true;
}

static inline void
xform_look_at(struct Transform *t, struct vec3 *target, struct vec3 *up)
{
	//
}

static inline void
xform_update(struct Transform *t)
{
	struct mat4 m1;
	struct mat4 m2;
	struct vec4 tmp;
	size_t i;

	if ((t->parent && !t->parent->dirty) && !t->dirty)
		return;

	m4_rot_quat(&m1, &t->rotation);

	v4_mul_m4(&tmp, v4(&tmp, 0.f, 0.f, 1.f, 1.f), &m1);
	v3(&t->forward, tmp.x, tmp.y, tmp.z);

	v4_mul_m4(&tmp, v4(&tmp, 1.f, 0.f, 0.f, 1.f), &m1);
	v3(&t->right, tmp.x, tmp.y, tmp.z);

	v4_mul_m4(&tmp, v4(&tmp, 0.f, 1.f, 0.f, 1.f), &m1);
	v3(&t->up, tmp.x, tmp.y, tmp.z);

	m4_translate(&m2, t->position.x, t->position.y, t->position.z);
	m4_mul(&m1, &m2, &m1);

	m4_scale(&m2, t->scale.x, t->scale.y, t->scale.z);
	m4_mul(&t->mat, &m1, &m2);

	for (i = 0; i < t->children.count; ++i)
		xform_update((struct Transform *)Rt_ArrayGetPtr(&t->children, i));

	t->dirty = false;
}

static inline struct vec3 *
xform_position(const struct Transform *t, struct vec3 *pos)
{
	if (t->parent) {
		struct vec3 tmp;
		xform_position(t->parent, pos);
		v3_add(pos, &tmp, &t->position);
	} else {
		memcpy(pos, &t->position, sizeof(*pos));
	}

	return pos;
}

static inline struct quat *
xform_rotation(const struct Transform *t, struct quat *rot)
{
	if (t->parent) {
		struct quat tmp;
		xform_rotation(t->parent, &tmp);
		quat_mul(rot, &tmp, &t->rotation);
	} else {
		memcpy(rot, &t->rotation, sizeof(*rot));
	}

	return rot;
}

static inline struct vec3 *
xform_rotation_angles(const struct Transform *t, struct vec3 *rot)
{
	struct quat q;
	xform_rotation(t, &q);

	rot->x = quat_roll(&q);
	rot->y = quat_pitch(&q);
	rot->z = quat_yaw(&q);

	return rot;
}

static inline void
xform_move_forward(struct Transform *t, float distance)
{
	struct vec3 tmp;
	v3_add(&t->position, &t->position, v3_muls(&tmp, &t->forward, distance));
	t->dirty = true;
}

static inline void
xform_move_backward(struct Transform *t, float distance)
{
	xform_move_forward(t, -distance);
}

static inline void
xform_move_right(struct Transform *t, float distance)
{
	struct vec3 tmp;
	v3_add(&t->position, &t->position, v3_muls(&tmp, &t->right, distance));
	t->dirty = true;
}

static inline void
xform_move_left(struct Transform *t, float distance)
{
	xform_move_right(t, -distance);
}

static inline void
xform_move_up(struct Transform *t, float distance)
{
	struct vec3 tmp;
	v3_add(&t->position, &t->position, v3_muls(&tmp, &t->up, distance));
	t->dirty = true;
}

static inline void
xform_move_down(struct Transform *t, float distance)
{
	xform_move_up(t, -distance);
}

#endif /* _SCN_TRANSOFRM_H_ */
