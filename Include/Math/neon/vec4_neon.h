/* NekoEngine
 *
 * vec4_neon.h
 * Author: Alexandru Naiman
 *
 * 4 component vector functions NEON implementation
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
 */

#ifndef _NE_MATH_NEON_VEC4_H_
#define _NE_MATH_NEON_VEC4_H_

#include <Math/defs.h>
#include <Math/mat4.h>

#ifdef USE_NEON

#include <arm_neon.h>

static inline struct vec4 *
v4(struct vec4 *v, float x, float y, float z, float w)
{
	ALIGN(16) float data[4] = { x, y, z, w };
	v->sv = vld1q_f32(data);
	return v;
}

static inline struct vec4 *
v4_fill(struct vec4 *v, float f)
{
	v->sv = vdupq_n_f32(f);
	return v;
}

static inline struct vec4 *
v4_copy(struct vec4 *dst, const struct vec4 *src)
{
	dst->sv = vld1q_f32(&src->x);
	return dst;
}

static inline struct vec4 *
v4_zero(struct vec4 *v)
{
	v->sv = vmovq_n_f32(0.f);
	return v;
}

static inline struct vec4 *
v4_lerp(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2, float t)
{
	const float32x4_t t2 = vdupq_n_f32(t);
	const float32x4_t t0 = vsubq_f32(v1->sv, v2->sv);
	dst-> sv = vmlaq_f32(v1->sv, t0, t2);
	return dst;
}

static inline struct vec4 *
v4_add(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = vaddq_f32(v1->sv, v2->sv);
	return dst;
}


static inline struct vec4 *
v4_adds(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	float32x4_t tmp = vdupq_n_f32(s);
	dst->sv = vaddq_f32(v1->sv, tmp);
	return dst;
}

static inline struct vec4 *
v4_sub(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = vsubq_f32(v1->sv, v2->sv);
	return dst;
}

static inline struct vec4 *
v4_subs(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	float32x4_t tmp = vdupq_n_f32(s);
	dst->sv = vsubq_f32(v1->sv, tmp);
	return dst;
}

static inline struct vec4 *
v4_mul(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = vmulq_f32(v1->sv, v2->sv);
	return dst;
}

static inline struct vec4 *
v4_muls(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	dst->sv = vmulq_n_f32(v1->sv, s);
	return dst;
}

static inline struct vec4 *
v4_div(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = vdivq_f32(v1->sv, v2->sv);
	return dst;
}

static inline struct vec4 *
v4_divs(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	float32x4_t tmp = vdupq_n_f32(s);
	dst->sv = vdivq_f32(v1->sv, tmp);
	return dst;
}

static inline float
v4_dot(const struct vec4 *v1, const struct vec4 *v2)
{
	const float32x4_t tmp = vmulq_f32(v1->sv, v2->sv);
	float32x2_t t1 = vget_low_f32(tmp);
	float32x2_t t2 = vget_high_f32(tmp);
	t1 = vadd_f32(t1, t2);
	t1 = vpadd_f32(t1, t1);
	return vget_lane_f32(t1, 0);
}

static inline float
v4_len_sq(const struct vec4 *v)
{
	return v4_dot(v, v);
}

static inline float
v4_len(const struct vec4 *v)
{
	return sqrtf(v4_len_sq(v));
}

static inline struct vec4 *
v4_norm(struct vec4 *dst, const struct vec4 *src)
{
	const float l = 1.f / v4_len(src);
	dst->sv = vmulq_n_f32(src->sv, l);
	return dst;
}

static inline struct vec4 *
v4_scale(struct vec4 *dst, const struct vec4 *src, const float s)
{
	v4_norm(dst, src);
	dst->sv = vmulq_n_f32(dst->sv, s);
	return dst;
}

static inline float
v4_distance(const struct vec4 *v1, const struct vec4 *v2)
{
	struct vec4 diff;
	v4_sub(&diff, v2, v1);
	return fabsf(v4_len(&diff));
}

static inline void
v4_swap(struct vec4 *a, struct vec4 *b)
{
	const float x = a->x;
	const float y = a->y;
	const float z = a->z;
	const float w = a->w;
	
	a->x = b->x; b->x = x;
	a->y = b->y; b->y = y;
	a->z = b->z; b->z = z;
	a->w = b->w; b->w = w;
}

static inline int
v4_equal(const struct vec4 *p1, const struct vec4 *p2)
{
	// TODO: vceqq_f32
	return  (p1->x < p2->x + FLT_EPSILON && p1->x > p2->x - FLT_EPSILON) &&
		(p1->y < p2->y + FLT_EPSILON && p1->y > p2->y - FLT_EPSILON) &&
		(p1->z < p2->z + FLT_EPSILON && p1->z > p2->z - FLT_EPSILON) &&
		(p1->w < p2->w + FLT_EPSILON && p1->w > p2->w - FLT_EPSILON);
}

static inline struct vec4 *
v4_mul_m4(struct vec4 *dst, const struct vec4 *v, const struct mat4 *m)
{
	struct mat4 tr;
	float32x4_t v0, v1;

	m4_transpose(&tr, m);

	v0 = vmulq_f32(vdupq_n_f32(v->x), tr.sm[0]);
	v0 = vmlaq_f32(v0, vdupq_n_f32(v->y), tr.sm[1]);
	v1 = vmulq_f32(vdupq_n_f32(v->z), tr.sm[2]);
	v1 = vmlaq_f32(v1, vdupq_n_f32(v->w), tr.sm[3]);

	dst->sv = vaddq_f32(v0, v1);

	return dst;
}

#endif

#endif /* _NE_MATH_NEON_VEC4_H_ */
