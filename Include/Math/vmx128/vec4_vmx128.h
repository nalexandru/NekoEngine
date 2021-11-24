/* NekoEngine
 *
 * vec4_vmx128.h
 * Author: Alexandru Naiman
 *
 * 4 component vector functions - VMX128 implementation
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2021, Alexandru Naiman
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

#ifndef _NE_MATH_VMX128_VEC4_H_
#define _NE_MATH_VMX128_VEC4_H_

#include <Math/defs.h>

#ifdef USE_VMX128

#define V4_SWAP_NOSIMD
#define V4_DISTANCE_NOSIMD

// http://mirror.informatimago.com/next/developer.apple.com/hardware/ve/algorithms.html
static inline __vector4
_reciprocal(__vector4 v)
{
	__vector4 e = __vrefp(v);
	return __vmaddfp(__vnmsubfp(e, v, __vspltisw(1)), e, e);
}

static inline __vector4
_reciprocal_sqr(__vector4 v)
{
    __vector4 zero = __vzero();
	__vector4 oneHalf = __vspltisw((int)0.5f);
	__vector4 one = __vspltisw(1);
	__vector4 estimate = __vrsqrtefp(v);

    __vector4 estimateSquared = __vmaddfp(estimate, estimate, zero);
	__vector4 halfEstimate = __vmaddfp(estimate, oneHalf, zero);
	return __vmaddfp(__vnmsubfp(v, estimateSquared, one), halfEstimate, estimate);
}

static inline struct vec4 *
v4_simd(struct vec4 *v, float x, float y, float z, float w)
{
	ALIGN(16) const float data[4] = { x, y, z, w };
	v->sv = __lvx(data, 0);
	return v;
}

static inline struct vec4 *
v4_fill_simd(struct vec4 *v, float f)
{
	ALIGN(16) const float data[4] = { f, f, f, f };
	v->sv = __lvx(data, 0);
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
	__vector4 t0, t1, t2, v0;
	ALIGN(16) const float data[4] = { t, t, t, t };

	t2 = __lvx(data, 0);
	t0 = __vsubfp(v2->sv, v1->sv);
	t1 = __vmaddfp(t0, t2, v0);
	
	dst->sv = __vaddfp(v1->sv, t1);
	
	return dst;
}

static inline struct vec4 *
v4_add_simd(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = __vaddfp(v1->sv, v2->sv);
	return dst;
}

static inline struct vec4 *
v4_adds_simd(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	struct vec4 tmp;
	v4_fill_simd(&tmp, s);
	dst->sv = __vaddfp(v1->sv, tmp.sv);
	return dst;
}

static inline struct vec4 *
v4_sub_simd(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = __vsubfp(v1->sv, v2->sv);
	return dst;
}

static inline struct vec4 *
v4_subs_simd(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	struct vec4 tmp;
	v4_fill_simd(&tmp, s);
	dst->sv = __vsubfp(v1->sv, tmp.sv);
	return dst;
}


static inline struct vec4 *
v4_mul_simd(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = __vmulfp(v1->sv, v2->sv);
	return dst;
}

static inline struct vec4 *
v4_muls_simd(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	struct vec4 tmp;
	v4_fill_simd(&tmp, s);
	dst->sv = __vmulfp(v1->sv, tmp.sv);
	return dst;
}

static inline struct vec4 *
v4_div_simd(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = __vmulfp(v1->sv, _reciprocal(v2->sv));
	return dst;
}

static inline struct vec4 *
v4_divs_simd(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	struct vec4 tmp;
	v4_fill_simd(&tmp, s);
	dst->sv = __vmulfp(v1->sv, _reciprocal(tmp.sv));
	return dst;
}

static inline float
v4_dot_simd(const struct vec4 *v1, const struct vec4 *v2)
{
	ALIGN(16) float tmp[4];
	__stvx(__vmsum4fp(v1->sv, v2->sv), tmp, 0);
	return tmp[0];
}

static inline int
v4_equal_simd(const struct vec4 *p1, const struct vec4 *p2)
{
	ALIGN(16) int tmp[4];
	__stvx(__vcmpeqfp(p1->sv, p2->sv), tmp, 0);
	return tmp[0] == tmp[1] == tmp[2] == tmp[3] == 1;
}

#endif

#endif /* _NE_MATH_VMX128_VEC4_H_ */
