#ifndef _NE_MATH_NEON_MAT4_H_
#define _NE_MATH_NEON_MAT4_H_

#include <Math/defs.h>

#ifdef USE_NEON

#include <Math/vec3.h>

#define M4_INVERSE_NOSIMD

static inline struct NeMatrix *
M_Matrix_SIMD(struct NeMatrix *dst, const float *m)
{
	dst->sm[0] = vld1q_f32(&m[0]);
	dst->sm[1] = vld1q_f32(&m[4]);
	dst->sm[2] = vld1q_f32(&m[8]);
	dst->sm[3] = vld1q_f32(&m[12]);
	return dst;
}

static inline struct NeMatrix *
M_MatrixF_SIMD(struct NeMatrix *dst,
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

static inline struct NeMatrix *
M_CopyMatrix_SIMD(struct NeMatrix *dst, const struct NeMatrix *src)
{
	dst->sm[0] = vld1q_f32(&src->r[0][0]);
	dst->sm[1] = vld1q_f32(&src->r[1][0]);
	dst->sm[2] = vld1q_f32(&src->r[2][0]);
	dst->sm[3] = vld1q_f32(&src->r[3][0]);

	return dst;
}

static inline struct NeMatrix *
M_MulMatrix_SIMD(struct NeMatrix *dst, const struct NeMatrix *m1, const struct NeMatrix *m2)
{
	float32x4_t zero = vmovq_n_f32(0.f);
	float32x4_t r0, r1, r2, r3;

	// TODO: profile instruction order
	r0 = vmlaq_f32(zero, vdupq_n_f32(m2->r[0][0]), m1->sm[0]);
	r1 = vmlaq_f32(zero, vdupq_n_f32(m2->r[1][0]), m1->sm[0]);
	r2 = vmlaq_f32(zero, vdupq_n_f32(m2->r[2][0]), m1->sm[0]);
	r3 = vmlaq_f32(zero, vdupq_n_f32(m2->r[3][0]), m1->sm[0]);

	r0 = vmlaq_f32(r0, vdupq_n_f32(m2->r[0][1]), m1->sm[1]);
	r1 = vmlaq_f32(r1, vdupq_n_f32(m2->r[1][1]), m1->sm[1]);
	r2 = vmlaq_f32(r2, vdupq_n_f32(m2->r[2][1]), m1->sm[1]);
	r3 = vmlaq_f32(r3, vdupq_n_f32(m2->r[3][1]), m1->sm[1]);
	
	r0 = vmlaq_f32(r0, vld1q_dup_f32(&m2->r[0][2]), m1->sm[2]);
	r1 = vmlaq_f32(r1, vld1q_dup_f32(&m2->r[1][2]), m1->sm[2]);
	r2 = vmlaq_f32(r2, vld1q_dup_f32(&m2->r[2][2]), m1->sm[2]);
	r3 = vmlaq_f32(r3, vld1q_dup_f32(&m2->r[3][2]), m1->sm[2]);
	
	r0 = vmlaq_f32(r0, vld1q_dup_f32(&m2->r[0][3]), m1->sm[3]);
	r1 = vmlaq_f32(r1, vld1q_dup_f32(&m2->r[1][3]), m1->sm[3]);
	r2 = vmlaq_f32(r2, vld1q_dup_f32(&m2->r[2][3]), m1->sm[3]);
	r3 = vmlaq_f32(r3, vld1q_dup_f32(&m2->r[3][3]), m1->sm[3]);
	
	dst->sm[0] = r0;
	dst->sm[1] = r1;
	dst->sm[2] = r2;
	dst->sm[3] = r3;
	
	return dst;
}

static inline struct NeMatrix *
M_MulMatrixS_SIMD(struct NeMatrix *dst, const struct NeMatrix *m, const float f)
{
	float32x4_t scalar = vdupq_n_f32(f);

	dst->sm[0] = vmulq_f32(m->sm[0], scalar);
	dst->sm[1] = vmulq_f32(m->sm[1], scalar);
	dst->sm[2] = vmulq_f32(m->sm[2], scalar);
	dst->sm[3] = vmulq_f32(m->sm[3], scalar);

	return dst;
}

static inline struct NeMatrix *
M_TransposeMatrix_SIMD(struct NeMatrix *dst, const struct NeMatrix *src)
{
	const float32x4_t r0 = vtrn1q_f32(src->sm[0], src->sm[1]);
	const float32x4_t r1 = vtrn2q_f32(src->sm[0], src->sm[1]);
	const float32x4_t r2 = vtrn1q_f32(src->sm[2], src->sm[3]);
	const float32x4_t r3 = vtrn2q_f32(src->sm[2], src->sm[3]);

	dst->sm[0] = vcombine_f32(vget_low_f32(r0), vget_low_f32(r2));
	dst->sm[1] = vcombine_f32(vget_low_f32(r1), vget_low_f32(r3));
	dst->sm[2] = vcombine_f32(vget_high_f32(r0), vget_high_f32(r2));
	dst->sm[3] = vcombine_f32(vget_high_f32(r1), vget_high_f32(r3));
	
	return dst;
}

static inline struct NeMatrix *
M_InverseMatrix_SIMD(struct NeMatrix *dst, const struct NeMatrix *src)
{
	(void)src; // TODO
	return dst;
}


static inline struct NeVec4 *
M_MulVec4MatrixSIMD(struct NeVec4 *dst, const struct NeVec4 *v, const struct NeMatrix *m)
{
	struct NeMatrix tr;
	float32x4_t v0, v1;

	M_TransposeMatrix_SIMD(&tr, m);

	v0 = vmulq_f32(vdupq_n_f32(v->x), tr.sm[0]);
	v0 = vmlaq_f32(v0, vdupq_n_f32(v->y), tr.sm[1]);
	v1 = vmulq_f32(vdupq_n_f32(v->z), tr.sm[2]);
	v1 = vmlaq_f32(v1, vdupq_n_f32(v->w), tr.sm[3]);

	dst->sv = vaddq_f32(v0, v1);

	return dst;
}

static inline struct NeVec4 *
M_MulMatrixVec4SIMD(struct NeVec4 *dst, const struct NeMatrix *m, const struct NeVec4 *v)
{
	// FIXME
	return M_MulVec4MatrixSIMD(dst, v, m);
}

#endif

#endif /* _NE_MATH_NEON_MAT4_H_ */

/* NekoEngine
 *
 * mat4_neon.h
 * Author: Alexandru Naiman
 *
 * 4x4 matrix functions NEON implementation
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
