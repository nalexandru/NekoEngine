#ifndef _NE_MATH_MATH_H_
#define _NE_MATH_MATH_H_

#include <Math/defs.h>

#include <Math/func.h>

#include <Math/vec2.h>
#include <Math/vec3.h>
#include <Math/vec4.h>

#include <Math/quat.h>

#include <Math/mat3.h>
#include <Math/mat4.h>

#include <Math/ray.h>

#include <Math/aabb.h>

#include <Math/plane.h>

#include <Math/frustum.h>

#include <Math/debug.h>

// Generics
#define M_Normalize(x, y) _Generic((x),	\
	struct NeVec2 *: M_NormalizeVec2,	\
	struct NeVec3 *: M_NormalizeVec3,	\
	struct NeVec4 *: M_NormalizeVec4,	\
	struct NeQuat *: M_NormalizeQuat,	\
	struct NePlane *: M_NormalizePlane	\
)(x, y)

#define M_Add(x, y, z) _Generic((x),	\
	struct NeVec2 *: M_AddVec2,	\
	struct NeVec3 *: M_AddVec3,	\
	struct NeVec4 *: M_AddVec4,	\
	struct NeQuaternion *: M_AddQuat	\
)(x, y, z)

#define M_AddS(x, y, z) _Generic((x),	\
	struct NeVec2 *: M_AddVec2S,	\
	struct NeVec3 *: M_AddVec3S,	\
	struct NeVec4 *: M_AddVec4S		\
)(x, y, z)

#define M_Sub(x, y, z) _Generic((x),	\
	struct NeVec2 *: M_SubVec2,	\
	struct NeVec3 *: M_SubVec3,	\
	struct NeVec4 *: M_SubVec4,	\
	struct NeQuaternion *: M_SubQuat	\
)(x, y, z)

#define M_SubS(x, y, z) _Generic((x),	\
	struct NeVec2 *: M_SubVec2S,	\
	struct NeVec3 *: M_SubVec3S,	\
	struct NeVec4 *: M_SubVec4S		\
)(x, y, z)

#define M_Mul(x, y, z) _Generic((x),	\
	struct NeVec2 *: M_MulVec2,	\
	struct NeVec3 *: M_MulVec3,	\
	struct NeVec4 *: M_MulVec4,	\
	struct NeMatrix *: M_MulMatrix,	\
	struct NeQuaternion *: M_MulQuat	\
)(x, y, z)

#define M_MulVecS(x, y, z) _Generic((x),	\
	struct NeVec2 *: M_MulVec2S,	\
	struct NeVec3 *: M_MulVec3S,	\
	struct NeVec4 *: M_MulVec4S		\
)(x, y, z)

#define M_Div(x, y, z) _Generic((x),	\
	struct NeVec2 *: M_DivVec2,	\
	struct NeVec3 *: M_DivVec3,	\
	struct NeVec4 *: M_DivVec4,	\
	struct NeQuaternion *: M_DivQuat	\
)(x, y, z)

#define M_DivS(x, y, z) _Generic((x),	\
	struct NeVec2 *: M_DivVec2S,	\
	struct NeVec3 *: M_DivVec3S,	\
	struct NeVec4 *: M_DivVec4S		\
)(x, y, z)

#define M_Cross(x, y, z) _Generic((x),	\
	struct NeVec2 *: M_CrossVec2,	\
	struct NeVec3 *: M_CrossVec3	\
)(x, y, z)

#define M_Dot(x, y, z) _Generic((x),	\
	struct NeVec2 *: M_DotVec2,	\
	struct NeVec3 *: M_DotVec3,	\
	struct NeVec4 *: M_DotVec4,	\
	struct NeQuaternion *: M_DotQuat	\
)(x, y, z)

#define M_Copy(x, y) _Generic((x),	\
	struct NeVec2 *: M_CopyVec2,	\
	struct NeVec3 *: M_CopyVec3,	\
	struct NeVec4 *: M_CopyVec4,	\
	struct NeQuaternion *: M_CopyQuat,	\
	struct NeMatrix *: M_CopyMatrix	\
)(x, y)

#define M_Fill(x, y) _Generic((x),	\
	struct NeVec2 *: M_FillVec2,	\
	struct NeVec3 *: M_FillVec3,	\
	struct NeVec4 *: M_FillVec4		\
)(x, y)

#define M_Identity(x) _Generic((x),	\
	struct NeQuaternion *: M_QuatIdentity,	\
	struct NeMatrix *: M_MatrixIdentity	\
)(x)

#define M_Clamp(x, y, z) _Generic((x), \
	float: M_ClampF,                   \
	int32_t: M_ClampI,                 \
	uint32_t: M_ClampUI                \
)(x, y, z)

#define M_Min(x, y) _Generic((x),	\
	struct NeVec2 *: M_FillVec2,	\
	struct NeVec3 *: M_FillVec3,	\
	struct NeVec4 *: M_FillVec4		\
)(x, y)

#endif /* _NE_MATH_MATH_H_ */

/* NekoEngine
 *
 * Math.h
 * Author: Alexandru Naiman
 *
 * Math library
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
