/* NekoEngine
 *
 * aabb3.h
 * Author: Alexandru Naiman
 *
 * 3D AABB functions
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

#ifndef _NE_MATH_AABB3_H_
#define _NE_MATH_AABB3_H_

#include <Math/defs.h>

/*
 * Initializes the AABB around a central point. If centre is NULL
 * then the origin is used. Returns pBox.
 */
static inline struct aabb3 *
aabb3(
	struct aabb3 *dst,
	const struct vec3 *center,
	const float width,
	const float height,
	const float depth)
{
	struct vec3 origin;
	struct vec3 *point;

	point = center ? (struct vec3 *) center : &origin;
	v3_zero(&origin);

	dst->min.x = point->x - (width / 2);
	dst->min.y = point->y - (height / 2);
	dst->min.z = point->z - (depth / 2);

	dst->max.x = point->x + (width / 2);
	dst->max.y = point->y + (height / 2);
	dst->max.z = point->z + (depth / 2);

	return dst;
}

/*
 * Returns true if point is in the specified AABB, returns false
 * otherwise.
 */
static inline int
aabb3_contains_pt(
	const struct aabb3 *aabb,
	const struct vec3 *pt)
{
	return (pt->x >= aabb->min.x && pt->x <= aabb->max.x &&
		pt->y >= aabb->min.y && pt->y <= aabb->max.y &&
		pt->z >= aabb->min.z && pt->z <= aabb->max.z);
}

static inline uint8_t
aabb3_contains_aabb3(
	const struct aabb3 *aabb,
	const struct aabb3 *chk)
{
	uint8_t i;
	struct vec3 corners[8];
	uint8_t result = M_CONTAINS_ALL;
	bool found = false;

	v3(&corners[0], chk->min.x, chk->min.y, chk->min.z);
	v3(&corners[1], chk->max.x, chk->min.y, chk->min.z);
	v3(&corners[2], chk->max.x, chk->max.y, chk->min.z);
	v3(&corners[3], chk->min.x, chk->max.y, chk->min.z);
	v3(&corners[4], chk->min.x, chk->min.y, chk->max.z);
	v3(&corners[5], chk->max.x, chk->min.y, chk->max.z);
	v3(&corners[6], chk->max.x, chk->max.y, chk->max.z);
	v3(&corners[7], chk->min.x, chk->max.y, chk->max.z);

	for (i = 0; i < 8; ++i) {
		if (!aabb3_contains_pt(aabb, &corners[i])) {
			result = M_CONTAINS_PARTIAL;

			/* If we previously found a corner that was within the container
			 * We know that partial is the final result
			 */
			if (found)
				return result;
		} else {
			found = true;
		}
	}

	if (!found)
		result = M_CONTAINS_NONE;

	return result;
}

/*
 * Assigns pIn to pOut, returns pOut.
 */
static inline struct aabb3 *
aabb3_copy(
	struct aabb3 *dst,
	const struct aabb3 *src)
{
	v3_copy(&dst->min, &src->min);
	v3_copy(&dst->max, &src->max);

	return dst;
}

/*
 * Scales pIn by s, stores the resulting AABB in pOut. Returns pOut
 */
static inline struct aabb3 *
aabb3_scale(
	struct aabb3 *dst,
	const struct aabb3 *src,
	float s)
{
	v3_scale(&(dst->max), &(src->max), s);
	v3_scale(&(dst->min), &(src->min), s);

	return dst;
}

static inline bool
aabb3_intersects_triangle(
	struct aabb3 *box,
	const struct vec3 *p1,
	const struct vec3 *p2,
	const struct vec3 *p3)
{
	(void)box; (void)p1; (void)p2; (void)p3;
	// FIXME: Not implemented
	return false;
}

static inline bool
aabb3_intersects_aabb3(
	const struct aabb3 *box,
	const struct aabb3 *other)
{
	// Probably should store center point and radius for things like this

	float acx = (box->min.x + box->max.x) * .5f;
	float acy = (box->min.y + box->max.y) * .5f;
	float acz = (box->min.z + box->max.z) * .5f;

	float bcx = (other->min.x + other->max.x) * .5f;
	float bcy = (other->min.y + other->max.y) * .5f;
	float bcz = (other->min.z + other->max.z) * .5f;

	float arx = (box->max.x - box->min.x) * .5f;
	float ary = (box->max.y - box->min.y) * .5f;
	float arz = (box->max.z - box->min.z) * .5f;

	float brx = (other->max.x - other->min.x) * .5f;
	float bry = (other->max.y - other->min.y) * .5f;
	float brz = (other->max.z - other->min.z) * .5f;

	bool x = fabsf(acx - bcx) <= (arx + brx);
	bool y = fabsf(acy - bcy) <= (ary + bry);
	bool z = fabsf(acz - bcz) <= (arz + brz);

	return x && y && z;
}

static inline float
aabb3_size_x(const struct aabb3 *aabb)
{
	return fabsf(aabb->max.x - aabb->min.x);
}

static inline float
aabb3_size_y(const struct aabb3 *aabb)
{
	return fabsf(aabb->max.y - aabb->min.y);
}

static inline float
aabb3_size_z(const struct aabb3 *aabb)
{
	return fabsf(aabb->max.z - aabb->min.z);
}

static inline struct vec3 *
aabb3_center(
	const struct aabb3 *aabb,
	struct vec3 *dst)
{
	v3_add(dst, &aabb->min, &aabb->max);
	v3_scale(dst, dst, 0.5);

	return dst;
}

/*
 * @brief aabb3_expand
 * @param pOut - The resulting AABB
 * @param pIn - The original AABB
 * @param other - Another AABB that you want pIn expanded to contain
 * @return
 */
static inline struct aabb3 *
aabb3_expand(
	struct aabb3 *dst,
	const struct aabb3 *src,
	const struct aabb3 *other)
{
	dst->min.x = (src->min.x < other->min.x) ? src->min.x : other->min.x;
	dst->max.x = (src->max.x > other->max.x) ? src->max.x : other->max.x;
	dst->min.y = (src->min.y < other->min.y) ? src->min.y : other->min.y;
	dst->max.y = (src->max.y > other->max.y) ? src->max.y : other->max.y;
	dst->min.z = (src->min.z < other->min.z) ? src->min.z : other->min.z;
	dst->max.z = (src->max.z > other->max.z) ? src->max.z : other->max.z;

	return dst;
}

#endif /* _NE_MATH_AABB3_H_ */

