#ifndef _NE_MATH_DEFS_H_
#define _NE_MATH_DEFS_H_

#include <System/System.h>
#include <Engine/Types.h>

#include <math.h>
#include <float.h>
#include <stddef.h>
#include <stdint.h>

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

NE_ALIGNED_STRUCT(NeVec2, 16,
	union {
		struct {
			float x;
			float y;
		};
		float v[2];
	};
);

NE_ALIGNED_STRUCT(NeVec3, 16,
	union {
		struct {
			float x;
			float y;
			float z;
		};
		float v[3];
	};
);

NE_ALIGNED_STRUCT(NeVec4, 16,
	union {
		struct {
			float x;
			float y;
			float z;
			float w;
		};
		float v[4];
	};
);

NE_ALIGNED_STRUCT(NeQuaternion, 16,
	union {
		struct {
			float x;
			float y;
			float z;
			float w;
		};
	};
);

NE_ALIGNED_STRUCT(NeMatrix, 16,
	union {
		float r[4][4];
		float m[16];
	};
);

NE_ALIGNED_STRUCT(NeAABB, 16,
	struct NeVec3 min;
	struct NeVec3 max;
);

NE_ALIGNED_STRUCT(NePlane, 16,
	struct NeVec3 normal;
	float distance;
);

NE_ALIGNED_STRUCT(NeFrustum, 16,
	struct NePlane planes[6];
);

#pragma pack(pop)

#define PI					 3.14159265358979323846f
#define M_PI_180			 0.01745329251994329576f
#define M_180_PI			57.29577951308232087684f

#endif /* _NE_MATH_DEFS_H_ */

/* NekoEngine
 *
 * Types.h
 * Author: Alexandru Naiman
 *
 * Math data structures and constants
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
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
