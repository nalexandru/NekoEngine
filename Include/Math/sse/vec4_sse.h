#ifndef _NE_MATH_SSE_VEC4_H_
#define _NE_MATH_SSE_VEC4_H_

#include <Math/defs.h>

#ifdef USE_SSE

static inline struct NeVec4 *
M_Vec4_SIMD(struct NeVec4 *v, float x, float y, float z, float w)
{
	v->sv = _mm_setr_ps(x, y, z, w);
	return v;
}

static inline struct NeVec4 *
M_FillVec4_SIMD(struct NeVec4 *v, float f)
{
	v->sv = _mm_set_ps1(f);
	return v;
}

static inline struct NeVec4 *
M_CopyVec4_SIMD(struct NeVec4 *dst, const struct NeVec4 *src)
{
	dst->sv = _mm_loadu_ps(&src->x);
	return dst;
}

static inline struct NeVec4 *
M_ZeroVec4_SIMD(struct NeVec4 *v)
{
	return M_FillVec4_SIMD(v, 0.f);
}

static inline struct NeVec4 *
M_LerpVec4_SIMD(struct NeVec4 *dst, const struct NeVec4 *v1, const struct NeVec4 *v2, const float t)
{
	const __m128 t2 = _mm_set_ps1(t);
	const __m128 t0 = _mm_sub_ps(v2->sv, v1->sv);
	const __m128 t1 = _mm_mul_ps(t0, t2);
	
	dst->sv = _mm_add_ps(v1->sv, t1);
	
	return dst;
}

static inline struct NeVec4 *
M_AddVec4_SIMD(struct NeVec4 *dst, const struct NeVec4 *v1, const struct NeVec4 *v2)
{
	dst->sv = _mm_add_ps(v1->sv, v2->sv);
	return dst;
}

static inline struct NeVec4 *
M_AddVec4S_SIMD(struct NeVec4 *dst, const struct NeVec4 *v1, const float s)
{
	const __m128 tmp = _mm_set_ps1(s);
	dst->sv = _mm_add_ps(v1->sv, tmp);
	return dst;
}

static inline struct NeVec4 *
M_SubVec4_SIMD(struct NeVec4 *dst, const struct NeVec4 *v1, const struct NeVec4 *v2)
{
	dst->sv = _mm_sub_ps(v1->sv, v2->sv);
	return dst;
}

static inline struct NeVec4 *
M_SubVec4S_SIMD(struct NeVec4 *dst, const struct NeVec4 *v1, const float s)
{
	const __m128 tmp = _mm_set_ps1(s);
	dst->sv = _mm_sub_ps(v1->sv, tmp);
	return dst;
}

static inline struct NeVec4 *
M_MulVec4_SIMD(struct NeVec4 *dst, const struct NeVec4 *v1, const struct NeVec4 *v2)
{
	dst->sv = _mm_mul_ps(v1->sv, v2->sv);
	return dst;
}

static inline struct NeVec4 *
M_MulVec4S_SIMD(struct NeVec4 *dst, const struct NeVec4 *v1, const float s)
{
	const __m128 tmp = _mm_set_ps1(s);
	dst->sv = _mm_mul_ps(v1->sv, tmp);
	return dst;
}

static inline struct NeVec4 *
M_DivVec4_SIMD(struct NeVec4 *dst, const struct NeVec4 *v1, const struct NeVec4 *v2)
{
	dst->sv = _mm_div_ps(v1->sv, v2->sv);
	return dst;
}

static inline struct NeVec4 *
M_DivVec4S_SIMD(struct NeVec4 *dst, const struct NeVec4 *v1, const float s)
{
	const __m128 tmp = _mm_set_ps1(s);
	dst->sv = _mm_div_ps(v1->sv, tmp);
	return dst;
}

static inline float
M_DotVec4_SIMD(const struct NeVec4 *v1, const struct NeVec4 *v2)
{
#ifdef USE_AVX
	return _mm_cvtss_f32(_mm_dp_ps(v1->sv, v2->sv, 0xFF));
#else
	NE_ALIGN(16) float out[4];
	__m128 t2 = v2->sv;
	__m128 t1 = _mm_mul_ps(v1->sv, t2);
	

	t2 = _mm_shuffle_ps(t2, t1, _MM_SHUFFLE(1, 0, 0, 0));
	t2 = _mm_add_ps(t2, t1);

	t1 = _mm_shuffle_ps(t1, t2, _MM_SHUFFLE(0, 3, 0, 0));
	t1 = _mm_add_ps(t1, t2);

	_mm_store1_ps(out, _mm_shuffle_ps(t1, t1, _MM_SHUFFLE(2, 2, 2, 2)));

	return out[0];
#endif
}

static inline float
M_Vec4Distance_SIMD(const struct NeVec4 *v1, const struct NeVec4 *v2)
{
	const __m128 tmp = _mm_sub_ps(v1->sv, v2->sv);

#ifdef USE_AVX
	return fabsf(sqrtf(_mm_cvtss_f32(_mm_dp_ps(tmp, tmp, 0xFF))));
#else
	NE_ALIGN(16) float out[4];
	__m128 t2 = tmp;
	__m128 t1 = _mm_mul_ps(tmp, t2);
	
	t2 = _mm_shuffle_ps(t2, t1, _MM_SHUFFLE(1, 0, 0, 0));
	t2 = _mm_add_ps(t2, t1);

	t1 = _mm_shuffle_ps(t1, t2, _MM_SHUFFLE(0, 3, 0, 0));
	t1 = _mm_add_ps(t1, t2);

	_mm_store1_ps(out, _mm_shuffle_ps(t1, t1, _MM_SHUFFLE(2, 2, 2, 2)));

	return out[0];
#endif
}

static inline void
M_SwapVec4_SIMD(struct NeVec4 *a, struct NeVec4 *b)
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
M_Vec4Equal_SIMD(const struct NeVec4 *p1, const struct NeVec4 *p2)
{
	union {
		struct {
			uint32_t a, b, c, d;
		};
		__m128 v;
	} eq;
	eq.v = _mm_cmpeq_ps(p1->sv, p2->sv);
	return eq.a && eq.b && eq.c && eq.d;
}

#endif

#endif /* _NE_MATH_SSE_VEC4_H_ */

/* NekoEngine
 *
 * vec4_sse.h
 * Author: Alexandru Naiman
 *
 * 4 component vector functions SSE implementation
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
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
 */
