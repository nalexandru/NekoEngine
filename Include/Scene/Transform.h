#ifndef NE_SCENE_TRANSFORM_H
#define NE_SCENE_TRANSFORM_H

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
Xform_UpdateOrientation(struct NeTransform *t)
{
	const XMMATRIX mat = XMMatrixRotationQuaternion(M_Load(&t->rotation));
	M_Store(&t->forward, XMVector4Transform(XMVectorSet(0.f, 0.f, 1.f, 1.f), mat));
	M_Store(&t->right,   XMVector4Transform(XMVectorSet(1.f, 0.f, 0.f, 1.f), mat));
	M_Store(&t->up,      XMVector4Transform(XMVectorSet(0.f, 1.f, 0.f, 1.f), mat));
}

static inline void
Xform_Move(struct NeTransform *t, const struct NeVec3 *movement)
{
	M_Store(&t->position, XMVectorAdd(M_Load(&t->position), M_Load(movement)));
	t->dirty = true;
}

static inline void
Xform_Rotate(struct NeTransform *t, float angle, const struct NeVec3 *axis)
{
	M_Store(&t->rotation,
		XMQuaternionMultiply(M_Load(&t->rotation), XMQuaternionRotationAxis(M_Load(axis), XMConvertToRadians(angle))));
	Xform_UpdateOrientation(t);
	t->dirty = true;
}

static inline void
Xform_Scale(struct NeTransform *t, const struct NeVec3 *scale)
{
	M_Store(&t->position, XMVectorMultiply(M_Load(&t->position), M_Load(scale)));
	t->dirty = true;
}

static inline void
Xform_SetPosition(struct NeTransform *t, const struct NeVec3 *pos)
{
	memcpy(&t->position, pos, sizeof(t->position));
	t->dirty = true;
}

static inline void
Xform_SetRotation(struct NeTransform *t, const struct NeQuaternion *rot)
{
	memcpy(&t->rotation, rot, sizeof(t->rotation));
	Xform_UpdateOrientation(t);
	t->dirty = true;
}

static inline void
Xform_SetScale(struct NeTransform *t, const struct NeVec3 *scale)
{
	memcpy(&t->scale, scale, sizeof(t->scale));
	t->scale.x = fmaxf(t->scale.x, .0000001f);
	t->scale.y = fmaxf(t->scale.y, .0000001f);
	t->scale.z = fmaxf(t->scale.z, .0000001f);
	t->dirty = true;
}

static inline void
Xform_LookAt(struct NeTransform *t, struct NeVec3 *target, struct NeVec3 *up)
{
	M_Store(&t->mat, XMMatrixLookAtRH(M_Load(&t->position), M_Load(target), M_Load(up)));
}

static inline void
Xform_Update(struct NeTransform *t)
{
	struct NeScene *s = Scn_GetScene((uint8_t)t->_sceneId);
	const struct NeTransform *parent = (struct NeTransform *)E_ComponentPtrS(s, t->parent);

	if ((t->parent != NE_INVALID_HANDLE && !parent->dirty) && !t->dirty)
		return;

	XMMATRIX mat = XMMatrixMultiply(XMMatrixScaling(t->scale.x, t->scale.y, t->scale.z),
		XMMatrixRotationQuaternion(M_Load(&t->rotation)));
	mat = XMMatrixMultiply(mat, XMMatrixTranslation(t->position.x, t->position.y, t->position.z));

	if (parent)
		mat = XMMatrixMultiply(mat, M_Load(&parent->mat));

	M_Store(&t->mat, mat);

	for (size_t i = 0; i < t->children.count; ++i)
		Xform_Update((struct NeTransform *) E_ComponentPtrS(s, *((NeCompHandle *) Rt_ArrayGet(&t->children, i))));

	t->dirty = false;
}

static inline struct NeVec3 *
Xform_Position(const struct NeTransform *t, struct NeVec3 *pos)
{
	if (t->parent != NE_INVALID_HANDLE) {
		struct NeVec3 tmp;
		const struct NeTransform *parent = (struct NeTransform *)E_ComponentPtr(t->parent);

		Xform_Position(parent, &tmp);
		M_Store(pos, XMVectorAdd(M_Load(&tmp), M_Load(&t->position)));
	} else {
		memcpy(pos, &t->position, sizeof(*pos));
	}

	return pos;
}

static inline struct NeQuaternion *
Xform_Rotation(const struct NeTransform *t, struct NeQuaternion *rot)
{
	if (t->parent != NE_INVALID_HANDLE) {
		struct NeQuaternion tmp;
		const struct NeTransform *parent = (struct NeTransform *)E_ComponentPtr(t->parent);

		Xform_Rotation(parent, &tmp);
		M_Store(rot, XMQuaternionMultiply(M_Load(&tmp), M_Load(&t->rotation)));
	} else {
		memcpy(rot, &t->rotation, sizeof(*rot));
	}

	return rot;
}

static inline void
Xform_RotationAnglesF(const struct NeTransform *t, float *pitch, float *yaw, float *roll)
{
	struct NeQuaternion q;
	Xform_Rotation(t, &q);

	if (pitch)
		*pitch = M_QuatPitch(&q);

	if (yaw)
		*yaw = M_QuatYaw(&q);

	if (roll)
		*roll = M_QuatRoll(&q);
}

static inline struct NeVec3 *
Xform_RotationAngles(const struct NeTransform *t, struct NeVec3 *rot)
{
	struct NeQuaternion q;
	Xform_Rotation(t, &q);

	rot->x = M_QuatPitch(&q);
	rot->y = M_QuatYaw(&q);
	rot->z = M_QuatRoll(&q);

	Xform_RotationAnglesF(t, &rot->x, &rot->y, &rot->z);

	return rot;
}

static inline void
Xform_MoveForward(struct NeTransform *t, float distance)
{
	M_Store(&t->position, XMVectorMultiplyAdd(M_Load(&t->forward), XMVectorReplicate(-distance), M_Load(&t->position)));
	t->dirty = true;
}

static inline void
Xform_MoveBackward(struct NeTransform *t, float distance)
{
	Xform_MoveForward(t, -distance);
}

static inline void
Xform_MoveRight(struct NeTransform *t, float distance)
{
	M_Store(&t->position, XMVectorMultiplyAdd(M_Load(&t->right), XMVectorReplicate(distance), M_Load(&t->position)));
	t->dirty = true;
}

static inline void
Xform_MoveLeft(struct NeTransform *t, float distance)
{
	Xform_MoveRight(t, -distance);
}

static inline void
Xform_MoveUp(struct NeTransform *t, float distance)
{
	M_Store(&t->position, XMVectorMultiplyAdd(M_Load(&t->up), XMVectorReplicate(distance), M_Load(&t->position)));
	t->dirty = true;
}

static inline void
Xform_MoveDown(struct NeTransform *t, float distance)
{
	Xform_MoveUp(t, -distance);
}

#ifdef __cplusplus
}
#endif

#endif /* NE_SCENE_TRANSFORM_H */

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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
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
