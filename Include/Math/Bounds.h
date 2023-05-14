#ifndef NE_MATH_BOUNDS_H
#define NE_MATH_BOUNDS_H

#include <Math/Util.h>

static inline void
M_XformBounds(const struct NeBounds *b, const struct NeMatrix *model, struct NeBounds *dst)
{
	const XMMATRIX m = M_Load(model);

	{ // AABB
		const XMVECTOR corners[] =
		{
			XMVector3Transform(M_Load(&b->aabb.min), m),
			XMVector3Transform(M_Load(&b->aabb.max), m),
			XMVector3Transform(XMVectorSet(b->aabb.min.x, b->aabb.min.y, b->aabb.max.z, 1.f), m),
			XMVector3Transform(XMVectorSet(b->aabb.min.x, b->aabb.max.y, b->aabb.min.z, 1.f), m),
			XMVector3Transform(XMVectorSet(b->aabb.min.x, b->aabb.max.y, b->aabb.max.z, 1.f), m),
			XMVector3Transform(XMVectorSet(b->aabb.max.x, b->aabb.max.y, b->aabb.min.z, 1.f), m),
			XMVector3Transform(XMVectorSet(b->aabb.max.x, b->aabb.min.y, b->aabb.max.z, 1.f), m),
			XMVector3Transform(XMVectorSet(b->aabb.max.x, b->aabb.min.y, b->aabb.min.z, 1.f), m)
		};

		XMVECTOR min = corners[0], max = corners[0];
		for (uint32_t i = 1; i < NE_ARRAY_SIZE(corners); ++i) {
			min = XMVectorMin(min, corners[i]);
			max = XMVectorMax(max, corners[i]);
		}

		M_Store(&dst->aabb.min, min);
		M_Store(&dst->aabb.max, max);
	}

	{ // Sphere
		XMVECTOR center = M_Load(&b->sphere.center);
		XMVECTOR edge = XMVectorAdd(center, XMVectorSet(b->sphere.radius, 0.f, 0.f, 1.f));

		center = XMVector3Transform(center, m);
		edge = XMVector3Transform(edge, m);

		M_Store(&dst->sphere.center, center);
		dst->sphere.radius = XMVectorGetX(XMVector3Length(XMVectorSubtract(center, edge)));
	}
}

#endif /* NE_MATH_BOUNDS_H */

/* NekoEngine
 *
 * Bounds.h
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
