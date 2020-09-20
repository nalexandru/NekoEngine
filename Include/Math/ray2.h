/* NekoEngine
 *
 * ray2.h
 * Author: Alexandru Naiman
 *
 * 2 component ray functions
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

Copyright (c) 2011, Luke Benstead.
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

#ifndef _NE_MATH_RAY2_H_
#define _NE_MATH_RAY2_H_

#include <Math/defs.h>
#include <Math/vec2.h>

static inline void
__r2_calculate_line_normal(
	struct vec2 p1,
	struct vec2 p2,
	struct vec2 other_point,
	struct vec2 *normal_out)
{
	/*
	 * A = (3,4)
	 * B = (2,1)
	 * C = (1,3)
	 *
	 * AB = (2,1) - (3,4) = (-1,-3)
	 * AC = (1,3) - (3,4) = (-2,-1)
	 * N = n(AB) = (-3,1)
	 * D = dot(N,AC) = 6 + -1 = 5
	 *
	 * since D > 0:
	 * N = -N = (3,-1)
	 */

	struct vec2 edge, other_edge;
	float d;
	struct vec2 n;

	v2_sub(&edge, &p2, &p1);
	v2_sub(&other_edge, &other_point, &p1);
	v2_norm(&edge, &edge);
	v2_norm(&other_edge, &other_edge);

	n.x = edge.y;
	n.y = -edge.x;

	d = v2_dot(&n, &other_edge);
	if(d > 0.0f) {
		n.x = -n.x;
		n.y = -n.y;
	}

	normal_out->x = n.x;
	normal_out->y = n.y;

	v2_norm(normal_out, normal_out);
}

static inline void
r2(
	struct ray2 *ray,
	float px,
	float py,
	float vx,
	float vy)
{
	ray->start.x = px;
	ray->start.y = py;
	ray->dir.x = vx;
	ray->dir.y = vy;
}

static inline void
r2_endpoints(
	struct ray2 *ray,
	const struct vec2 *start,
	const struct vec2 *end)
{
	ray->start.x = start->x;
	ray->start.y = start->y;
	ray->dir.x = end->x - start->x;
	ray->dir.y = end->y - start->y;
}

/*
 * Lines are defined by a pt and a vector. It outputs the vector
 *multiply factor that gives the intersection point
 */
static inline bool
r2_line_intersection(
	const struct vec2 *pt_a,
	const struct vec2 *vec_a,
	const struct vec2 *pt_b,
	const struct vec2 *vec_b,
	float *out_ta,
	float *out_tb,
	struct vec2 *intersection)
{
	float x1 = pt_a->x;
	float y1 = pt_a->y;
	float x2 = x1 + vec_a->x;
	float y2 = y1 + vec_a->y;
	float x3 = pt_b->x;
	float y3 = pt_b->y;
	float x4 = x3 + vec_b->x;
	float y4 = y3 + vec_b->y;

	float ua;
	float ub;

	float x;
	float y;

	float denom = (y4 -y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);

	// If denom is zero, the lines are parallel
	if(denom > -FLT_EPSILON && denom < FLT_EPSILON)
		return false;

	ua = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / denom;
	ub = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / denom;

	x = x1 + ua * (x2 - x1);
	y = y1 + ua * (y2 - y1);

	if (out_ta)
		*out_ta = ua;

	if (out_tb)
		*out_tb = ub;

	if (intersection) {
		intersection->x = x;
		intersection->y = y;
	}

	return true;
}

static inline bool
r2_segment_intersection(
	const struct ray2 *seg_a,
	const struct ray2 *seg_b,
	struct vec2 *intersection)
{
	float ua;
	float ub;
	struct vec2 pt;

	if (r2_line_intersection(&(seg_a->start), &(seg_a->dir),
					&(seg_b->start), &(seg_b->start),
					&ua, &ub, &pt) &&
					(0.0 <= ua) &&
					(ua <= 1.0) &&
					(0.0 <= ub) &&
					(ub <= 1.0)) {
		intersection->x = pt.x;
		intersection->y = pt.y;
		return true;
	}

	return false;
}

static inline bool
r2_line_segment_intersection(
	const struct ray2 *ray,
	const struct vec2 *p1,
	const struct vec2 *p2,
	struct vec2 *intersection)
{
	float ua;
	float ub;
	struct vec2 pt;
	struct ray2 other;

	r2_endpoints(&other, p1, p2);

	if (r2_line_intersection(&(ray->start), &(ray->dir),
					&(other.start),
					&(other.dir),
					&ua, &ub, &pt) &&
					(0.0 <= ua) &&
					(0.0 <= ub) &&
					(ub <= 1.0)) {
		intersection->x = pt.x;
		intersection->y = pt.y;
		return true;
	}

