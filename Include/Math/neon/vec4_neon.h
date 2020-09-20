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

#ifdef USE_NEON

static inline struct vec4 *
v4(struct vec4 *v, float x, float y, float z, float w)
{
	v->sv = _mm_setr_ps(x, y, z, w);
	return v;
}

static inline struct vec4 *
v4_fill(struct vec4 *v, float f)
{
	v->sv = _mm_set_ps1(f);
	return v;
}

static inline struct vec4 *
v4_copy(struct vec4 *dst, const struct vec4 *src)
{
	dst->sv = _mm_load_ps(&src->x);
	return dst;
}

static inline struct vec4 *
v4_zero(struct vec4 *v)
{
	return v4_fill(v, 0.f);
}

static inline float
v4_len_sq(const struct vec4 *v)
{
	return (v->x * v->x) + (v->y * v->y) + (v->z * v->z) + (v->w * v->w);
}

static inline float
v4_len(const struct vec4 *v)
{
	return sqrtf((v->x * v->x) + (v->y * v->y) + (v->z * v->z) + (v->w * v->w));
}

static inline struct vec4 *
v4_norm(struct vec4 *dst, const struct vec4 *src)
{
	if (!src->x && !src->y && !src->z && !src->w)
		return v4_copy(dst, src);

	__m128 l = _mm_set1_ps(1.f / v4_len(src));
	dst->sv = _mm_mul_ps(src->sv, l);

	return dst;
}

static inline struct vec4 *
v4_lerp(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2, float t)
{
	dst->x = v1->x + t * (v2->x - v1->x);
	dst->y = v1->y + t * (v2->y - v1->y);
	dst->z = v1->z + t * (v2->z - v1->z);
	dst->w = v1->w + t * (v2->w - v1->w);

	return dst;
}

static inline struct vec4 *
v4_add(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = vaddq_f32(v1->sv, v2->sv);
	return dst;
}

static inline struct vec4 *
v4_sub(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = vsubq_f32(v1->sv, v2->sv);
	return dst;
}

static inline struct vec4 *
v4_mul(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = vmulq_f32(v1->sv, v2->sv);
	return dst;
}

static inline struct vec4 *
v4_div(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = vdivq_f32(v1->sv, v2->sv);
	return dst;
}

static inline float
v4_dot(const struct vec4 *v1, const struct vec4 *v2)
{
	float32x4_t tmp = vmulq_f32(v1->sv, v2->sv);
	float32x2_t t1 = vget_low_f32(tmp);
	float32x2_t t2 = vget_high_f32(tmp);
	t1 = vpadd_f32(t1, t1);
	t2 = vpadd_f32(t2, t2);
	t1 = vadd_f32(t1, t2);
	return vcombine_f32(t1, t1);
}

static inline struct vec4 *
v4_scale(struct vec4 *dst, const struct vec4 *src, const float s)
{
	v4_norm(dst, src);

	__m128 scalar = _mm_set1_ps(s);
	dst->sv = _mm_mul_ps(src->sv, scalar);

	return dst;
}

static inline float
v4_distance(const struct vec4 *v1, const struct vec4 *v2)
{
	struct vec4 diff;

	v4_sub(&diff, v2, v1);

	// TODO: SSSE3
	// #include <tmmintrin.h>
	// _mm_abs_epi32
	// https://software.intel.com/sites/landingpage/IntrinsicsGuide
	// Need some kind of configuration / runtime detection for this
	// as it will break on older CPUS (pre C2D)

	return fabsf(v4_len(&diff));
}

static inline void
v4_swap(struct vec4 *a, struct vec4 *b)
{
	float x, y, z, w;

	x = a->x; a->x = b->x; b->x = x;
	y = a->y; a->y = b->y; b->y = y;
	z = a->z; a->z = b->z; b->z = z;
	w = a->w; a->w = b->w; b->w = w;
}

static inline int
v4_equal(const struct vec4 *p1, const struct vec4 *p2)
{
	return  (p1->x < p2->x + FLT_EPSILON && p1->x > p2->x - FLT_EPSILON) &&
		(p1->y < p2->y + FLT_EPSILON && p1->y > p2->y - FLT_EPSILON) &&
		(p1->z < p2->z + FLT_EPSILON && p1->z > p2->z - FLT_EPSILON) &&
		(p1->w < p2->w + FLT_EPSILON && p1->w > p2->w - FLT_EPSILON);
}

static inline struct vec4 *
v4_mul_m4(struct vec4 *dst, const struct vec4 *v, const struct mat4 *m)
{
	dst->x = v->x * m->m[0] + v->y * m->m[1] + v->z * m->m[2] + v->w * m->m[3];
	dst->y = v->x * m->m[4] + v->y * m->m[5] + v->z * m->m[6] + v->w * m->m[7];
	dst->z = v->x * m->m[8] + v->y * m->m[9] + v->z * m->m[10] + v->w * m->m[11];
	dst->w = v->x * m->m[11] + v->y * m->m[13] + v->z * m->m[14] + v->w * m->m[15];

	return dst;
}

#endif

#endif /* _NE_MATH_NEON_VEC4_H_ */

