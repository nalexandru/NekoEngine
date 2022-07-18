#ifndef _NE_MATH_NEON_VEC4_H_
#define _NE_MATH_NEON_VEC4_H_

#include <Math/defs.h>

#ifdef USE_NEON

#include <arm_neon.h>

#define V4_EQ_NOSIMD
#define V4_SWAP_NOSIMD
#define V4_DISTANCE_NOSIMD

static inline struct NeVec4 *
M_Vec4_SIMD(struct NeVec4 *v, float x, float y, float z, float w)
{
	NE_ALIGN(16) float data[4] = { x, y, z, w };
	v->sv = vld1q_f32(data);
	return v;
}

static inline struct NeVec4 *
M_FillVec4_SIMD(struct NeVec4 *v, float f)
{
	v->sv = vdupq_n_f32(f);
	return v;
}

static inline struct NeVec4 *
M_CopyVec4_SIMD(struct NeVec4 *dst, const struct NeVec4 *src)
{
	dst->sv = vld1q_f32(&src->x);
	return dst;
}

static inline struct NeVec4 *
M_ZeroVec4_SIMD(struct NeVec4 *v)
{
	v->sv = vmovq_n_f32(0.f);
	return v;
}

static inline struct NeVec4 *
M_LerpVec4_SIMD(struct NeVec4 *dst, const struct NeVec4 *v1, const struct NeVec4 *v2, float t)
{
	const float32x4_t t2 = vdupq_n_f32(t);
	const float32x4_t t0 = vsubq_f32(v1->sv, v2->sv);
	dst-> sv = vmlaq_f32(v1->sv, t0, t2);
	return dst;
}

static inline struct NeVec4 *
M_AddVec4_SIMD(struct NeVec4 *dst, const struct NeVec4 *v1, const struct NeVec4 *v2)
{
	dst->sv = vaddq_f32(v1->sv, v2->sv);
	return dst;
}


static inline struct NeVec4 *
M_AddVec4S_SIMD(struct NeVec4 *dst, const struct NeVec4 *v1, const float s)
{
	float32x4_t tmp = vdupq_n_f32(s);
	dst->sv = vaddq_f32(v1->sv, tmp);
	return dst;
}

static inline struct NeVec4 *
M_SubVec4_SIMD(struct NeVec4 *dst, const struct NeVec4 *v1, const struct NeVec4 *v2)
{
	dst->sv = vsubq_f32(v1->sv, v2->sv);
	return dst;
}

static inline struct NeVec4 *
M_SubVec4S_SIMD(struct NeVec4 *dst, const struct NeVec4 *v1, const float s)
{
	float32x4_t tmp = vdupq_n_f32(s);
	dst->sv = vsubq_f32(v1->sv, tmp);
	return dst;
}

static inline struct NeVec4 *
M_MulVec4_SIMD(struct NeVec4 *dst, const struct NeVec4 *v1, const struct NeVec4 *v2)
{
	dst->sv = vmulq_f32(v1->sv, v2->sv);
	return dst;
}

static inline struct NeVec4 *
M_MulVec4S_SIMD(struct NeVec4 *dst, const struct NeVec4 *v1, const float s)
{
	dst->sv = vmulq_n_f32(v1->sv, s);
	return dst;
}

static inline struct NeVec4 *
M_DivVec4_SIMD(struct NeVec4 *dst, const struct NeVec4 *v1, const struct NeVec4 *v2)
{
	dst->sv = vdivq_f32(v1->sv, v2->sv);
	return dst;
}

static inline struct NeVec4 *
M_DivVec4S_SIMD(struct NeVec4 *dst, const struct NeVec4 *v1, const float s)
{
	float32x4_t tmp = vdupq_n_f32(s);
	dst->sv = vdivq_f32(v1->sv, tmp);
	return dst;
}

static inline float
M_DotVec4_SIMD(const struct NeVec4 *v1, const struct NeVec4 *v2)
{
	const float32x4_t tmp = vmulq_f32(v1->sv, v2->sv);
	float32x2_t t1 = vget_low_f32(tmp);
	float32x2_t t2 = vget_high_f32(tmp);
	t1 = vadd_f32(t1, t2);
	t1 = vpadd_f32(t1, t1);
	return vget_lane_f32(t1, 0);
}

#endif

#endif /* _NE_MATH_NEON_VEC4_H_ */

/* NekoEngine
 *
 * vec4_neon.h
 * Author: Alexandru Naiman
 *
 * 4 component vector functions NEON implementation
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
