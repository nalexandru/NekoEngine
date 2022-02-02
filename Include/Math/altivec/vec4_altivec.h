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

#define V4_DOT_NOSIMD
#define V4_SWAP_NOSIMD
#define V4_DISTANCE_NOSIMD

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
v4_simd(struct vec4 *v, float x, float y, float z, float w)
{
	NE_ALIGN(16) const float data[4] = { x, y, z, w };
	v->sv = vec_ld(0, data);
	return v;
}

static inline struct vec4 *
v4_fill_simd(struct vec4 *v, float f)
{
	NE_ALIGN(16) const float data[4] = { f, f, f, f };
	v->sv = vec_ld(0, data);
	return v;
}

static inline struct vec4 *
v4_copy_simd(struct vec4 *dst, const struct vec4 *src)
{
	dst->sv = src->sv;
	return dst;
}

static inline struct vec4 *
v4_zero_simd(struct vec4 *v)
{
	return v4_fill_simd(v, 0.f);
}

static inline struct vec4 *
v4_lerp_simd(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2, float t)
{
	vector float t0, t1, t2, v0;
	
	t2 = vec_splat(t2, t);
	t0 = vec_sub(v2->sv, v1->sv);
	t1 = vec_madd(t0, t2, v0);
	
	dst->sv = vec_add(v1->sv, t1);
	
	return dst;
}

static inline struct vec4 *
v4_add_simd(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = vec_add(v1->sv, v2->sv);
	return dst;
}

static inline struct vec4 *
v4_adds_simd(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	struct vec4 tmp;
	v4_fill_simd(&tmp, s);
	dst->sv = vec_add(v1->sv, tmp.sv);
	return dst;
}

static inline struct vec4 *
v4_sub_simd(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = vec_sub(v1->sv, v2->sv);
	return dst;
}

static inline struct vec4 *
v4_subs_simd(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	struct vec4 tmp;
	v4_fill_simd(&tmp, s);
	dst->sv = vec_sub(v1->sv, tmp.sv);
	return dst;
}


static inline struct vec4 *
v4_mul_simd(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = vec_madd(v1->sv, v2->sv, (vector float)(0));
	return dst;
}

static inline struct vec4 *
v4_muls_simd(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	struct vec4 tmp;
	v4_fill_simd(&tmp, s);
	dst->sv = vec_madd(v1->sv, tmp.sv, (vector float)(0));
	return dst;
}

static inline struct vec4 *
v4_div_simd(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = vec_madd(v1->sv, _reciprocal(v2->sv), (vector float)(0));
	return dst;
}

static inline struct vec4 *
v4_divs_simd(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	struct vec4 tmp;
	v4_fill_simd(&tmp, s);
	dst->sv = vec_madd(v1->sv, _reciprocal(tmp.sv), (vector float)(0));
	return dst;
}

static inline float
v4_dot_simd(const struct vec4 *v1, const struct vec4 *v2)
{
	// TODO
	return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z + v1->w * v2->w;
}

static inline int
v4_equal_simd(const struct vec4 *p1, const struct vec4 *p2)
{
	return vec_all_eq(p1->sv, p2->sv);
}

#endif

#endif /* _NE_MATH_ALTIVEC_VEC4_H_ */
