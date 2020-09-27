/* NekoEngine
 *
 * defs.h
 * Author: Alexandru Naiman
 *
 * Math data structures and constants
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

#ifndef _NE_MATH_DEFS_H_
#define _NE_MATH_DEFS_H_

#ifdef __GNUC__
#	if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ <= 2)
#		define DISABLE_SIMD
#	endif
#endif

#ifndef DISABLE_SIMD
#if defined(__amd64__) || defined(_M_X64) || defined(_M_IX86)
#	include <xmmintrin.h>
#	if defined(__AVX__)
#		define USE_AVX
#		include <immintrin.h>
#	endif
#	define USE_SSE
#	define ALIGN	_Alignas(32)
#elif defined(__PPU__) || defined(__powerpc__)
#	ifndef __APPLE_ALTIVEC__
#		include <altivec.h>
#	endif
#	define USE_ALTIVEC
#elif defined(_M_ARM64)
#	define USE_NEON
#else
#	define ALIGN
#endif
#endif

#include <math.h>
#include <float.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#if __STDC_VERSION__ < 199901L && (!defined(_MSC_VER) || (_MSC_VER <= 1800))

// Mostly for VC6 compatibility

#ifndef fabsf
#	define fabsf		(float)fabs
#endif

#ifndef atan2f
#	define atan2f		(float)atan2
#endif

#ifndef cosf
#	define cosf			(float)cos
#endif

#ifndef sinf
#	define sinf			(float)sin
#endif

#ifndef acosf
#	define acosf		(float)acos
#endif

#ifndef asinf
#	define asinf		(float)asin
#endif

#ifndef tanf
#	define tanf			(float)tan
#endif

#ifndef sqrtf
#	define sqrtf		(float)sqrt
#endif

#ifndef copysignf
#	define copysignf	(float)_copysign
#endif

#if __cplusplus < 201103L

	static inline double fmax(double left, double right) { return (left > right) ? left : right; }
	static inline double fmin(double left, double right) { return (left < right) ? right : left; }
	static inline float fmaxf(float left, float right) { return (left > right) ? left : right; }
	static inline float fminf(float left, float right) { return (left < right) ? right : left; }

#endif

#endif

#pragma pack(push, 1)

struct vec2
{
	float x;
	float y;
};

struct vec3
{
	float x;
	float y;
	float z;
};

struct vec4
{
	union {
		struct {
			float x;
			float y;
			float z;
			float w;
		};
#if defined(USE_SSE)
		/*_Alignas(16) */__m128 sv;
#elif defined(USE_ALTIVEC)
		__vector float sv;
#elif defined (USE_NEON)
		float32x4_t sv;
#endif
	};
};

struct quat
{
	union {
		struct {
			float x;
			float y;
			float z;
			float w;
		};
#if defined(USE_SSE)
	/*	_Alignas(16)*/ __m128 sv;
#elif defined(USE_ALTIVEC)
		__vector float sv;
#elif defined(USE_NEON)
		float32x4_t sv;
#endif
	};
};

struct mat3
{
	float mat[9];
};

struct mat4
{
	union {
		float r[4][4];
		float m[16];
#if defined(USE_SSE)
		/*_Alignas(16)*/ __m128 sm[4];
#elif defined(USE_ALTIVEC)
		__vector float sm[4];
#elif defined(USE_NEON)
		float32x4_t sv[4];
#endif
	};
};

struct ray2
{
	struct vec2 start;
	struct vec2 dir;
};

struct ray3
{
	struct vec3 start;
	struct vec3 dir;
};

/*
 * A struture that represents an axis-aligned
 * bounding box.
 */
struct aabb2
{
	struct vec2 min; /** The max corner of the box */
	struct vec2 max; /** The min corner of the box */
};

/*
 * A struture that represents an axis-aligned
 * bounding box.
 */
struct aabb3
{
	struct vec3 min; /** The max corner of the box */
	struct vec3 max; /** The min corner of the box */
};

struct plane
{
	float a, b, c, d;
};

#pragma pack(pop)

#define M_PLANE_LEFT		0
#define M_PLANE_RIGHT		1
#define M_PLANE_BOTTOM		2
#define M_PLANE_TOP			3
#define M_PLANE_NEAR		4
#define M_PLANE_FAR			5

#define M_CONTAINS_NONE		0
#define M_CONTAINS_PARTIAL	1
#define M_CONTAINS_ALL		2

#define PI					 3.14159265358979323846f
#define M_PI_180			 0.01745329251994329576f
#define M_180_PI			57.29577951308232087684f

#endif /* _NE_MATH_DEFS_H_ */

