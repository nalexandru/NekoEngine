#ifndef _NE_SCENE_TRANSFORM_H_
#define _NE_SCENE_TRANSFORM_H_

#include <Math/Math.h>

#include <Runtime/Runtime.h>
#include <Engine/Component.h>

struct NeTransform
{
	NE_COMPONENT_BASE;

	struct NeMatrix mat;
	bool dirty;
	struct NeVec3 position, scale;
	struct NeQuaternion rotation;
	
	struct NeVec3 forward, right, up;

	struct NeTransform *parent;
	struct NeArray children;
};

void Scn_UpdateTransform(void **comp, void *args);

static inline void
xform_move(struct NeTransform *t, struct NeVec3 *movement)
{
	M_Add(&t->position, &t->position, movement);
	t->dirty = true;
}

static inline void
xform_rotate(struct NeTransform *t, float angle, const struct NeVec3 *axis)
{
	struct NeQuaternion tmp;
	M_QuatFromAxisAngle(&tmp, axis, angle);
	M_Mul(&t->rotation, &t->rotation, &tmp);
	t->dirty = true;
}

static inline void
xform_scale(struct NeTransform *t, const struct NeVec3 *scale)
{
	M_Mul(&t->scale, &t->scale, scale);
	//M_MulVec3(&t->scale, &t->scale, scale);
	t->dirty = true;
}

static inline void
xform_set_pos(struct NeTransform *t, const struct NeVec3 *pos)
{
	M_Copy(&t->position, pos);
	t->dirty = true;
}

static inline void
xform_set_scale(struct NeTransform *t, const struct NeVec3 *scale)
{
	M_Copy(&t->scale, scale);
	t->scale.x = fmaxf(t->scale.x, .0000001f);
	t->scale.y = fmaxf(t->scale.y, .0000001f);
	t->scale.z = fmaxf(t->scale.z, .0000001f);
	t->dirty = true;
}

static inline void
xform_look_at(struct NeTransform *t, struct NeVec3 *target, struct NeVec3 *up)
{
	//
}

static inline void
xform_update_orientation_mat(struct NeTransform *t, struct NeMatrix *rot)
{
	struct NeVec4 tmp;

	M_MulVec4Matrix(&tmp, M_Vec4(&tmp, 0.f, 0.f, 1.f, 1.f), rot);
	M_Vec3(&t->forward, tmp.x, tmp.y, tmp.z);

	M_MulVec4Matrix(&tmp, M_Vec4(&tmp, 1.f, 0.f, 0.f, 1.f), rot);
	M_Vec3(&t->right, tmp.x, tmp.y, tmp.z);

	M_MulVec4Matrix(&tmp, M_Vec4(&tmp, 0.f, 1.f, 0.f, 1.f), rot);
	M_Vec3(&t->up, tmp.x, tmp.y, tmp.z);
}

static inline void
xform_update_orientation(struct NeTransform *t)
{
	struct NeMatrix rot;
	M_RotationMatrixFromQuat(&rot, &t->rotation);
	xform_update_orientation_mat(t, &rot);
}

static inline void
xform_update(struct NeTransform *t)
{
	struct NeMatrix m1;
	struct NeMatrix m2;
	size_t i;

	if ((t->parent && !t->parent->dirty) && !t->dirty)
		return;

	M_RotationMatrixFromQuat(&m1, &t->rotation);
	xform_update_orientation_mat(t, &m1);

	M_TranslationMatrix(&m2, t->position.x, t->position.y, t->position.z);
	M_MulMatrix(&m1, &m2, &m1);

	M_ScaleMatrix(&m2, t->scale.x, t->scale.y, t->scale.z);
	M_MulMatrix(&t->mat, &m1, &m2);

	for (i = 0; i < t->children.count; ++i)
		xform_update((struct NeTransform *)Rt_ArrayGetPtr(&t->children, i));

	t->dirty = false;
}

static inline struct NeVec3 *
xform_position(const struct NeTransform *t, struct NeVec3 *pos)
{
	if (t->parent) {
		struct NeVec3 tmp;
		xform_position(t->parent, &tmp);
		M_Add(pos, &tmp, &t->position);
	} else {
		memcpy(pos, &t->position, sizeof(*pos));
	}

	return pos;
}

static inline struct NeQuaternion *
xform_rotation(const struct NeTransform *t, struct NeQuaternion *rot)
{
	if (t->parent) {
		struct NeQuaternion tmp;
		xform_rotation(t->parent, &tmp);
		M_Mul(rot, &tmp, &t->rotation);
	} else {
		memcpy(rot, &t->rotation, sizeof(*rot));
	}

	return rot;
}

static inline void
xform_rotation_angles_f(const struct NeTransform *t, float *pitch, float *yaw, float *roll)
{
	struct NeQuaternion q;
	xform_rotation(t, &q);

	if (pitch)
		*pitch = M_QuatPitch(&q);

	if (yaw)
		*yaw = M_QuatYaw(&q);

	if (roll)
		*roll = M_QuatRoll(&q);
}

static inline struct NeVec3 *
xform_rotation_angles(const struct NeTransform *t, struct NeVec3 *rot)
{
	struct NeQuaternion q;
	xform_rotation(t, &q);

	rot->x = M_QuatPitch(&q);
	rot->y = M_QuatYaw(&q);
	rot->z = M_QuatRoll(&q);

	xform_rotation_angles_f(t, &rot->x, &rot->y, &rot->z);

	return rot;
}

static inline void
xform_move_forward(struct NeTransform *t, float distance)
{
	struct NeVec3 tmp;
	M_Add(&t->position, &t->position, M_MulVec3S(&tmp, &t->forward, distance));
	t->dirty = true;
}

static inline void
xform_move_backward(struct NeTransform *t, float distance)
{
	xform_move_forward(t, -distance);
}

static inline void
xform_move_right(struct NeTransform *t, float distance)
{
	struct NeVec3 tmp;
	M_Add(&t->position, &t->position, M_MulVec3S(&tmp, &t->right, distance));
	t->dirty = true;
}

static inline void
xform_move_left(struct NeTransform *t, float distance)
{
	xform_move_right(t, -distance);
}

static inline void
xform_move_up(struct NeTransform *t, float distance)
{
	struct NeVec3 tmp;
	M_Add(&t->position, &t->position, M_MulVec3S(&tmp, &t->up, distance));
	t->dirty = true;
}

static inline void
xform_move_down(struct NeTransform *t, float distance)
{
	xform_move_up(t, -distance);
}

#endif /* _NE_SCENE_TRANSFORM_H_ */
