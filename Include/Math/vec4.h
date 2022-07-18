#ifndef _NE_MATH_VEC4_H_
#define _NE_MATH_VEC4_H_

#include <Math/defs.h>

#if defined(USE_SSE)
#	include <Math/sse/vec4_sse.h>
#elif defined(USE_ALTIVEC)
#	include <Math/altivec/vec4_altivec.h>
#elif defined(USE_NEON)
#	include <Math/neon/vec4_neon.h>
#elif defined(USE_VMX128)
#	include <Math/vmx128/vec4_vmx128.h>
#endif

#ifdef MATH_SIMD

#define M_Vec4		M_Vec4_SIMD
#define M_FillVec4			M_FillVec4_SIMD
#define M_CopyVec4			M_CopyVec4_SIMD
#define M_ZeroVec4			M_ZeroVec4_SIMD
#define M_AddVec4			M_AddVec4_SIMD
#define M_AddVec4S			M_AddVec4S_SIMD
#define M_SubVec4			M_SubVec4_SIMD
#define M_SubVec4S			M_SubVec4S_SIMD
#define M_MulVec4			M_MulVec4_SIMD
#define M_MulVec4S			M_MulVec4S_SIMD
#define M_DivVec4			M_DivVec4_SIMD
#define M_DivVec4S			M_DivVec4S_SIMD
#define M_DotVec4			M_DotVec4_SIMD

#ifndef V4_DISTANCE_NOSIMD
#define M_Vec4Distance		M_Vec4Distance_SIMD
#endif

#ifndef V4_SWAP_NOSIMD
#define M_SwapVec4			M_SwapVec4_SIMD
#endif

#ifndef V4_EQ_NOSIMD
#	define M_Vec4Equal		M_Vec4Equal_SIMD
#endif

#ifndef V4_LERP_NOSIMD
#	define M_LerpVec4		M_LerpVec4_SIMD
#endif

#else

static inline struct NeVec4 *
M_Vec4(struct NeVec4 *v, float x, float y, float z, float w)
{
	v->x = x; v->y = y; v->z = z; v->w = w;
	return v;
}

static inline struct NeVec4 *
M_FillVec4(struct NeVec4 *v, float f)
{
	v->x = v->y = v->z = v->w = f;
	return v;
}

static inline struct NeVec4 *
M_CopyVec4(struct NeVec4 *dst, const struct NeVec4 *src)
{
	dst->x = src->x; dst->y = src->y; dst->z = src->z; dst->w = src->w;
	return dst;
}

static inline struct NeVec4 *
M_ZeroVec4(struct NeVec4 *v)
{
	v->x = v->y = v->z = v->w = 0.f;
	return v;
}

/*
 * Adds 2 4D vectors together. The result is stored in dst, dst is returned.
 */
static inline struct NeVec4 *
M_AddVec4(struct NeVec4 *dst, const struct NeVec4 *v1, const struct NeVec4 *v2)
{
	dst->x = v1->x + v2->x;
	dst->y = v1->y + v2->y;
	dst->z = v1->z + v2->z;
	dst->w = v1->w + v2->w;

	return dst;
}

static inline struct NeVec4 *
M_AddVec4S(struct NeVec4 *dst, const struct NeVec4 *v1, const float s)
{
	dst->x = v1->x + s;
	dst->y = v1->y + s;
	dst->z = v1->z + s;
	dst->w = v1->w + s;

	return dst;
}

/*
 * Subtracts one 4D v2 from v1. The result is stored in dst. dst is returned
 */
static inline struct NeVec4 *
M_SubVec4(struct NeVec4 *dst, const struct NeVec4 *v1, const struct NeVec4 *v2)
{
	dst->x = v1->x - v2->x;
	dst->y = v1->y - v2->y;
	dst->z = v1->z - v2->z;
	dst->w = v1->w - v2->w;

	return dst;
}

static inline struct NeVec4 *
M_SubVec4S(struct NeVec4 *dst, const struct NeVec4 *v1, const float s)
{
	dst->x = v1->x - s;
	dst->y = v1->y - s;
	dst->z = v1->z - s;
	dst->w = v1->w - s;

	return dst;
}

static inline struct NeVec4 *
M_MulVec4(struct NeVec4 *dst, const struct NeVec4 *v1, const struct NeVec4 *v2)
{
	dst->x = v1->x * v2->x;
	dst->y = v1->y * v2->y;
	dst->z = v1->z * v2->z;
	dst->w = v1->w * v2->w;

	return dst;
}