	return false;
}

static inline bool
r2_intersect_triangle(
	const struct ray2 *ray,
	const struct vec2 *p1,
	const struct vec2 *p2,
	const struct vec2 *p3,
	struct vec2 *intersection,
	struct vec2 *normal_out,
	float *distance_out)
{
	struct vec2 intersect;
	struct vec2 final_intersect;
	struct vec2 normal;
	float distance = 10000.f;
	bool intersected = false;

	if (r2_line_segment_intersection(ray, p1, p2, &intersect)) {
		struct vec2 tmp;
		float this_distance = v2_len(v2_sub(&tmp, &intersect, &ray->start));
		struct vec2 this_normal;
		__r2_calculate_line_normal(*p1, *p2, *p3, &this_normal);

		if (this_distance < distance &&
				v2_dot(&this_normal, &ray->dir) < 0.0f) {
			final_intersect.x = intersect.x;
			final_intersect.y = intersect.y;
			distance = this_distance;
			v2_copy(&normal, &this_normal);
			intersected = true;
		}
	}

	if (r2_line_segment_intersection(ray, p2, p3, &intersect)) {
		struct vec2 tmp;
		float this_distance =
			v2_len(v2_sub(&tmp,
						&intersect, &ray->start));

		struct vec2 this_normal;
		__r2_calculate_line_normal(*p2, *p3, *p1, &this_normal);

		if (this_distance < distance &&
				v2_dot(&this_normal, &ray->dir) < 0.0f) {
			final_intersect.x = intersect.x;
			final_intersect.y = intersect.y;
			distance = this_distance;
			v2_copy(&normal, &this_normal);
			intersected = true;
		}
	}

	if (r2_line_segment_intersection(ray, p3, p1, &intersect)) {
		struct vec2 tmp;
		float this_distance =
			v2_len(v2_sub(&tmp,
						&intersect, &ray->start));

		struct vec2 this_normal;
		__r2_calculate_line_normal(*p3, *p1, *p2, &this_normal);

		if (this_distance < distance &&
				v2_dot(&this_normal, &ray->dir) < 0.0f) {
			final_intersect.x = intersect.x;
			final_intersect.y = intersect.y;
			distance = this_distance;
			v2_copy(&normal, &this_normal);
			intersected = true;
		}
	}

	if (intersected) {
		intersection->x = final_intersect.x;
		intersection->y = final_intersect.y;

		if(normal_out) {
			normal_out->x = normal.x;
			normal_out->y = normal.y;
		}

		if (distance)
			*distance_out = distance;
	}

	return intersected;
}

static inline bool
r2_intersect_box(
	const struct ray2 *ray,
	const struct vec2 *p1,
	const struct vec2 *p2,
	const struct vec2 *p3,
	const struct vec2 *p4,
	struct vec2 *intersection,
	struct vec2 *normal_out)
{
	bool intersected = false;
	struct vec2 intersect, final_intersect, normal;
	float distance = 10000.0f;

	const struct vec2 *points[4];
	unsigned int i = 0;
	points[0] = p1;
	points[1] = p2;
	points[2] = p3;
	points[3] = p4;

	for(; i < 4; ++i) {
		const struct vec2 *this_point = points[i];
		const struct vec2 *next_point = (i == 3) ? points[0] : points[i+1];
		const struct vec2 *other_point = (i == 3 || i == 0) ? points[1] : points[0];

		if (r2_line_segment_intersection(ray, this_point,
						next_point, &intersect)) {
			struct vec2 tmp;
			float this_distance =
				v2_len(v2_sub(&tmp, &intersect, &ray->start));

			struct vec2 this_normal;

			__r2_calculate_line_normal(*this_point, *next_point, *other_point, &this_normal);

			if (this_distance < distance &&
					v2_dot(&this_normal, &ray->dir) < 0.0f) {
				v2_copy(&final_intersect, &intersect);
				distance = this_distance;
				intersected = true;
				v2_copy(&normal, &this_normal);
			}
		}
	}

	if (intersected) {
		intersection->x = final_intersect.x;
		intersection->y = final_intersect.y;

		if (normal_out) {
			normal_out->x = normal.x;
			normal_out->y = normal.y;
		}
	}

	return intersected;
}

static inline bool
r2_intersect_circle(
	const struct ray2 *ray,
	const struct vec2 center,
	const float radius,
	struct vec2 *intersection)
{
	(void)ray; (void)center; (void)radius; (void)intersection;
	// FIXME: Not implemented
	return true;
}

#endif /* _NE_MATH_RAY2_H_ */

