#ifndef _NE_MATH_PLANE_H_
#define _NE_MATH_PLANE_H_

#include <Math/defs.h>

#define	PT_BEHIND_PLANE			-1
#define	PT_ON_PLANE				 0
#define	PT_INFRONT_OF_PLANE		 1

static inline struct NePlane *
M_Plane(struct NePlane *p, float a, float b, float c, float d)
{
	M_Vec3(&p->normal, a, b, c);
	M_NormalizeVec3(&p->normal, &p->normal);
	p->distance = d;

	return p;
}

static inline struct NePlane *
M_PlaneFromNormalDistance(struct NePlane *p, const struct NeVec3 *n, const float d)
{
	M_NormalizeVec3(&p->normal, n);
	p->distance = d;

	return p;
}

static inline struct NePlane *
M_PlaneFromNormalPoint(struct NePlane *p, const struct NeVec3 *pt, const struct NeVec3 *n)
{
	M_NormalizeVec3(&p->normal, n);
	p->distance = M_DotVec3(n, pt);

	return p;
}

/*
 * Creates a NePlane from 3 points. The result is stored in pOut.
 * pOut is returned.
 */
static inline struct NePlane *
M_PlaneFromPoints(struct NePlane *p, const struct NeVec3 *p1,
	const struct NeVec3 *p2, const struct NeVec3 *p3)
{
	/*
	 * v = (B − A) × (C − A)
	 * n = 1⁄|v| v
	 * Outa = nx
	 * Outb = ny
	 * Outc = nz
	 * Outd = −n⋅A
	 */

	struct NeVec3 n, v1, v2;

	M_SubVec3(&v1, p2, p1);
	M_SubVec3(&v2, p3, p1);
	M_CrossVec3(&n, &v1, &v2);

	M_NormalizeVec3(&p->normal, &n);
	p->distance = M_DotVec3(M_ScaleVec3(&n, &n, -1.f), p1);

	return p;
}

static inline float
M_PlaneDot(const struct NePlane *p, const struct NeVec4 *v)
{
	struct NeVec4 tmp;
	return M_DotVec4(M_Vec4(&tmp, p->normal.x, p->normal.y, p->normal.z, p->distance), v);

}

static inline float
M_PlaneDotCoord(const struct NePlane *p, const struct NeVec3 *v)
{
	return (p->normal.x * v->x +
		p->normal.y * v->y +
		p->normal.z * v->z + p->distance);
}

static inline float
M_PlaneDotNormal(const struct NePlane *p, const struct NeVec3 *v)
{
	return (p->normal.x * v->x +
		p->normal.y * v->y +
		p->normal.z * v->z);
}

static inline struct NeVec3 *
M_PlaneIntersectsLine(struct NeVec3 *pt, const struct NePlane *p,
	const struct NeVec3 *v1, const struct NeVec3 *v2)
{
	/*
	 * n = (Planea, Planeb, Planec)
	 * d = V − U
	 * Out = U − d⋅(Pd + n⋅U)⁄(d⋅n) [iff d⋅n ≠ 0]
	 */
	struct NeVec3 d;
	float nt;
	float dt;
	float t;
	struct NeVec3 n;

	M_SubVec3(&d, v2, v1);
	M_CopyVec3(&n, &p->normal);

	nt = -(n.x * v1->x + n.y * v1->y + n.z * v1->z + p->distance);
	dt = (n.x * d.x + n.y * d.y + n.z * d.z);

	if (fabsf(dt) < FLT_EPSILON)
		return NULL;

	t = nt / dt;

	pt->x = v1->x + d.x * t;
	pt->y = v1->y + d.y * t;
	pt->z = v1->z + d.z * t;

	return pt;
}

static inline struct NePlane *
M_NormalizePlane(struct NePlane *dst, const struct NePlane *src)
{
	const float m = sqrtf(
		src->normal.x * src->normal.x +
		src->normal.y * src->normal.y +
		src->normal.z * src->normal.z
	);

	dst->normal.x = src->normal.x / m;
	dst->normal.y = src->normal.y / m;
	dst->normal.z = src->normal.z / m;
	dst->distance = dst->distance / m;

	return dst;
}

/*
 * Returns POINT_INFRONT_OF_PLANE if pt is in front of p. Returns
 * POINT_BEHIND_PLANE if it is behind. Returns POINT_ON_PLANE otherwise
 */
static inline int
M_PlanePointPosition(const struct NePlane *p, const struct NeVec3 *pt)
{
	/*
	 * This function will determine if a point is on, in front of, or behind
	 * the NePlane.  First we store the dot product of the NePlane and the point.
	 */
	const float distance = M_DotVec3(&p->normal, pt);

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

static inline struct NePlane *
M_PlaneFromMatrix(struct NePlane *p, const struct NeMatrix *m, int32_t r)
{
	const int scale = (r < 0) ? -1 : 1;
	r = abs(r) - 1;

	p->normal.x = m->m[3] + scale * m->m[r];
	p->normal.y = m->m[7] + scale * m->m[r + 4];
	p->normal.z = m->m[11] + scale * m->m[r + 8];
	p->distance = m->m[15] + scale * m->m[r + 12];

	return M_NormalizePlane(p, p);
}

static inline struct NeVec3 *
M_PlaneIntersection(struct NeVec3 *v, const struct NePlane *p1,
	const struct NePlane *p2, const struct NePlane *p3)
{
	struct NeVec3 cross;
	struct NeVec3 r1, r2, r3;

	M_CrossVec3(&cross, &p2->normal, &p3->normal);

	const float denom = M_DotVec3(&p1->normal, &cross);

	if (M_FloatEqual(denom, 0.f))
		return NULL;

	M_CrossVec3(&r1, &p2->normal, &p3->normal);
	M_CrossVec3(&r2, &p3->normal, &p1->normal);
	M_CrossVec3(&r3, &p1->normal, &p2->normal);

	M_ScaleVec3(&r1, &r1, -p1->distance);
	M_ScaleVec3(&r2, &r2,  p2->distance);
	M_ScaleVec3(&r3, &r3,  p3->distance);

	M_SubVec3(v, &r1, &r2);
	M_SubVec3(v, v, &r3);

	return M_ScaleVec3(v, v, 1.f / denom);
}

static inline float
M_PlaneDistanceToPoint(const struct NePlane *p, const struct NeVec3 *pt)
{
	return M_DotVec3(&p->normal, pt) + p->distance;
}

#endif /* _NE_MATH_PLANE_H_ */

/* NekoEngine
 *
 * plane.h
 * Author: Alexandru Naiman
 *
 * Plane functions
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
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
