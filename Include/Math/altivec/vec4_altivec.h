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
#include <Math/mat4.h>

#ifdef USE_ALTIVEC

// http://mirror.informatimago.com/next/developer.apple.com/hardware/ve/algorithms.html
static inline vector float
_reciprocal(vector float v)
{
	vector float e = vec_re(v);
	return vec_madd(vec_nmsub(e, v, (vector float)1.0), e, e);
}

static inline vector float
_reciprocal_sqr(vector float v)
{
    vector float zero = (vector float)(0);
	vector float oneHalf = (vector float)(0.5);
	vector float one = (vector float)(1.0);
	vector float estimate = vec_rsqrte(v);

    vector float estimateSquared = vec_madd(estimate, estimate, zero);
	vector float halfEstimate = vec_madd(estimate, oneHalf, zero);
	return vec_madd(vec_nmsub(v, estimateSquared, one), halfEstimate, estimate);
}

static inline struct vec4 *
v4(struct vec4 *v, float x, float y, float z, float w)
{
	ALIGN(16) const float data[4] = { x, y, z, w };
	v->sv = vec_ld(0, data);
	return v;
}

static inline struct vec4 *
v4_fill(struct vec4 *v, float f)
{
	ALIGN(16) const float data[4] = { f, f, f, f };
	v->sv = vec_ld(0, data);
	return v;
}

static inline struct vec4 *
v4_copy(struct vec4 *dst, const struct vec4 *src)
{
	dst->sv = src->sv;
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
	vector float t0, t1, t2, v0;
	
	t2 = vec_splat(t2, t);
	t0 = vec_sub(v2->sv, v1->sv);
	t1 = vec_madd(t0, t2, v0);
	
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
v4_adds(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	struct vec4 tmp;
	v4_fill(&tmp, s);
	dst->sv = vec_add(v1->sv, tmp.sv);
	return dst;
}

static inline struct vec4 *
v4_sub(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = vec_sub(v1->sv, v2->sv);
	return dst;
}

static inline struct vec4 *
v4_subs(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	struct vec4 tmp;
	v4_fill(&tmp, s);
	dst->sv = vec_sub(v1->sv, tmp.sv);
	return dst;
}


static inline struct vec4 *
v4_mul(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = vec_madd(v1->sv, v2->sv, (vector float)(0));
	return dst;
}

static inline struct vec4 *
v4_muls(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	struct vec4 tmp;
	v4_fill(&tmp, s);
	dst->sv = vec_madd(v1->sv, tmp.sv, (vector float)(0));
	return dst;
}

static inline struct vec4 *
v4_div(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = vec_madd(v1->sv, _reciprocal(v2->sv), (vector float)(0));
	return dst;
}

static inline struct vec4 *
v4_divs(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	struct vec4 tmp;
	v4_fill(&tmp, s);
	dst->sv = vec_madd(v1->sv, _reciprocal(tmp.sv), (vector float)(0));
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
	vector float zero = (vector float)vec_splat_u32(0);
	struct vec4 l;
	v4_fill(&l, 1.f / v4_len(src));
	dst->sv = vec_madd(src->sv, l.sv, zero);
	return dst;
}

static inline struct vec4 *
v4_scale(struct vec4 *dst, const struct vec4 *src, const float s)
{
	struct vec4 tmp;
	vector float zero = (vector float)vec_splat_u32(0);
	v4_fill(&tmp, s);
	v4_norm(dst, src);
	dst->sv = vec_madd(dst->sv, tmp.sv, zero);
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
	return vec_all_eq(p1->sv, p2->sv);
}

static inline struct vec4 *
v4_mul_m4(struct vec4 *dst, const struct vec4 *v, const struct mat4 *m)
{
	struct mat4 tr;
	vector float v0, v1;
	const vector float zero = (vector float)vec_splat_u32(0);

	m4_transpose(&tr, m);
		
	v0 = vec_madd(vec_splat(v->sv, 0), tr.sm[0], zero);
	v0 = vec_madd(vec_splat(v->sv, 1), tr.sm[1], v0);
	
	v1 = vec_madd(vec_splat(v->sv, 2), tr.sm[2], zero);
	v1 = vec_madd(vec_splat(v->sv, 3), tr.sm[3], v1);
	
	dst->sv = vec_add(v0, v1);
	
	return dst;
}

#endif

#endif /* _NE_MATH_ALTIVEC_VEC4_H_ */
