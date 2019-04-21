/* NekoEngine
 *
 * transform.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Transform
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
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
 */

#ifndef _NE_SCENE_TRANSFORM_H_
#define _NE_SCENE_TRANSFORM_H_

#include <system/compat.h>

#include <engine/math.h>
#include <ecs/component.h>

typedef struct ne_transform
{
	kmMat4 mat;
	bool dirty;
	kmVec3 pos;
	kmVec3 scale;
	kmQuaternion rot;
	struct ne_transform *parent;
} ne_transform;

static INLINE void
transform_move(ne_transform *t,
	kmVec3 *movement)
{
	kmVec3Add(&t->pos, &t->pos, movement);
	t->dirty = true;
}

static INLINE void
transform_set_pos(ne_transform *t,
	const kmVec3 *pos)
{
	kmVec3Assign(&t->pos, pos);
	t->dirty = true;
}

static INLINE void
transform_rotate(ne_transform *t,
	float angle,
	const kmVec3 *axis)
{
	kmQuaternionRotationAxisAngle(&t->rot, axis, angle);
	t->dirty = true;
}

static INLINE void
transform_scale(ne_transform *t,
	const kmVec3 *scale)
{
	kmVec3Mul(&t->scale, &t->scale, scale);
	t->dirty = true;
}

static INLINE void
transform_set_scale(ne_transform *t,
	const kmVec3 *scale)
{
	kmVec3Assign(&t->scale, scale);
	t->dirty = true;
}

static INLINE const kmMat4 *
transform_model(ne_transform *t)
{
	if (t->parent && !t->parent->dirty && !t->dirty)
		return &t->mat;

	t->dirty = false;

	kmMat4 pos;
	kmMat4 rot;
	kmMat4 scale;

	kmMat4Translation(&pos, t->pos.x, t->pos.y, t->pos.z);
	kmMat4RotationQuaternion(&rot, &t->rot);
	kmMat4Scaling(&scale, t->scale.x, t->scale.y, t->scale.z);

	kmMat4Multiply(&t->mat, &pos, &rot);
	return kmMat4Multiply(&t->mat, &t->mat, &scale);
}

static INLINE void
transform_update(ne_transform *t)
{
	if (t->parent && !t->parent->dirty && !t->dirty)
		return;

	t->dirty = false;
}

struct ne_transform_comp
{
	NE_COMPONENT;

	kmVec3 pos;
	ne_transform transform;
};

#define NE_TRANSFORM_COMP_TYPE	"ne_transform_comp"

#ifdef _NE_ENGINE_INTERNAL_

void		*load_mesh(const char *path);

//ne_status	 init_drawable_mesh_comp(void *, const void *);
//void		 release_drawable_mesh_comp(void *);

NE_REGISTER_COMPONENT(NE_TRANSFORM_COMP_TYPE, struct ne_transform_comp, 0, 0)

#endif /* _NE_ENGINE_INTERNAL_ */

#endif /* _NE_SCENE_TRANSFORM_H_ */
