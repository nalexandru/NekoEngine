/* NekoEngine
 *
 * plane.h
 * Author: Alexandru Naiman
 *
 * Plane functions
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2020, Alexandru Naiman
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
 * Original copyright:

Copyright (c) 2008, Luke Benstead.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _NE_MATH_PLANE_H_
#define _NE_MATH_PLANE_H_

#include <Math/defs.h>

#define	PT_BEHIND_PLANE			-1
#define	PT_ON_PLANE				 0
#define	PT_INFRONT_OF_PLANE		 1

static inline struct plane *
plane(
	struct plane *p,
	float a,
	float b,
	float c,
	float d)
{
	p->a = a;
	p->b = b;
	p->c = c;
	p->d = d;

	return p;
}

static inline struct plane *
plane_normal_distance(
	struct plane *p,
	const struct vec3 *n,
	const float d)
{
	p->a = n->x;
	p->b = n->y;
	p->c = n->z;
	p->d = d;

	return p;
}

static inline struct plane *
plane_point_normal(
	struct plane *p,
	const struct vec3 *pt,
	const struct vec3 *n)
{
	p->a = n->x;
	p->b = n->y;
	p->c = n->z;
	p->d = -v3_dot(n, pt);

	return p;
}

/*
 * Creates a plane from 3 points. The result is stored in pOut.
 * pOut is returned.
 */
static inline struct plane *
plane_points(
	struct plane *p,
	const struct vec3 *p1,
	const struct vec3 *p2,
	const struct vec3 *p3)
{
	/*
	 * v = (B − A) × (C − A)
	 * n = 1⁄|v| v
	 * Outa = nx
	 * Outb = ny
	 * Outc = nz
	 * Outd = −n⋅A
	 */

	struct vec3 n, v1, v2;

	v3_sub(&v1, p2, p1);
	v3_sub(&v2, p3, p1);
	v3_cross(&n, &v1, &v2);

	v3_norm(&n, &n);

	p->a = n.x;
	p->b = n.y;
	p->c = n.z;
	p->d = v3_dot(v3_scale(&n, &n, -1.f), p1);

	return p;
}

static inline float
plane_dot(
	const struct plane *p,
	const struct vec4 *v)
{
	return (p->a * v->x +
		p->b * v->y +
		p->c * v->z +
		p->d * v->w);
}

static inline float
plane_dot_coord(
	const struct plane *p,
	const struct vec3 *v)
{
	return (p->a * v->x +
		p->b * v->y +
		p->c * v->z + p->d);
}

static inline float
plane_dot_Normal(
	const struct plane *p,
	const struct vec3 *v)
{
	return (p->a * v->x +
		p->b * v->y +
		p->c * v->z);
}

static inline struct vec3 *
plane_intersect_line(
	struct vec3 *pt,
	const struct plane *p,
	const struct vec3 *v1,
	const struct vec3 *v2)
{
	/*
	 * n = (Planea, Planeb, Planec)
	 * d = V − U
	 * Out = U − d⋅(Pd + n⋅U)⁄(d⋅n) [iff d⋅n ≠ 0]
	 */
	struct vec3 d;
	float nt;
	float dt;
	float t;
	struct vec3 n;

	v3_sub(&d, v2, v1);

	n.x = p->a;
	n.y = p->b;
	n.z = p->c;
	v3_norm(&n, &n);

	nt = -(n.x * v1->x + n.y * v1->y + n.z * v1->z + p->d);
	dt = (n.x * d.x + n.y * d.y + n.z * d.z);

	if (fabsf(dt) < FLT_EPSILON)
		return NULL;

	t = nt / dt;

	pt->x = v1->x + d.x * t;
	pt->y = v1->y + d.y * t;
	pt->z = v1->z + d.z * t;

	return pt;
}

static inline struct plane *
plane_norm(
	struct plane *dst,
	const struct plane *src)
{
	struct vec3 n;
	float l = 0;

	if (!src->a && !src->b && !src->c) {
		dst->a = src->a;
		dst->b = src->b;
		dst->c = src->c;
		dst->d = src->d;
		return dst;
	}

	n.x = src->a;
	n.y = src->b;
	n.z = src->c;

	l = 1.0f / v3_len(&n);
	v3_norm(&n, &n);

	dst->a = n.x;
	dst->b = n.y;
	dst->c = n.z;

	dst->d = src->d * l;

	return dst;
}

/*
 * Returns POINT_INFRONT_OF_PLANE if pP is in front of pIn. Returns
 * POINT_BEHIND_PLANE if it is behind. Returns POINT_ON_PLANE otherwise
 */
static inline int
plane_pt_pos(
	const struct plane *pIn,
	const struct vec3 *pP)
{
	/*
	 * This function will determine if a point is on, in front of, or behind
	 * the plane.  First we store the dot product of the plane and the point.
	 */
	float distance = pIn->a * pP->x + pIn->b * pP->y + pIn->c * pP->z + pIn->d;

	/* Simply put if the dot product is greater than 0 then it is in
	 * front of it.  If it is less than 0 then it is behind it.  And if
	 * it is 0 then it is on it.
	 */

	if (distance > FLT_EPSILON)
		return PT_INFRONT_OF_PLANE;

	if (distance < -FLT_EPSILON)
		return PT_BEHIND_PLANE;

	return PT_ON_PLANE;
}

static inline struct plane *
plane_from_mat4(
	struct plane *p,
	const struct mat4 *m,
	int32_t r)
{
	int scale = (r < 0) ? -1 : 1;
	r = abs(r) - 1;

	p->a = m->m[3] + scale * m->m[r];
	p->b = m->m[7] + scale * m->m[r + 4];
	p->c = m->m[11] + scale * m->m[r + 8];
	p->d = m->m[15] + scale * m->m[r + 12];

	return plane_norm(p, p);
}

static inline struct vec3 *
plane_intersect(
	struct vec3 *v,
	const struct plane *p1,
	const struct plane *p2,
	const struct plane *p3)
{
	struct vec3 n1, n2, n3, cross;
	struct vec3 r1, r2, r3;
	float denom = 0.f;

	v3(&n1, p1->a, p1->b, p1->c);
	v3(&n2, p2->a, p2->b, p2->c);
	v3(&n3, p3->a, p3->b, p3->c);

	v3_cross(&cross, &n2, &n3);

	denom = v3_dot(&n1, &cross);

	if (float_equal(denom, 0.f))
		return NULL;

	v3_cross(&r1, &n2, &n3);
	v3_cross(&r2, &n3, &n1);
	v3_cross(&r3, &n1, &n2);

	v3_scale(&r1, &r1, -p1->d);
	v3_scale(&r2, &r2, p2->d);
	v3_scale(&r3, &r3, p3->d);

	v3_sub(v, &r1, &r2);
	v3_sub(v, v, &r3);

	return v3_scale(v, v, 1.f / denom);
}

#endif /* _NE_MATH_PLANE_H_ */

