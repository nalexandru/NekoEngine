/* NekoEngine
 *
 * mat4_altivec.h
 * Author: Alexandru Naiman
 *
 * 4x4 matrix functions
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

#ifndef _NE_MATH_ALTIVEC_MAT4_H_
#define _NE_MATH_ALTIVEC_MAT4_H_

#include <Math/defs.h>

#ifdef USE_ALTIVEC

#include <Math/vec3.h>
#include <Math/quat.h>

static inline struct mat4 *
m4_simd(struct mat4 *dst, const float *m)
{
	dst->sm[0] = vec_ld(0, m);
	dst->sm[1] = vec_ld(1 * sizeof(vector float), m);
	dst->sm[2] = vec_ld(2 * sizeof(vector float), m);
	dst->sm[3] = vec_ld(3 * sizeof(vector float), m);
	return dst;
}

static inline struct mat4 *
m4f_simd(struct mat4 *dst,
	float m0, float m1, float m2, float m3,
	float m4, float m5, float m6, float m7,
	float m8, float m9, float m10, float m11,
	float m12, float m13, float m14, float m15)
{
	dst->m[0] = m0; dst->m[1] = m1; dst->m[2] = m2; dst->m[3] = m3;
	dst->m[4] = m4; dst->m[5] = m5; dst->m[6] = m6; dst->m[7] = m7;
	dst->m[8] = m8; dst->m[9] = m9; dst->m[10] = m10; dst->m[11] = m11;
	dst->m[12] = m12; dst->m[13] = m13; dst->m[14] = m14; dst->m[15] = m15;
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
	vector float zero = (vector float)vec_splat_u32(0);
	vector float r0, r1, r2, r3;

	// TODO: profile instruction order
	r0 = vec_madd(vec_splat(m2->sm[0], 0), m1->sm[0], zero);
	r1 = vec_madd(vec_splat(m2->sm[1], 0), m1->sm[0], zero);
	r2 = vec_madd(vec_splat(m2->sm[2], 0), m1->sm[0], zero);
	r3 = vec_madd(vec_splat(m2->sm[3], 0), m1->sm[0], zero);

	r0 = vec_madd(vec_splat(m2->sm[0], 1), m1->sm[1], r0);
	r1 = vec_madd(vec_splat(m2->sm[1], 1), m1->sm[1], r1);
	r2 = vec_madd(vec_splat(m2->sm[2], 1), m1->sm[1], r2);
	r3 = vec_madd(vec_splat(m2->sm[3], 1), m1->sm[1], r3);

	r0 = vec_madd(vec_splat(m2->sm[0], 2), m1->sm[2], r0);
	r1 = vec_madd(vec_splat(m2->sm[1], 2), m1->sm[2], r1);
	r2 = vec_madd(vec_splat(m2->sm[2], 2), m1->sm[2], r2);
	r3 = vec_madd(vec_splat(m2->sm[3], 2), m1->sm[2], r3);

	r0 = vec_madd(vec_splat(m2->sm[0], 3), m1->sm[3], r0);
	r1 = vec_madd(vec_splat(m2->sm[1], 3), m1->sm[3], r1);
	r2 = vec_madd(vec_splat(m2->sm[2], 3), m1->sm[3], r2);
	r3 = vec_madd(vec_splat(m2->sm[3], 3), m1->sm[3], r3);

	dst->sm[0] = r0;
	dst->sm[1] = r1;
	dst->sm[2] = r2;
	dst->sm[3] = r3;

	return dst;
}

static inline struct mat4 *
m4_muls_simd(struct mat4 *dst, const struct mat4 *m, const float f)
{
	ALIGN(16) const float data[4] = { f, f, f, f };
	const vector float zero = (vector float)vec_splat_u32(0);
	const vector float scalar = vec_ld(0, data);

	dst->sm[0] = vec_madd(m->sm[0], scalar, zero);
	dst->sm[1] = vec_madd(m->sm[1], scalar, zero);
	dst->sm[2] = vec_madd(m->sm[2], scalar, zero);
	dst->sm[3] = vec_madd(m->sm[3], scalar, zero);

	return dst;
}

static inline struct mat4 *
m4_transpose_simd(struct mat4 *dst, const struct mat4 *src)
{
	vector float r0, r1, r2, r3;

	r0 = vec_mergeh(src->sm[0], src->sm[2]);
	r1 = vec_mergel(src->sm[0], src->sm[2]);
	r2 = vec_mergeh(src->sm[1], src->sm[3]);
	r3 = vec_mergel(src->sm[1], src->sm[3]);

	dst->sm[0] = vec_mergeh(r0, r2);
	dst->sm[1] = vec_mergel(r0, r2);
	dst->sm[2] = vec_mergeh(r1, r3);
	dst->sm[3] = vec_mergel(r1, r3);

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
	vector float v0, v1;
	const vector float zero = (vector float)vec_splat_u32(0);

	m4_transpose_simd(&tr, m);

	v0 = vec_madd(vec_splat(v->sv, 0), tr.sm[0], zero);
	v0 = vec_madd(vec_splat(v->sv, 1), tr.sm[1], v0);

	v1 = vec_madd(vec_splat(v->sv, 2), tr.sm[2], zero);
	v1 = vec_madd(vec_splat(v->sv, 3), tr.sm[3], v1);

	dst->sv = vec_add(v0, v1);

	return dst;
}

#endif

#endif /* _NE_MATH_ALTIVEC_MAT4_H_ */
