/* NekoEngine
 *
 * vec4_altivec.h
 * Author: Alexandru Naiman
 *
 * 4 component vector functions Altivec implementation
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

#ifndef _NE_MATH_ALTIVEC_VEC4_H_
#define _NE_MATH_ALTIVEC_VEC4_H_

#include <Math/defs.h>

#ifdef USE_ALTIVEC

static inline struct vec4 *
v4(struct vec4 *v, float x, float y, float z, float w)
{
	v->x = x;
	v->y = y;
	v->z = z;
	v->w = w;

	return v;
}

static inline struct vec4 *
v4_fill(struct vec4 *v, float f)
{
	v->sv = vec_splat(v->sv, f);
	return v;
}

static inline struct vec4 *
v4_copy(struct vec4 *dst, const struct vec4 *src)
{
	dst->x = src->x;
	dst->y = src->y;
	dst->z = src->z;
	dst->w = src->w;

	return dst;
}

static inline struct vec4 *
v4_zero(struct vec4 *v)
{
	return v4_fill(v, 0.f);
}

static inline struct vec4 *
v4_lerp(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2, float t)
{
	__vector float t0, t1, t2, v0;
	
	t2 = vec_splat(t2, t);
	t0 = vec_sub(v2->sv, v1->sv);
	t1 = vec_madd(t0, t2, v0); // FMA ?
	
	dst->sv = vec_add(v1->sv, t1);
	
	return dst;
}

static inline struct vec4 *
v4_add(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = vec_add(v1->sv, v2->sv);
	return dst;
}

static inline struct vec4 *
v4_sub(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = vec_sub(v1->sv, v2->sv);
	return dst;
}

static inline struct vec4 *
v4_mul(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	__vector float v0 = { 0 };
	dst->sv = vec_madd(v1->sv, v2->sv, v0);
	return dst;
}

static inline struct vec4 *
v4_div(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
//	dst->sv = vec_div(v1->sv, v2->sv);

	dst->x = v1->x / v2->x;
	dst->y = v1->y / v2->y;
	dst->z = v1->z / v2->z;
	dst->w = v1->w / v2->w;

	return dst;
}

static inline float
v4_dot(const struct vec4 *v1, const struct vec4 *v2)
{
	return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z + v1->w * v2->w;
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
	float l;

	if (!src->x && !src->y && !src->z && !src->w)
		return v4_copy(dst, src);

	l = 1.f / v4_len(src);

	dst->x = src->x * l;
	dst->y = src->y * l;
	dst->z = src->z * l;
	dst->w = src->w * l;

	return dst;
}

static inline struct vec4 *
v4_scale(struct vec4 *dst, const struct vec4 *src, const float s)
{
	v4_norm(dst, src);

//	struct vec4 tmp;
//	v4(&tmp, s, s, s, s);
//	dst->sv = vec_mul(src->sv, tmp.sv);

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

#endif /* _NE_MATH_ALTIVEC_VEC4_H_ */

