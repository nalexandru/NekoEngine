#ifndef _NE_MATH_RAY3_H_
#define _NE_MATH_RAY3_H_

#include <Math/defs.h>
#include <Math/vec3.h>
#include <Math/plane.h>

static inline struct NeRay *
M_Ray(struct NeRay *ray, float px, float py,
	float pz, float vx, float vy, float vz)
{
	ray->start.x = px;
	ray->start.y = py;
	ray->start.z = pz;

	ray->dir.x = vx;
	ray->dir.y = vy;
	ray->dir.z = vz;

	return ray;
}

static inline struct NeRay *
M_RayFromPointDirection(struct NeRay *ray, const struct NeVec3 *point,
	const struct NeVec3 *direction)
{
	M_CopyVec3(&ray->start, point);
	M_CopyVec3(&ray->dir, direction);

	return ray;
}

static inline bool
M_RayIntersectsPlane(struct NeVec3 *dst,
	const struct NeRay *ray, const struct NePlane *p)
{
	// t = - (A*org.x + B*org.y + C*org.z + D) / (A*dir.x + B*dir.y + C*dir.z)

	struct NeVec3 scaled_dir;
	const float d = M_DotVec3(&p->normal, &ray->dir);

	if (d == 0.f)
		return false;

	const float t = -(p->normal.x * ray->start.x +
			p->normal.y * ray->start.y +
			p->normal.z * ray->start.z + p->distance) / d;

	if (t < 0.f)
		return false;

	M_ScaleVec3(&scaled_dir, &ray->dir, t);
	M_AddVec3(dst, &ray->start, &scaled_dir);

	return true;
}

static inline bool
M_RayIntersectsTriangle(const struct NeRay *ray, const struct NeVec3 *v0,
	const struct NeVec3 *v1, const struct NeVec3 *v2,
	struct NeVec3 *intersection, struct NeVec3 *normal, float *distance)
{
	struct NeVec3 e1, e2, pvec, tvec, qvec, dir;
	float det, inv_det, u, v, t;

	M_NormalizeVec3(&dir, &ray->dir);

	M_SubVec3(&e1, v1, v0);
	M_SubVec3(&e2, v2, v0);

	M_CrossVec3(&pvec, &dir, &e2);
	det = M_DotVec3(&e1, &pvec);

	// Backfacing, discard.
	if (det < FLT_EPSILON)
		return false;

	if (M_FloatEqual(det, 0.f))
		return false;

	inv_det = 1.f / det;

	M_SubVec3(&tvec, &ray->start, v0);

	u = inv_det * M_DotVec3(&tvec, &pvec);
	if (u < 0.f || u > 1.f)
		return false;

	M_CrossVec3(&qvec, &tvec, &e1);
	v = inv_det * M_DotVec3(&dir, &qvec);

	if (v < 0.f || (u + v) > 1.f)
		return false;

	t = inv_det * M_DotVec3(&e2, &qvec);
	
	if (t > FLT_EPSILON && (t*t) <= M_Vec3LengthSquared(&ray->dir)) {
		struct NeVec3 scaled;
		*distance = t; // Distance

		M_CrossVec3(normal, &e1, &e2); // Surface normal of collision
		M_NormalizeVec3(normal, normal);
		M_NormalizeVec3(&scaled, &dir);
		M_ScaleVec3(&scaled, &scaled, *distance);
		M_AddVec3(intersection, &ray->start, &scaled);

		return true;
	}

	return false;
}

static inline bool
M_RayIntersectsAABB(const struct NeRay *ray, const struct NeAABB *aabb,
	struct NeVec3 *intersection, float *distance)
{
	// http://gamedev.stackexchange.com/a/18459/15125
	struct NeVec3 rdir, dirfrac, diff;
	float t1, t2, t3, t4, t5, t6, tmin, tmax;

	M_NormalizeVec3(&rdir, &ray->dir);
	M_Vec3(&dirfrac, 1.f / rdir.x, 1.f / rdir.y, 1.f / rdir.z);

	t1 = (aabb->min.x - ray->start.x) * dirfrac.x;
	t2 = (aabb->max.x - ray->start.x) * dirfrac.x;
	t3 = (aabb->min.y - ray->start.y) * dirfrac.y;
	t4 = (aabb->max.y - ray->start.y) * dirfrac.y;
	t5 = (aabb->min.z - ray->start.z) * dirfrac.z;
	t6 = (aabb->max.z - ray->start.z) * dirfrac.z;

	tmin = fmaxf(fmaxf(fminf(t1, t2), fminf(t3, t4)), fminf(t5, t6));
	tmax = fminf(fminf(fmaxf(t1, t2), fmaxf(t3, t4)), fmaxf(t5, t6));

	// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behind us
	if (tmax < 0)
		return false;

	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax)
		return false;

	if (distance)
		*distance = tmin;

	if (intersection) {
		M_ScaleVec3(&diff, &rdir, tmin);
		M_AddVec3(intersection, &ray->start, &diff);
	}

	return true;
}

#endif /* _NE_MATH_RAY3_H_ */

/* NekoEngine
 *
 * ray.h
 * Author: Alexandru Naiman
 *
 * 3 component ray
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