static inline struct NeVec4 *
M_MulVec4S(struct NeVec4 *dst, const struct NeVec4 *v1, const float s)
{
	dst->x = v1->x * s;
	dst->y = v1->y * s;
	dst->z = v1->z * s;
	dst->w = v1->w * s;

	return dst;
}

static inline struct NeVec4 *
M_DivVec4(struct NeVec4 *dst, const struct NeVec4 *v1, const struct NeVec4 *v2)
{
	dst->x = v1->x / v2->x;
	dst->y = v1->y / v2->y;
	dst->z = v1->z / v2->z;
	dst->w = v1->w / v2->w;

	return dst;
}

static inline struct NeVec4 *
M_DivVec4S(struct NeVec4 *dst, const struct NeVec4 *v1, const float s)
{
	dst->x = v1->x / s;
	dst->y = v1->y / s;
	dst->z = v1->z / s;
	dst->w = v1->w / s;

	return dst;
}

/*
 * Returns the dot product of 2 4D vectors
 */
static inline float
M_DotVec4(const struct NeVec4 *v1, const struct NeVec4 *v2)
{
	return v1->x * v2->x +
			v1->y * v2->y +
			v1->z * v2->z +
			v1->w * v2->w;
}

#endif

#if !defined(MATH_SIMD) || defined(V4_LERP_NOSIMD)
static inline struct NeVec4 *
M_LerpVec4(struct NeVec4 *dst, const struct NeVec4 *v1, const struct NeVec4 *v2, float t)
{
	dst->x = v1->x + t * (v2->x - v1->x);
	dst->y = v1->y + t * (v2->y - v1->y);
	dst->z = v1->z + t * (v2->z - v1->z);
	dst->w = v1->w + t * (v2->w - v1->w);

	return dst;
}
#endif

#if !defined(MATH_SIMD) || defined(V4_EQ_NOSIMD)
static inline int
M_Vec4Equal(const struct NeVec4 *p1, const struct NeVec4 *p2)
{
	return  (p1->x < p2->x + FLT_EPSILON && p1->x > p2->x - FLT_EPSILON) &&
		(p1->y < p2->y + FLT_EPSILON && p1->y > p2->y - FLT_EPSILON) &&
		(p1->z < p2->z + FLT_EPSILON && p1->z > p2->z - FLT_EPSILON) &&
		(p1->w < p2->w + FLT_EPSILON && p1->w > p2->w - FLT_EPSILON);
}
#endif

#if !defined(MATH_SIMD) || defined(V4_SWAP_NOSIMD)
static inline void
M_SwapVec4(struct NeVec4 *a, struct NeVec4 *b)
{
	float x, y, z, w;
	x = a->x; a->x = b->x; b->x = x;
	y = a->y; a->y = b->y; b->y = y;
	z = a->z; a->z = b->z; b->z = z;
	w = a->w; a->w = b->w; b->w = w;
}
#endif

/*
 * Returns the length of the 4D vector squared.
 */
static inline float
M_Vec4LengthSquared(const struct NeVec4 *v)
{
	return M_DotVec4(v, v);
}

/*
 * Returns the length of a 4D vector, this uses a sqrt so if the
 * squared length will do use
 */
static inline float
M_Vec4Length(const struct NeVec4 *v)
{
	return sqrtf(M_Vec4LengthSquared(v));
}

/*
 * Normalizes a 4D vector. The result is stored in pOut. pOut is returned
 */
static inline struct NeVec4 *
M_NormalizeVec4(struct NeVec4 *dst, const struct NeVec4 *src)
{
	return M_MulVec4S(dst, src, 1.f / M_Vec4Length(src));
}

/*
 * Scales a vector to the required length. This performs a Normalize
 * before multiplying by S.
 */
static inline struct NeVec4 *
M_ScaleVec4(struct NeVec4 *dst, const struct NeVec4 *src, const float s)
{
	return M_MulVec4S(dst, M_NormalizeVec4(dst, src), s);
}

#if !defined(MATH_SIMD) || defined(V4_DISTANCE_NOSIMD)
static inline float
M_Vec4Distance(const struct NeVec4 *v1, const struct NeVec4 *v2)
{
	struct NeVec4 diff;
	M_SubVec4(&diff, v2, v1);
	return fabsf(M_Vec4Length(&diff));
}
#endif

#endif /* _NE_MATH_VEC4_H_ */

/* NekoEngine
 *
 * vec4.h
 * Author: Alexandru Naiman
 *
 * 4 component vector functions
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
 * Original copyright:

Copyright (c) 2008, Luke Benstead.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
