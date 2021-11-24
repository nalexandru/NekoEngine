/* NekoEngine
 *
 * mat4_sse.h
 * Author: Alexandru Naiman
 *
 * 4x4 matrix functions SSE implementation
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

#ifndef _NE_MATH_SSE_MAT4_H_
#define _NE_MATH_SSE_MAT4_H_

#include <Math/defs.h>

#ifdef USE_SSE

#ifdef USE_AVX2
#	include <immintrin.h>
#endif

#define M4_INVERSE_NOSIMD

#include <Math/vec3.h>
#include <Math/quat.h>

static inline struct mat4 *
m4_simd(struct mat4 *dst, const float *m)
{
	dst->sm[0] = _mm_setr_ps(m[0], m[1], m[2], m[3]);
	dst->sm[1] = _mm_setr_ps(m[4], m[5], m[6], m[7]);
	dst->sm[2] = _mm_setr_ps(m[8], m[9], m[10], m[11]);
	dst->sm[3] = _mm_setr_ps(m[12], m[13], m[14], m[15]);
	return dst;
}

static inline struct mat4 *
m4f_simd(struct mat4 *dst,
	float m0, float m1, float m2, float m3,
	float m4, float m5, float m6, float m7,
	float m8, float m9, float m10, float m11,
	float m12, float m13, float m14, float m15)
{
	dst->sm[0] = _mm_setr_ps(m0, m1, m2, m3);
	dst->sm[1] = _mm_setr_ps(m4, m5, m6, m7);
	dst->sm[2] = _mm_setr_ps(m8, m9, m10, m11);
	dst->sm[3] = _mm_setr_ps(m12, m13, m14, m15);
	return dst;
}

static inline struct mat4 *
m4_copy_simd(struct mat4 *dst, const struct mat4 *src)
{
	memcpy(dst->m, src->m, sizeof(float) * 16);
	return dst;
}

static inline struct mat4 *
m4_mul_simd(struct mat4 *dst, const struct mat4 *m1, const struct mat4 *m2)
{
	__m128 tmp0, tmp1, tmp2, tmp3;
		
	tmp0 = _mm_mul_ps(_mm_shuffle_ps(m2->sm[0], m2->sm[0x00], 0x00), m1->sm[0]);
	tmp0 = _mm_add_ps(tmp0, _mm_mul_ps(_mm_shuffle_ps(m2->sm[0], m2->sm[0], 0x55), m1->sm[1]));
	tmp0 = _mm_add_ps(tmp0, _mm_mul_ps(_mm_shuffle_ps(m2->sm[0], m2->sm[0], 0xAA), m1->sm[2]));
	tmp0 = _mm_add_ps(tmp0, _mm_mul_ps(_mm_shuffle_ps(m2->sm[0], m2->sm[0], 0xFF), m1->sm[3]));

	tmp1 = _mm_mul_ps(_mm_shuffle_ps(m2->sm[1], m2->sm[1], 0x00), m1->sm[0]);
	tmp1 = _mm_add_ps(tmp1, _mm_mul_ps(_mm_shuffle_ps(m2->sm[1], m2->sm[1], 0x55), m1->sm[1]));
	tmp1 = _mm_add_ps(tmp1, _mm_mul_ps(_mm_shuffle_ps(m2->sm[1], m2->sm[1], 0xAA), m1->sm[2]));
	tmp1 = _mm_add_ps(tmp1, _mm_mul_ps(_mm_shuffle_ps(m2->sm[1], m2->sm[1], 0xFF), m1->sm[3]));

	tmp2 = _mm_mul_ps(_mm_shuffle_ps(m2->sm[2], m2->sm[2], 0x00), m1->sm[0]);
	tmp2 = _mm_add_ps(tmp2, _mm_mul_ps(_mm_shuffle_ps(m2->sm[2], m2->sm[2], 0x55), m1->sm[1]));
	tmp2 = _mm_add_ps(tmp2, _mm_mul_ps(_mm_shuffle_ps(m2->sm[2], m2->sm[2], 0xAA), m1->sm[2]));
	tmp2 = _mm_add_ps(tmp2, _mm_mul_ps(_mm_shuffle_ps(m2->sm[2], m2->sm[2], 0xFF), m1->sm[3]));

	tmp3 = _mm_mul_ps(_mm_shuffle_ps(m2->sm[3], m2->sm[3], 0x00), m1->sm[0]);
	tmp3 = _mm_add_ps(tmp3, _mm_mul_ps(_mm_shuffle_ps(m2->sm[3], m2->sm[3], 0x55), m1->sm[1]));
	tmp3 = _mm_add_ps(tmp3, _mm_mul_ps(_mm_shuffle_ps(m2->sm[3], m2->sm[3], 0xAA), m1->sm[2]));
	tmp3 = _mm_add_ps(tmp3, _mm_mul_ps(_mm_shuffle_ps(m2->sm[3], m2->sm[3], 0xFF), m1->sm[3]));

	dst->sm[0] = tmp0;
	dst->sm[1] = tmp1;
	dst->sm[2] = tmp2;
	dst->sm[3] = tmp3;

	return dst;
}

static inline struct mat4 *
m4_muls_simd(struct mat4 *dst, const struct mat4 *m, const float f)
{
	const __m128 scalar = _mm_set1_ps(f);

	dst->sm[0] = _mm_mul_ps(m->sm[0], scalar);
	dst->sm[1] = _mm_mul_ps(m->sm[1], scalar);
	dst->sm[2] = _mm_mul_ps(m->sm[2], scalar);
	dst->sm[3] = _mm_mul_ps(m->sm[3], scalar);

	return dst;
}

static inline struct mat4 *
m4_transpose_simd(struct mat4 *dst, const struct mat4 *src)
{
	(void)m4_copy_simd(dst, src);
	_MM_TRANSPOSE4_PS(dst->sm[0], dst->sm[1], dst->sm[2], dst->sm[3]);
	return dst;
}

static inline struct mat4 *
m4_inverse_simd(struct mat4 *dst, const struct mat4 *src)
{
	(void)src; // TODO
	return dst;
}

static inline struct vec4 *
v4_mul_m4_simd(struct vec4 *dst, const struct vec4 *v, const struct mat4 *m)
{
	struct mat4 tr;
	__m128 v0, v1, v2, v3;

	m4_transpose_simd(&tr, m);

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

#endif /* _NE_MATH_SSE_MAT4_H_ */
