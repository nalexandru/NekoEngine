#ifndef _NE_MATH_MATH_H_
#define _NE_MATH_MATH_H_

#include <DirectXMath.h>
#include <Math/Types.h>

using namespace DirectX;

template<typename T>
static inline T
M_Clamp(T x, T min, T max)
{
	return x < min ? min : (x > max ? max : x);
}

template<typename T>
static inline T
M_Max(T a, T b)
{
	return a > b ? a : b;
}

template<typename T>
static inline T
M_Min(T a, T b)
{
	return a < b ? a : b;
}

template<typename T>
static inline T
M_Lerp(T x, T y, T t)
{
	return x + t * (y - x);
}

template<typename T>
static inline T
M_Mod(T x, T y)
{
	return x - y * floor(x / y);
}

static inline bool
M_FloatEqual(float lhs, float rhs)
{
	return (fabsf(lhs - rhs) <= FLT_EPSILON * fmaxf(1.f, fmaxf(lhs, rhs)));
}

static inline struct NeMatrix *
M_InfinitePerspectiveMatrixRZ(struct NeMatrix *dst, float fov_y, float aspect, float z_near)
{
	const float rad = 0.5f * XMConvertToRadians(fov_y / 2.f);
	const float h = cosf(rad) / sinf(rad);

	float m[16] = {
		h / aspect, 0.f, 0.f,  0.f,
		0.f,   h, 0.f,  0.f,
		0.f, 0.f, 0.f, 1.f,
		0.f, 0.f, z_near,  0.f
	};

	XMStoreFloat4x4A((XMFLOAT4X4A *)dst, XMLoadFloat4x4((XMFLOAT4X4 *)m));
	return dst;
}

// Load/Store

static inline XMVECTOR
M_Load(const struct NeVec2 *v) noexcept
{
	return XMLoadFloat2A((XMFLOAT2A *)v);
}

static inline XMVECTOR
M_Load(const struct NeVec3 *v) noexcept
{
	return XMLoadFloat3A((XMFLOAT3A *)v);
}

static inline XMVECTOR
M_Load(const struct NeVec4 *v) noexcept
{
	return XMLoadFloat4A((XMFLOAT4A *)v);
}

static inline XMVECTOR
M_Load(const struct NeQuaternion *v) noexcept
{
	return XMLoadFloat4A((XMFLOAT4A *)v);
}

static inline XMMATRIX
M_Load(const struct NeMatrix *m) noexcept
{
	return XMLoadFloat4x4A((XMFLOAT4X4A *)m);
}

static inline void
M_Store(struct NeVec2 *d, XMVECTOR v)
{
	XMStoreFloat2A((XMFLOAT2A *)d, v);
}

static inline void
M_Store(struct NeVec3 *d, XMVECTOR v)
{
	XMStoreFloat3A((XMFLOAT3A *)d, v);
}

static inline void
M_Store(struct NeVec4 *d, XMVECTOR v)
{
	XMStoreFloat4A((XMFLOAT4A *)d, v);
}

static inline void
M_Store(struct NeQuaternion *d, XMVECTOR v)
{
	XMStoreFloat4A((XMFLOAT4A *)d, v);
}

static inline void
M_Store(struct NeMatrix *d, XMMATRIX v)
{
	XMStoreFloat4x4A((XMFLOAT4X4A *)d, v);
}

// Quaternion

static inline float
M_QuatRoll(const struct NeQuaternion *q)
{
	const struct NeVec2 v =
	{
		2.f * (q->x * q->y + q->w * q->z),
		q->w * q->w + q->x * q->x - q->y * q->y - q->z * q->z
	};

	if (XMVectorGetX(XMVectorEqual(M_Load(&v), XMVectorZero())))
		return 0.f;

	return XMConvertToDegrees(atan2f(v.x, v.y));
}

static inline float
M_QuatPitch(const struct NeQuaternion *q)
{
	const struct NeVec2 v =
	{
		2.f * (q->y * q->z + q->w * q->x),
		q->w * q->w - q->x * q->x - q->y * q->y + q->z * q->z
	};

	if (XMVectorGetX(XMVectorEqual(M_Load(&v), XMVectorZero())))
		return XMConvertToDegrees(2.f * atan2f(q->x, q->w));

	return XMConvertToDegrees(atan2f(v.x, v.y));
}

static inline float
M_QuatYaw(const struct NeQuaternion *q)
{
	return XMConvertToDegrees(asinf(M_Clamp(-2.f * (q->x * q->z - q->w * q->y), -1.f, 1.f)));
}

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
