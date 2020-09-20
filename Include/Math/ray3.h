/* NekoEngine
 *
 * ray3.h
 * Author: Alexandru Naiman
 *
 * 3 component ray
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
 */

#ifndef _NE_MATH_RAY3_H_
#define _NE_MATH_RAY3_H_

#include <Math/defs.h>
#include <Math/vec3.h>
#include <Math/plane.h>

static inline struct ray3 *
r3(
	struct ray3 *ray,
	float px,
	float py,
	float pz,
	float vx,
	float vy,
	float vz)
{
	ray->start.x = px;
	ray->start.y = py;
	ray->start.z = pz;

	ray->dir.x = vx;
	ray->dir.y = vy;
	ray->dir.z = vz;

	return ray;
}

static inline struct ray3 *
r3_point_dir(
	struct ray3 *ray,
	const struct vec3 *point,
	const struct vec3 *direction)
{
	v3_copy(&ray->start, point);
	v3_copy(&ray->dir, direction);

	return ray;
}

static inline bool
r3_intersect_plane(
	struct vec3 *dst,
	const struct ray3 *ray,
	const struct plane *plane)
{
	// t = - (A*org.x + B*org.y + C*org.z + D) / (A*dir.x + B*dir.y + C*dir.z)

	struct vec3 scaled_dir;
	float t;

	float d = (plane->a * ray->dir.x +
			plane->b * ray->dir.y +
			plane->c * ray->dir.z);

	if (d == 0.f)
		return false;

	t = -(plane->a * ray->start.x +
			plane->b * ray->start.y +
			plane->c * ray->start.z + plane->d) / d;

	if (t < 0.f)
		return false;

	v3_scale(&scaled_dir, &ray->dir, t);
	v3_add(dst, &ray->start, &scaled_dir);

	return true;
}

static inline bool
r3_intersect_triangle(
	const struct ray3 *ray,
	const struct vec3 *v0,
	const struct vec3 *v1,
	const struct vec3 *v2,
	struct vec3 *intersection,
	struct vec3 *normal,
	float *distance)
{
	struct vec3 e1, e2, pvec, tvec, qvec, dir;
	float det, inv_det, u, v, t;

	v3_norm(&dir, &ray->dir);

	v3_sub(&e1, v1, v0);
	v3_sub(&e2, v2, v0);

	v3_cross(&pvec, &dir, &e2);
	det = v3_dot(&e1, &pvec);

	// Backfacing, discard.
	if (det < FLT_EPSILON)
		return false;

	if (float_equal(det, 0.f))
		return false;

	inv_det = 1.f / det;

	v3_sub(&tvec, &ray->start, v0);

	u = inv_det * v3_dot(&tvec, &pvec);
	if (u < 0.f || u > 1.f)
		return false;

	v3_cross(&qvec, &tvec, &e1);
	v = inv_det * v3_dot(&dir, &qvec);

	if (v < 0.f || (u + v) > 1.f)
		return false;

	t = inv_det * v3_dot(&e2, &qvec);
	
	if (t > FLT_EPSILON && (t*t) <= v3_len_sq(&ray->dir)) {
		struct vec3 scaled;
		*distance = t; // Distance

		v3_cross(normal, &e1, &e2); // Surface normal of collision
		v3_norm(normal, normal);
		v3_norm(&scaled, &dir);
		v3_scale(&scaled, &scaled, *distance);
		v3_add(intersection, &ray->start, &scaled);

		return true;
	}

	return false;
}

static inline bool
r3_intersect_aabb3(
	const struct ray3 *ray,
	const struct aabb3 *aabb,
	struct vec3 *intersection,
	float *distance)
{
	// http://gamedev.stackexchange.com/a/18459/15125
	struct vec3 rdir, dirfrac, diff;
	float t1, t2, t3, t4, t5, t6, tmin, tmax;

	v3_norm(&rdir, &ray->dir);
	v3(&dirfrac, 1.f / rdir.x, 1.f / rdir.y, 1.f / rdir.z);

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
		v3_scale(&diff, &rdir, tmin);
		v3_add(intersection, &ray->start, &diff);
	}

	return true;
}

#endif /* _NE_MATH_RAY3_H_ */

