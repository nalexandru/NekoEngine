/* NekoEngine
 *
 * mat4_vmx128.h
 * Author: Alexandru Naiman
 *
 * 4x4 matrix functions - VMX128 implementation
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

#ifndef _NE_MATH_VMX128_MAT4_H_
#define _NE_MATH_VMX128_MAT4_H_

#include <Math/defs.h>

#ifdef USE_VMX128

#include <Math/vec3.h>
#include <Math/quat.h>

#define M4_INVERSE_NOSIMD

static inline struct mat4 *
m4_simd(struct mat4 *dst, const float *m)
{
	NE_ALIGN(16) float data[4];

	data[0] = m[0]; data[1] = m[1]; data[2] = m[2]; data[3] = m[3];
	dst->sm[0] = __lvx(data, 0);

	data[0] = m[4]; data[1] = m[5]; data[2] = m[6]; data[3] = m[7];
	dst->sm[1] = __lvx(data, 0);

	data[0] = m[8]; data[1] = m[9]; data[2] = m[10]; data[3] = m[11];
	dst->sm[2] = __lvx(data, 0);

	data[0] = m[12]; data[1] = m[13]; data[2] = m[14]; data[3] = m[15];
	dst->sm[3] = __lvx(data, 0);

	return dst;
}

static inline struct mat4 *
m4f_simd(struct mat4 *dst,
	float m0, float m1, float m2, float m3,
	float m4, float m5, float m6, float m7,
	float m8, float m9, float m10, float m11,
	float m12, float m13, float m14, float m15)
{
	NE_ALIGN(16) float data[4];

	data[0] = m0; data[1] = m1; data[2] = m2; data[3] = m3;
	dst->sm[0] = __lvx(data, 0);

	data[0] = m4; data[1] = m5; data[2] = m6; data[3] = m7;
	dst->sm[1] = __lvx(data, 0);

	data[0] = m8; data[1] = m9; data[2] = m10; data[3] = m11;
	dst->sm[2] = __lvx(data, 0);

	data[0] = m12; data[1] = m13; data[2] = m14; data[3] = m15;
	dst->sm[3] = __lvx(data, 0);

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
	__vector4 r0, r1, r2, r3;

	// TODO: profile instruction order
    r0 = __vmulfp(__vspltw(m2->sm[0], 0), m1->sm[0]);
	r1 = __vmulfp(__vspltw(m2->sm[1], 0), m1->sm[0]);
	r2 = __vmulfp(__vspltw(m2->sm[2], 0), m1->sm[0]);
	r3 = __vmulfp(__vspltw(m2->sm[3], 0), m1->sm[0]);
    
	r0 = __vmaddfp(__vspltw(m2->sm[0], 1), m1->sm[1], r0);
	r1 = __vmaddfp(__vspltw(m2->sm[1], 1), m1->sm[1], r1);
	r2 = __vmaddfp(__vspltw(m2->sm[2], 1), m1->sm[1], r2);
	r3 = __vmaddfp(__vspltw(m2->sm[3], 1), m1->sm[1], r3);
    
	r0 = __vmaddfp(__vspltw(m2->sm[0], 2), m1->sm[2], r0);
	r1 = __vmaddfp(__vspltw(m2->sm[1], 2), m1->sm[2], r1);
	r2 = __vmaddfp(__vspltw(m2->sm[2], 2), m1->sm[2], r2);
	r3 = __vmaddfp(__vspltw(m2->sm[3], 2), m1->sm[2], r3);
    
	r0 = __vmaddfp(__vspltw(m2->sm[0], 3), m1->sm[3], r0);
	r1 = __vmaddfp(__vspltw(m2->sm[1], 3), m1->sm[3], r1);
    r2 = __vmaddfp(__vspltw(m2->sm[2], 3), m1->sm[3], r2);
    r3 = __vmaddfp(__vspltw(m2->sm[3], 3), m1->sm[3], r3);
	
	dst->sm[0] = r0;
	dst->sm[1] = r1;
	dst->sm[2] = r2;
	dst->sm[3] = r3;
	
	return dst;
}

static inline struct mat4 *
m4_muls_simd(struct mat4 *dst, const struct mat4 *m, const float f)
{
	NE_ALIGN(16) const float data[4] = { f, f, f, f };
	const __vector4 scalar = __lvx(data, 0);
	
	dst->sm[0] = __vmulfp(m->sm[0], scalar);
	dst->sm[1] = __vmulfp(m->sm[1], scalar);
	dst->sm[2] = __vmulfp(m->sm[2], scalar);
	dst->sm[3] = __vmulfp(m->sm[3], scalar);

	return dst;
}

static inline struct mat4 *
m4_transpose_simd(struct mat4 *dst, const struct mat4 *src)
{
	__vector4 r0, r1, r2, r3;
	
	r0 = __vmrghw(src->sm[0], src->sm[2]);
	r1 = __vmrglw(src->sm[0], src->sm[2]);
	r2 = __vmrghw(src->sm[1], src->sm[3]);
	r3 = __vmrglw(src->sm[1], src->sm[3]);
	
	dst->sm[0] = __vmrghw(r0, r2);
	dst->sm[1] = __vmrglw(r0, r2);
	dst->sm[2] = __vmrghw(r1, r3);
	dst->sm[3] = __vmrglw(r1, r3);

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
	__vector4 v0, v1;

	m4_transpose_simd(&tr, m);

	v0 = __vmulfp(__vspltw(v->sv, 0), tr.sm[0]);
	v0 = __vmaddfp(__vspltw(v->sv, 1), tr.sm[1], v0);

	v1 = __vmulfp(__vspltw(v->sv, 2), tr.sm[2]);
	v1 = __vmaddfp(__vspltw(v->sv, 3), tr.sm[3], v1);

	dst->sv = __vaddfp(v0, v1);

	return dst;
}

#endif

#endif /* _NE_MATH_VMX128_MAT4_H_ */
