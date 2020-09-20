/* NekoEngine
 *
 * func.h
 * Author: Alexandru Naiman
 *
 * Math shared functions
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

#ifndef _NE_MATH_FUNC_H_
#define _NE_MATH_FUNC_H_

#include <Math/defs.h>

/**
  * Returns degrees as radians.
  */
static inline float
deg_to_rad(float degrees)
{
	return degrees * M_PI_180;
}

/**
 * Returns radians as degrees
 */
static inline float
rad_to_deg(float radians)
{
	return radians * M_180_PI;
}

static inline bool
float_equal(float lhs, float rhs)
{
	return (fabsf(lhs - rhs) <= FLT_EPSILON * fmaxf(1.f, fmaxf(lhs, rhs)));
}

static inline float
clamp(float x, float min, float max)
{
	return x < min ? min : (x > max ? max : x);
}

static inline float
lerp(float x, float y, float t)
{
	return x + t * (y - x);
}

#endif /* _NE_MATH_FUNC_H_ */

