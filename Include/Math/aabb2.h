/* NekoEngine
 *
 * aabb2.h
 * Author: Alexandru Naiman
 *
 * 2D AABB functions
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

#ifndef _NE_MATH_AABB2_H_
#define _NE_MATH_AABB2_H_

#include <Math/defs.h>
#include <Math/vec2.h>

/*
 * Initializes the AABB around a central point. If center is NULL
 * then the origin is used. Returns pBox.
 */
static inline struct aabb2 *
aabb2(
	struct aabb2 *aabb,
	const struct vec2 *center,
	const float width,
	const float height)
{
	struct vec2 origin;
	struct vec2 *point;

	point = center ? (struct vec2 *) center : &origin;
	v2_fill(&origin, 0.f);

	aabb->min.x = point->x - (width / 2);
	aabb->min.y = point->y - (height / 2);

	aabb->max.x = point->x + (width / 2);
	aabb->max.y = point->y + (height / 2);

	return aabb;
}

/*
 * Assigns pIn to pOut, returns pOut.
 */
static inline struct aabb2 *
aabb2_copy(
	struct aabb2 *dst,
	const struct aabb2 *src)
{
	memcpy(dst, src, sizeof(*dst));

	return dst;
}

/*
 * Makes sure that min corresponds to the minimum values and max to
 * the maximum
 */
static inline struct aabb2 *
aabb2_sanitize(
	struct aabb2 *dst,
	const struct aabb2 *src)
{
	if (src->min.x <= src->max.x) {
		dst->min.x = src->min.x;
		dst->max.x = src->max.x;
	} else {
		dst->min.x = src->max.x;
		dst->max.x = src->min.x;
	}

	if (src->min.y <= src->max.y) {
		dst->min.y = src->min.y;
		dst->max.y = src->max.y;
	} else {
		dst->min.y = src->max.y;
		dst->max.y = src->min.y;
	}

	return dst;
}

/*
 * Returns true if point is in the specified AABB, returns false
 * otherwise.
 */
static inline int
aabb2_contains_pt(
	const struct aabb2 *aabb,
	const struct vec2 *pt)
{
	return (pt->x >= aabb->min.x && pt->x <= aabb->max.x &&
		pt->y >= aabb->min.y && pt->y <= aabb->max.y);
}

static inline uint8_t
aabb2_contains_aabb2(
	const struct aabb2 *aabb,
	const struct aabb2 *chk)
{
	struct vec2 corners[4];
	int contains;

	v2(&corners[0], chk->min.x, chk->min.y);
	v2(&corners[1], chk->max.x, chk->min.y);
	v2(&corners[2], chk->max.x, chk->max.y);
	v2(&corners[3], chk->min.x, chk->max.y);

	/*
	 * since true equals 1 , we can count the number of contained points
	 * by actually adding the results:
	 */
	contains = aabb2_contains_pt(aabb, &corners[0]) +
		aabb2_contains_pt(aabb, &corners[1]) +
		aabb2_contains_pt(aabb, &corners[2]) +
		aabb2_contains_pt(aabb, &corners[3]);

	if (contains == 0)
		return M_CONTAINS_NONE;
	else if (contains < 4)
		return M_CONTAINS_PARTIAL;
	else
		return M_CONTAINS_ALL;
}

/*
 * Scales pIn by s, stores the resulting AABB in pOut. Returns pOut.
 * It modifies both points, so position of the box will be
 * changed. Use aabb2_scale_pivot to specify the origin of the
 * scale.
 */
static inline struct aabb2 *
aabb2_translate(
	struct aabb2 *dst,
	const struct aabb2 *src,
	const struct vec2 *translation)
{
	v2_add(&(dst->min), &(src->min), translation);
	v2_add(&(dst->max), &(src->max), translation);

	return dst;
}

static inline struct aabb2 *
aabb2_scale(
	struct aabb2 *dst,
	const struct aabb2 *src,
	float s)
{
	v2_scale(&(dst->max), &(src->max), s);
	v2_scale(&(dst->min), &(src->min), s);

	return dst;
}

/*
 * Scales pIn by s, using pivot as the origin for the scale.
 */
static inline struct aabb2 *
aabb2_scale_pivot(
	struct aabb2 *dst,
	const struct aabb2 *src,
	const struct vec2 *pivot,
	float s)
{
	struct vec2 translate;

	translate.x = -pivot->x;
	translate.y = -pivot->y;

	aabb2_translate(dst, src, &translate);
	aabb2_scale(dst, src, s);
	aabb2_translate(dst, src, pivot);

	return dst;
}

static inline float
aabb2_size_x(const struct aabb2 *aabb)
{
	return fabsf(aabb->max.x - aabb->min.x);
}

static inline float
aabb2_size_y(const struct aabb2 *aabb)
{
	return fabsf(aabb->max.y - aabb->min.y);
}

static inline struct vec2 *
aabb2_center(
	const struct aabb2 *aabb,
	struct vec2 *dst)
{
	v2_add(dst, &aabb->min, &aabb->max);
	v2_scale(dst, dst, 0.5);

	return dst;
}

/*
 * @brief aabb2_expand
 * @param pOut - The resulting AABB
 * @param pIn - The original AABB
 * @param other - Another AABB that you want pIn expanded to contain
 * @return
 */
static inline struct aabb2 *
aabb2_expand(
	struct aabb2 *dst,
	const struct aabb2 *src,
	const struct aabb2 *other)
{
	dst->min.x = (src->min.x < other->min.x) ? src->min.x : other->min.x;
	dst->max.x = (src->max.x > other->max.x) ? src->max.x : other->max.x;
	dst->min.y = (src->min.y < other->min.y) ? src->min.y : other->min.y;
	dst->max.y = (src->max.y > other->max.y) ? src->max.y : other->max.y;

	return dst;
}

#endif /* _NE_MATH_AABB2_H_ */

