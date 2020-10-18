/* NekoEngine
 *
 * vec4_sse.h
 * Author: Alexandru Naiman
 *
 * 4 component vector functions SSE implementation
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

#ifndef _NE_MATH_SSE_VEC4_H_
#define _NE_MATH_SSE_VEC4_H_

#include <Math/defs.h>
#include <Math/mat4.h>

#ifdef USE_SSE

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
	dst->sv = _mm_loadu_ps(&src->x);
	return dst;
}

static inline struct vec4 *
v4_zero(struct vec4 *v)
{
	return v4_fill(v, 0.f);
}

static inline struct vec4 *
v4_lerp(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2, const float t)
{
	const __m128 t2 = _mm_set_ps1(t);
	const __m128 t0 = _mm_sub_ps(v2->sv, v1->sv);
	const __m128 t1 = _mm_mul_ps(t0, t2);
	
	dst->sv = _mm_add_ps(v1->sv, t1);
	
	return dst;
}

static inline struct vec4 *
v4_add(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = _mm_add_ps(v1->sv, v2->sv);
	return dst;
}

static inline struct vec4 *
v4_adds(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	const __m128 tmp = _mm_set_ps1(s);
	dst->sv = _mm_add_ps(v1->sv, tmp);
	return dst;
}

static inline struct vec4 *
v4_sub(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = _mm_sub_ps(v1->sv, v2->sv);
	return dst;
}

static inline struct vec4 *
v4_subs(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	const __m128 tmp = _mm_set_ps1(s);
	dst->sv = _mm_sub_ps(v1->sv, tmp);
	return dst;
}

static inline struct vec4 *
v4_mul(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = _mm_mul_ps(v1->sv, v2->sv);
	return dst;
}

static inline struct vec4 *
v4_muls(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	const __m128 tmp = _mm_set_ps1(s);
	dst->sv = _mm_mul_ps(v1->sv, tmp);
	return dst;
}

static inline struct vec4 *
v4_div(struct vec4 *dst, const struct vec4 *v1, const struct vec4 *v2)
{
	dst->sv = _mm_div_ps(v1->sv, v2->sv);
	return dst;
}

static inline struct vec4 *
v4_divs(struct vec4 *dst, const struct vec4 *v1, const float s)
{
	const __m128 tmp = _mm_set_ps1(s);
	dst->sv = _mm_div_ps(v1->sv, tmp);
	return dst;
}

static inline float
v4_dot(const struct vec4 *v1, const struct vec4 *v2)
{
#ifdef USE_AVX
	return _mm_cvtss_f32(_mm_dp_ps(v1->sv, v2->sv, 0xFF));
#else
	ALIGN(16) float out[4];
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
	const __m128 l = _mm_set1_ps(1.f / v4_len(src));
	dst->sv = _mm_mul_ps(src->sv, l);
	return dst;
}

static inline struct vec4 *
v4_scale(struct vec4 *dst, const struct vec4 *src, const float s)
{
	const __m128 scalar = _mm_set1_ps(s);

	v4_norm(dst, src);
	dst->sv = _mm_mul_ps(dst->sv, scalar);

	return dst;
}

static inline float
v4_distance(const struct vec4 *v1, const struct vec4 *v2)
{
	const __m128 tmp = _mm_sub_ps(v1->sv, v2->sv);

	// TODO: SSSE3
	// #include <tmmintrin.h>
	// _mm_abs_epi32
	// https://software.intel.com/sites/landingpage/IntrinsicsGuide
	// Need some kind of configuration / runtime detection for this
	// as it will break on older CPUS (pre C2D)


#ifdef USE_AVX
	return fabsf(sqrtf(_mm_cvtss_f32(_mm_dp_ps(tmp, tmp, 0xFF))));
#else
	ALIGN(16) float out[4];
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
	union {
		struct {
			uint32_t a, b, c, d;
		};
		__m128 v;
	} eq;
	eq.v = _mm_cmpeq_ps(p1->sv, p2->sv);
	return eq.a && eq.b && eq.c && eq.d;
}

static inline struct vec4 *
v4_mul_m4(struct vec4 *dst, const struct vec4 *v, const struct mat4 *m)
{
	struct mat4 tr;
	__m128 v0, v1, v2, v3;

	m4_transpose(&tr, m);

	v0 = _mm_shuffle_ps(v->sv, v->sv, _MM_SHUFFLE(0, 0, 0, 0));
	v1 = _mm_shuffle_ps(v->sv, v->sv, _MM_SHUFFLE(1, 1, 1, 1));
	v2 = _mm_shuffle_ps(v->sv, v->sv, _MM_SHUFFLE(2, 2, 2, 2));
	v3 = _mm_shuffle_ps(v->sv, v->sv, _MM_SHUFFLE(3, 3, 3, 3));

	dst->sv = _mm_add_ps(
		_mm_add_ps(_mm_mul_ps(v0, tr.sm[0]), _mm_mul_ps(v1, tr.sm[1])),
		_mm_add_ps(_mm_mul_ps(v2, tr.sm[2]), _mm_mul_ps(v3, tr.sm[3]))
	);

	return dst;
}

#endif

#endif /* _NE_MATH_SSE_VEC4_H_ */
