/* NekoEngine
 *
 * bounds.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Bounds
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

#include <scene/bounds.h>

static inline void
_transform_sphere(
	ne_bounding_sphere *s,
	kmMat4 *m)
{
	kmVec3 edge;
	kmVec3 tmp;

	kmVec3Add(&edge, &s->center,
		kmVec3Fill(&tmp, 1.f, 0.f, 0.f));

	kmVec3Mul(&edge, &edge,
		kmVec3Fill(&tmp, s->radius, s->radius, s->radius));

	kmVec3MultiplyMat4(&s->center, &s->center, m);
	kmVec3MultiplyMat4(&edge, &edge, m);
	s->radius = kmVec3Length(kmVec3Subtract(&tmp, &s->center, &edge));
}

static inline void
_transform_box(
	kmAABB3 *b,
	kmMat4 *m)
{
	//
}

static inline double
_box_sq_calc(
	double p,
	double min,
	double max)
{
	double ret = 0.0;

	if (p < min)
		ret = min - p;
	else if (p > max)
		ret = p - max;

	return ret * ret;
}

static inline double
_box_sq_dist(
	kmAABB3 *b,
	kmVec3 *p)
{
	return _box_sq_calc(p->x, b->min.x, b->max.x) +
		_box_sq_calc(p->y, b->min.y, b->max.y) +
		_box_sq_calc(p->z, b->min.z, b->max.z);
}

void
bounds_set_box(
	ne_bounds *b,
	kmVec3 *center,
	float width,
	float height,
	float depth) 
{
	kmAABB3Initialize(&b->box, center, width, height, depth);
	b->have_box = true;
}

void
bounds_set_sphere(
	ne_bounds *b,
	kmVec3 *center,
	float radius)
{
	b->sphere.center.x = center->x;
	b->sphere.center.y = center->y;
	b->sphere.center.z = center->z;
	b->sphere.radius = radius;
	b->have_sphere = true;
}

bool
bounds_contains(
	ne_bounds *b,
	ne_bounds *o)
{
	kmVec3 tmp;

	if (b->have_box &&o->have_box)
		return kmAABB3ContainsAABB(&b->box, &o->box) == 2;
	else if (b->have_box &&o->have_box)
		return _box_sq_dist(&b->box, &o->sphere.center) <=
			(o->sphere.radius * o->sphere.radius);
	else if (b->have_sphere &&o->have_box)
		return _box_sq_dist(&o->box, &b->sphere.center) <=
			(b->sphere.radius * b->sphere.radius);
	else if (b->have_sphere &&o->have_sphere)
		return kmVec3Length(kmVec3Subtract(&tmp, &b->sphere.center,
			&o->sphere.center))	+ o->sphere.radius < b->sphere.radius; 
	else
		return false;
}

bool
bounds_intersects(
	ne_bounds *b,
	ne_bounds *o)
{
	kmVec3 tmp;

	if (b->have_box &&o->have_box)
		return kmAABB3IntersectsAABB(&b->box, &o->box) == 2;
	else if (b->have_box &&o->have_box)
		return _box_sq_dist(&b->box, &o->sphere.center) <=
			(o->sphere.radius * o->sphere.radius);
	else if (b->have_sphere &&o->have_box)
		return _box_sq_dist(&o->box, &b->sphere.center) <=
			(b->sphere.radius * b->sphere.radius);
	else if (b->have_sphere &&o->have_sphere)
		return kmVec3Length(kmVec3Subtract(&tmp, &b->sphere.center,
			&o->sphere.center)) < (b->sphere.radius + o->sphere.radius);
	else
		return false;
}

void
bounds_transform(
	ne_bounds *b,
	kmMat4 *mat,
	ne_bounds *dst)
{
	ne_bounding_sphere *sphere = NULL;
	kmAABB3 *box = NULL;

	if (!dst) {
		sphere = &b->sphere;
		box = &b->box;
	} else {
		sphere = &dst->sphere;
		box = &dst->box;

		dst->have_sphere = b->have_sphere;
		dst->have_box = b->have_box;
	}

	if (b->have_sphere)
		_transform_sphere(sphere, mat);

	if (b->have_box)
		_transform_box(box, mat);
}

