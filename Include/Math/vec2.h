#ifndef _NE_MATH_VEC2_H_
#define _NE_MATH_VEC2_H_

#include <Math/defs.h>

static inline struct NeVec2 *
M_Vec2(struct NeVec2 *v, float x, float y)
{
	v->x = x;
	v->y = y;

	return v;
}

static inline struct NeVec2 *
M_FillVec2(struct NeVec2 *v, float f)
{
	v->x = v->y = f;

	return v;
}

static inline struct NeVec2 *
M_CopyVec2(struct NeVec2 *dst, const struct NeVec2 *src)
{
	dst->x = src->x;
	dst->y = src->y;

	return dst;
}

static inline struct NeVec2 *
M_ZeroVec2(struct NeVec2 *v)
{
	v->x = 0.f;
	v->y = 0.f;
	
	return v;
}

static inline float
M_Vec2LengthSquared(const struct NeVec2 *v)
{
	return (v->x * v->x) + (v->y * v->y);
}

static inline float
M_Vec2Length(const struct NeVec2 *v)
{
	return sqrtf((v->x * v->x) + (v->y * v->y));
}

static inline struct NeVec2 *
M_NormalizeVec2(struct NeVec2 *dst, const struct NeVec2 *v)
{
	const float l = 1.0f / M_Vec2Length(v);

	dst->x = v->x * l;
	dst->y = v->y * l;

	return dst;
}

static inline struct NeVec2 *
M_LerpVec2(struct NeVec2 *dst, const struct NeVec2 *v1,
	const struct NeVec2 *v2, float t)
{
	dst->x = v1->x + t * (v2->x - v1->x);
	dst->y = v1->y + t * (v2->y - v1->y);

	return dst;
}

static inline struct NeVec2 *
M_AddVec2(struct NeVec2 *dst, const struct NeVec2 *v1, const struct NeVec2 *v2)
{
	dst->x = v1->x + v2->x;
	dst->y = v1->y + v2->y;

	return dst;
}

static inline struct NeVec2 *
M_AddVec2S(struct NeVec2 *dst, const struct NeVec2 *v1, const float s)
{
	dst->x = v1->x + s;
	dst->y = v1->y + s;

	return dst;
}

static inline struct NeVec2 *
M_SubVec2(struct NeVec2 *dst, const struct NeVec2 *v1, const struct NeVec2 *v2)
{
	dst->x = v1->x - v2->x;
	dst->y = v1->y - v2->y;

	return dst;
}

static inline struct NeVec2 *
M_SubVec2S(struct NeVec2 *dst, const struct NeVec2 *v1, const float s)
{
	dst->x = v1->x - s;
	dst->y = v1->y - s;

	return dst;
}

static inline struct NeVec2 *
M_MulVec2(struct NeVec2 *dst, const struct NeVec2 *v1, const struct NeVec2 *v2)
{
	dst->x = v1->x * v2->x;
	dst->y = v1->y * v2->y;

	return dst;
}

static inline struct NeVec2 *
M_MulVec2S(struct NeVec2 *dst, const struct NeVec2 *v1, const float s)
{
	dst->x = v1->x * s;
	dst->y = v1->y * s;

	return dst;
}

static inline struct NeVec2 *
M_DivVec2(struct NeVec2 *dst, const struct NeVec2 *v1, const struct NeVec2 *v2)
{
	dst->x = v1->x / v2->x;
	dst->y = v1->y / v2->y;

	return dst;
}

static inline struct NeVec2 *
M_DivVec2S(struct NeVec2 *dst, const struct NeVec2 *v1, const float s)
{
	dst->x = v1->x / s;
	dst->y = v1->y / s;

	return dst;
}

static inline float
M_DotVec2(const struct NeVec2 *v1, const struct NeVec2 *v2)
{
	return v1->x * v2->x + v1->y * v2->y;
}

static inline float
M_CrossVec2(const struct NeVec2 *v1, const struct NeVec2 *v2)
{
	return v1->x * v2->y - v1->y * v2->x;
}

static inline struct NeVec2 *
M_MulVec2M3(struct NeVec2 *dst, const struct NeVec2 *v, const struct NeMat3 *m)
{
	dst->x = v->x * m->mat[0] + v->y * m->mat[3] + m->mat[6];
	dst->y = v->x * m->mat[1] + v->y * m->mat[4] + m->mat[7];

	return dst;
}

static inline struct NeVec2 *
M_ScaleVec2(struct NeVec2 *dst, const struct NeVec2 *v, const float s)
{
	return M_MulVec2S(dst, M_NormalizeVec2(dst, v), s);
}

static inline bool
M_Vec2Equal(const struct NeVec2 *v1, const struct NeVec2 *v2)
{
	return M_FloatEqual(v1->x, v2->x) && M_FloatEqual(v1->y, v2->y);
}

static inline float
M_Vec2Angle(const struct NeVec2 *v1, const struct NeVec2 *v2)
{
	struct NeVec2 t1, t2;
	float cross;
	float dot;

	if (M_Vec2Equal(v1, v2))
		return 0.f;

	M_NormalizeVec2(&t1, v1);
	M_NormalizeVec2(&t2, v2);

	cross = M_CrossVec2(&t1, &t2);
	dot = M_DotVec2(&t1, &t2);

	/*
	 * acos is only defined for -1 to 1. Outside the range we
	 * get NaN even if that's just because of a floating point error
	 * so we clamp to the -1 - 1 range
	 */

	if (dot > 1.f) dot = 1.f;
	if (dot < -1.f) dot = -1.f;

	return M_RadToDeg(atan2f(cross, dot));
}

static inline float
M_Vec2Distance(const struct NeVec2 *v1, const struct NeVec2 *v2)
{
	struct NeVec2 diff;
	M_SubVec2(&diff, v2, v1);
	return fabsf(M_Vec2Length(&diff));
}

static inline struct NeVec2 *
M_Vec2Middle(struct NeVec2 *dst, const struct NeVec2 *v1, const struct NeVec2 *v2)
{
	struct NeVec2 sum;

	M_AddVec2(&sum, v1, v2);
	
	dst->x = sum.x / 2.f;
	dst->y = sum.y / 2.f;

	return dst;
}

static inline struct NeVec2 *
M_ReflectVec2(struct NeVec2 *dst, const struct NeVec2 *v, const struct NeVec2 *n)
{
	struct NeVec2 tmp;

	return M_SubVec2(dst, v, M_ScaleVec2(&tmp, n, 2.0f * M_DotVec2(v, n)));
}

static inline void
M_SwapVec2( struct NeVec2 *v1, struct NeVec2 *v2)
{
	float x = v1->x;
	float y = v1->y;
	
	v1->x = v2->x;
	v1->y = v2->y;

	v2->x = x;
	v2->y = y;
}

static const struct NeVec2 M_Vec2PositiveY = {  0.f,  1.f };
static const struct NeVec2 M_Vec2NegativeY = {  0.f, -1.f };
static const struct NeVec2 M_Vec2NegativeX = { -1.f,  0.f };
static const struct NeVec2 M_Vec2PositiveX = {  1.f,  0.f };

#endif /* _NE_MATH_VEC2_H_ */

/* NekoEngine
 *
 * vec2.h
 * Author: Alexandru Naiman
 *
 * 2 component vector
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
