/* NekoEngine
 *
 * vec2.h
 * Author: Alexandru Naiman
 *
 * 2 component vector
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

#ifndef _NE_MATH_VEC2_H_
#define _NE_MATH_VEC2_H_

#include <Math/defs.h>

static inline struct vec2 *
v2(
	struct vec2 *v,
	float x,
	float y)
{
	v->x = x;
	v->y = y;

	return v;
}

static inline struct vec2 *
v2_fill(
	struct vec2 *v,
	float f)
{
	v->x = v->y = f;

	return v;
}

static inline struct vec2 *
v2_copy(
	struct vec2 *dst,
	const struct vec2 *src)
{
	dst->x = src->x;
	dst->y = src->y;

	return dst;
}

static inline struct vec2 *
v2_zero(struct vec2 *v)
{
	v->x = 0.f;
	v->y = 0.f;
	
	return v;
}

static inline float
v2_len_sq(const struct vec2 *v)
{
	return (v->x * v->x) + (v->y * v->y);
}

static inline float
v2_len(const struct vec2 *v)
{
	return sqrtf((v->x * v->x) + (v->y * v->y));
}

static inline struct vec2 *
v2_norm(
	struct vec2 *dst,
	const struct vec2 *v)
{
	float l = 1.0f / v2_len(v);

	dst->x = v->x * l;
	dst->y = v->y * l;

	return dst;
}

static inline struct vec2 *
v2_lerp(
	struct vec2 *dst,
	const struct vec2 *v1,
	const struct vec2 *v2,
	float t)
{
	dst->x = v1->x + t * (v2->x - v1->x);
	dst->y = v1->y + t * (v2->y - v1->y);

	return dst;
}

static inline struct vec2 *
v2_add(
	struct vec2 *dst,
	const struct vec2 *v1,
	const struct vec2 *v2)
{
	dst->x = v1->x + v2->x;
	dst->y = v1->y + v2->y;

	return dst;
}

static inline struct vec2 *
v2_sub(
	struct vec2 *dst,
	const struct vec2 *v1,
	const struct vec2 *v2)
{
	dst->x = v1->x - v2->x;
	dst->y = v1->y - v2->y;

	return dst;
}

static inline struct vec2 *
v2_mul(
	struct vec2 *dst,
	const struct vec2 *v1,
	const struct vec2 *v2)
{
	dst->x = v1->x * v2->x;
	dst->y = v1->y * v2->y;

	return dst;
}

static inline struct vec2 *
v2_div(
	struct vec2 *dst,
	const struct vec2 *v1,
	const struct vec2 *v2)
{
	dst->x = v1->x / v2->x;
	dst->y = v1->y / v2->y;

	return dst;
}

static inline float
v2_dot(
	const struct vec2 *v1,
	const struct vec2 *v2)
{
	return v1->x * v2->x + v1->y * v2->y;
}

static inline float
v2_cross(
	const struct vec2 *v1,
	const struct vec2 *v2)
{
	return v1->x * v2->y - v1->y * v2->x;
}

static inline struct vec2 *
v2_mul_m3(
	struct vec2 *dst,
	const struct vec2 *v,
	const struct mat3 *m)
{
	dst->x = v->x * m->mat[0] + v->y * m->mat[3] + m->mat[6];
	dst->y = v->x * m->mat[1] + v->y * m->mat[4] + m->mat[7];

	return dst;
}

static inline struct vec2 *
v2_scale(
	struct vec2 *dst,
	const struct vec2 *v,
	const float s)
{
	dst->x = v->x * s;
	dst->y = v->y * s;

	return dst;
}

static inline bool
v2_equal(
	const struct vec2 *v1,
	const struct vec2 *v2)
{
	return float_equal(v1->x, v2->x) && float_equal(v1->y, v2->y);
}

static inline float
v2_angle(
	const struct vec2 *v1,
	const struct vec2 *v2)
{
	struct vec2 t1, t2;
	float cross;
	float dot;

	if (v2_equal(v1, v2))
		return 0.f;

	v2_norm(&t1, v1);
	v2_norm(&t2, v2);

	cross = v2_cross(&t1, &t2);
	dot = v2_dot(&t1, &t2);

	/*
	 * acos is only defined for -1 to 1. Outside the range we
	 * get NaN even if that's just because of a floating point error
	 * so we clamp to the -1 - 1 range
	 */

	if (dot > 1.f) dot = 1.f;
	if (dot < -1.f) dot = -1.f;

	return rad_to_deg(atan2f(cross, dot));
}

static inline float
v2_distance(
	const struct vec2 *v1,
	const struct vec2 *v2)
{
	struct vec2 diff;
	
	v2_sub(&diff, v2, v1);

	return fabsf(v2_len(&diff));
}

static inline struct vec2 *
v2_mid(
	struct vec2 *dst,
	const struct vec2 *v1,
	const struct vec2 *v2)
{
	struct vec2 sum;

	v2_add(&sum, v1, v2);
	
	dst->x = sum.x / 2.f;
	dst->y = sum.y / 2.f;

	return dst;
}

static inline struct vec2 *
v2_reflect(
	struct vec2 *dst,
	const struct vec2 *v,
	const struct vec2 *n)
{
	struct vec2 tmp;

	return v2_sub(dst, v, v2_scale(&tmp, n, 2.0f * v2_dot(v, n)));
}

static inline void
kmVec2Swap(
	struct vec2 *v1,
	struct vec2 *v2)
{
	float x = v1->x;
	float y = v1->y;
	
	v1->x = v2->x;
	v1->y = v2->y;

	v2->x = x;
	v2->y = y;
}

static const struct vec2 v2_pos_y = {  0.f,  1.f };
static const struct vec2 v2_neg_y = {  0.f, -1.f };
static const struct vec2 v2_neg_x = { -1.f,  0.f };
static const struct vec2 v2_pos_x = {  1.f,  0.f };

#endif /* _NE_MATH_VEC2_H_ */

