#ifndef NE_MATH_FRUSTUM_H
#define NE_MATH_FRUSTUM_H

#include <Math/Util.h>

static inline void
M_FrustumFromVP(struct NeFrustum *f, const struct NeMatrix *vp)
{
	XMMATRIX m = XMMatrixTranspose(M_Load(vp));

	M_Store(&f->planes[NE_FRUSTUM_TOP_PLANE], XMPlaneNormalize(XMVectorSubtract(m.r[3], m.r[1])));
	M_Store(&f->planes[NE_FRUSTUM_BOTTOM_PLANE], XMPlaneNormalize(XMVectorAdd(m.r[3], m.r[1])));
	M_Store(&f->planes[NE_FRUSTUM_LEFT_PLANE], XMPlaneNormalize(XMVectorAdd(m.r[3], m.r[0])));
	M_Store(&f->planes[NE_FRUSTUM_RIGHT_PLANE], XMPlaneNormalize(XMVectorSubtract(m.r[3], m.r[0])));
	M_Store(&f->planes[NE_FRUSTUM_NEAR_PLANE], XMPlaneNormalize(m.r[2]));
	M_Store(&f->planes[NE_FRUSTUM_FAR_PLANE], XMPlaneNormalize(XMVectorSubtract(m.r[3], m.r[2])));
}

static inline bool
M_FrustumContainsBox(const struct NeFrustum *f, const struct NeAABB *box)
{
	const XMVECTOR corners[] =
	{
		M_Load(&box->min),
		M_Load(&box->max),
		XMVectorSet(box->min.x, box->min.y, box->max.z, 1.f),
		XMVectorSet(box->min.x, box->max.y, box->min.z, 1.f),
		XMVectorSet(box->min.x, box->max.y, box->max.z, 1.f),
		XMVectorSet(box->max.x, box->max.y, box->min.z, 1.f),
		XMVectorSet(box->max.x, box->min.y, box->max.z, 1.f),
		XMVectorSet(box->max.x, box->min.y, box->min.z, 1.f)
	};

	for (int32_t i = 0; i < 4; ++i) {
		uint8_t out = 0;

		out += XMVectorGetX(XMPlaneDotCoord(M_Load(&f->planes[i]), corners[0])) < 1.f ? 1 : 0;
		out += XMVectorGetX(XMPlaneDotCoord(M_Load(&f->planes[i]), corners[1])) < 1.f ? 1 : 0;
		out += XMVectorGetX(XMPlaneDotCoord(M_Load(&f->planes[i]), corners[2])) < 1.f ? 1 : 0;
		out += XMVectorGetX(XMPlaneDotCoord(M_Load(&f->planes[i]), corners[3])) < 1.f ? 1 : 0;
		out += XMVectorGetX(XMPlaneDotCoord(M_Load(&f->planes[i]), corners[4])) < 1.f ? 1 : 0;
		out += XMVectorGetX(XMPlaneDotCoord(M_Load(&f->planes[i]), corners[5])) < 1.f ? 1 : 0;
		out += XMVectorGetX(XMPlaneDotCoord(M_Load(&f->planes[i]), corners[6])) < 1.f ? 1 : 0;
		out += XMVectorGetX(XMPlaneDotCoord(M_Load(&f->planes[i]), corners[7])) < 1.f ? 1 : 0;

		if (out == 8)
			return false;
	}

	return true;
}

static inline bool
M_FrustumContainsSphere(const struct NeFrustum *f, const struct NeVec3 *center, float radius)
{
	XMVECTOR c = M_Load(center);

	for (uint32_t i = 0; i < 4; ++i)
		if (XMVectorGetX(XMPlaneDotCoord(M_Load(&f->planes[i]), c)) < -radius)
			return false;

	return true;
}

static inline bool
M_FrustumContainsBounds(const struct NeFrustum *f, const struct NeBounds *b)
{
	if (!M_FrustumContainsSphere(f, &b->sphere.center, b->sphere.radius))
		return false;

	if (!M_FrustumContainsBox(f, &b->aabb))
		return false;

	return true;
}

#endif /* NE_MATH_FRUSTUM_H */

/* NekoEngine
 *
 * Frustum.h
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
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
