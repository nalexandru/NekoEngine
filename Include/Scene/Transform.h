#ifndef _NE_SCENE_TRANSFORM_H_
#define _NE_SCENE_TRANSFORM_H_

#include <Math/Math.h>

#include <Runtime/Runtime.h>
#include <Engine/Component.h>
#include <Engine/Entity.h>
#include <Scene/Scene.h>

#ifdef __cplusplus
extern "C" {
#endif

struct NeTransform
{
	NE_COMPONENT_BASE;

	struct NeMatrix mat;
	bool dirty;
	struct NeVec3 position, scale;
	struct NeQuaternion rotation;
	
	struct NeVec3 forward, right, up;

	NeCompHandle parent;
	struct NeArray children;
};

void Scn_UpdateTransform(void **comp, void *args);

static inline void
xform_move(struct NeTransform *t, const struct NeVec3 *movement)
{
	M_Store(&t->position, XMVectorAdd(M_Load(&t->position), M_Load(movement)));
	t->dirty = true;
}

static inline void
xform_rotate(struct NeTransform *t, float angle, const struct NeVec3 *axis)
{
	M_Store(&t->rotation,
		XMQuaternionMultiply(M_Load(&t->rotation), XMQuaternionRotationAxis(M_Load(axis), XMConvertToRadians(angle))));
	t->dirty = true;
}

static inline void
xform_scale(struct NeTransform *t, const struct NeVec3 *scale)
{
	M_Store(&t->position, XMVectorMultiply(M_Load(&t->position), M_Load(scale)));
	t->dirty = true;
}

static inline void
xform_set_pos(struct NeTransform *t, const struct NeVec3 *pos)
{
	memcpy(&t->position, pos, sizeof(t->position));
	t->dirty = true;
}

static inline void
xform_set_scale(struct NeTransform *t, const struct NeVec3 *scale)
{
	memcpy(&t->scale, scale, sizeof(t->scale));
	t->scale.x = fmaxf(t->scale.x, .0000001f);
	t->scale.y = fmaxf(t->scale.y, .0000001f);
	t->scale.z = fmaxf(t->scale.z, .0000001f);
	t->dirty = true;
}

static inline void
xform_look_at(struct NeTransform *t, struct NeVec3 *target, struct NeVec3 *up)
{
	M_Store(&t->mat, XMMatrixLookAtRH(M_Load(&t->position), M_Load(target), M_Load(up)));
}

static inline void
xform_update_orientation_mat(struct NeTransform *t, const XMMATRIX &mat)
{
	M_Store(&t->forward, XMVector4Transform(XMVectorSet(0.f, 0.f, 1.f, 1.f), mat));
	M_Store(&t->right,   XMVector4Transform(XMVectorSet(1.f, 0.f, 0.f, 1.f), mat));
	M_Store(&t->up,      XMVector4Transform(XMVectorSet(0.f, 1.f, 0.f, 1.f), mat));
}

static inline void
xform_update_orientation(struct NeTransform *t)
{
	xform_update_orientation_mat(t, XMMatrixRotationQuaternion(M_Load(&t->rotation)));
}

static inline void
xform_update(struct NeTransform *t)
{
	struct NeScene *s = Scn_GetScene((uint8_t)t->_sceneId);
	const struct NeTransform *parent = (struct NeTransform *)E_ComponentPtrS(s, t->parent);

	if ((t->parent != E_INVALID_HANDLE && !parent->dirty) && !t->dirty)
		return;

	XMMATRIX mat = XMMatrixMultiply(XMMatrixScaling(t->scale.x, t->scale.y, t->scale.z),
		XMMatrixRotationQuaternion(M_Load(&t->rotation)));
	mat = XMMatrixMultiply(mat, XMMatrixTranslation(t->position.x, t->position.y, t->position.z));

	if (parent)
		mat = XMMatrixMultiply(mat, M_Load(&parent->mat));

	M_Store(&t->mat, mat);

	for (size_t i = 0; i < t->children.count; ++i)
		xform_update((struct NeTransform *)E_ComponentPtrS(s, *((NeCompHandle *)Rt_ArrayGet(&t->children, i))));

	t->dirty = false;
}

static inline struct NeVec3 *
xform_position(const struct NeTransform *t, struct NeVec3 *pos)
{
	if (t->parent != E_INVALID_HANDLE) {
		struct NeVec3 tmp;
		const struct NeTransform *parent = (struct NeTransform *)E_ComponentPtr(t->parent);

		xform_position(parent, &tmp);
		M_Store(pos, XMVectorAdd(M_Load(&tmp), M_Load(&t->position)));
	} else {
		memcpy(pos, &t->position, sizeof(*pos));
	}

	return pos;
}

static inline struct NeQuaternion *
xform_rotation(const struct NeTransform *t, struct NeQuaternion *rot)
{
	if (t->parent != E_INVALID_HANDLE) {
		struct NeQuaternion tmp;
		const struct NeTransform *parent = (struct NeTransform *)E_ComponentPtr(t->parent);

		xform_rotation(parent, &tmp);
		M_Store(rot, XMQuaternionMultiply(M_Load(&tmp), M_Load(&t->rotation)));
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
	M_Store(&t->position, XMVectorAdd(M_Load(&t->position), XMVectorMultiply(M_Load(&t->forward), XMVectorReplicate(-distance))));
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
	M_Store(&t->position, XMVectorAdd(M_Load(&t->position), XMVectorMultiply(M_Load(&t->right), XMVectorReplicate(distance))));
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
	M_Store(&t->position, XMVectorAdd(M_Load(&t->position), XMVectorMultiply(M_Load(&t->up), XMVectorReplicate(distance))));
	t->dirty = true;
}

static inline void
xform_move_down(struct NeTransform *t, float distance)
{
	xform_move_up(t, -distance);
}

#ifdef __cplusplus
}
#endif

#endif /* _NE_SCENE_TRANSFORM_H_ */

/* NekoEngine
 *
 * Transform.h
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
