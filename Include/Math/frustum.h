#ifndef _NE_MATH_FRUSTUM_H_
#define _NE_MATH_FRUSTUM_H_

#include <Math/defs.h>
#include <Math/mat4.h>
#include <Math/aabb.h>
#include <Math/plane.h>

#define FRUSTUM_PLANE_RIGHT		0
#define FRUSTUM_PLANE_LEFT		1
#define FRUSTUM_PLANE_TOP		2
#define FRUSTUM_PLANE_BOTTOM	3
#define FRUSTUM_PLANE_NEAR		4
#define FRUSTUM_PLANE_FAR		5

static inline void
M_FrustumFromVP(struct NeFrustum *f, const struct NeMatrix *view, const struct NeMatrix *proj)
{
	struct NeMatrix lvp, *vp = &lvp;
//	M_MulMatrix(&lvp, proj, M_TransposeMatrix(&lvp, view));

	M_TransposeMatrix(&lvp, proj);

	M_Plane(&f->planes[FRUSTUM_PLANE_LEFT],
		vp->r[3][0] + vp->r[0][0],
		vp->r[3][1] + vp->r[0][1],
		vp->r[3][2] + vp->r[0][2],
		vp->r[3][3] + vp->r[0][3]);

	M_Plane(&f->planes[FRUSTUM_PLANE_RIGHT],
		vp->r[3][0] - vp->r[0][0],
		vp->r[3][1] - vp->r[0][1],
		vp->r[3][2] - vp->r[0][2],
		vp->r[3][3] - vp->r[0][3]);

	M_Plane(&f->planes[FRUSTUM_PLANE_TOP],
		vp->r[3][0] - vp->r[1][0],
		vp->r[3][1] - vp->r[1][1],
		vp->r[3][2] - vp->r[1][2],
		vp->r[3][3] - vp->r[1][3]);

	M_Plane(&f->planes[FRUSTUM_PLANE_BOTTOM],
		vp->r[3][0] + vp->r[1][0],
		vp->r[3][1] + vp->r[1][1],
		vp->r[3][2] + vp->r[1][2],
		vp->r[3][3] + vp->r[1][3]);

	M_Plane(&f->planes[FRUSTUM_PLANE_NEAR],
		vp->r[3][0] + vp->r[2][0],
		vp->r[3][1] + vp->r[2][1],
		vp->r[3][2] + vp->r[2][2],
		vp->r[3][3] + vp->r[2][3]);

	M_Plane(&f->planes[FRUSTUM_PLANE_FAR],
		vp->r[3][0] - vp->r[2][0],
		vp->r[3][1] - vp->r[2][1],
		vp->r[3][2] - vp->r[2][2],
		vp->r[3][3] - vp->r[2][3]);

/*	M_Plane(&f->planes[FRUSTUM_PLANE_LEFT],
		vp->r[0][3] + vp->r[0][0],
		vp->r[1][3] + vp->r[1][0],
		vp->r[2][3] + vp->r[2][0],
		vp->r[3][3] + vp->r[3][0]);

	M_Plane(&f->planes[FRUSTUM_PLANE_RIGHT],
		vp->r[0][3] - vp->r[0][0],
		vp->r[1][3] - vp->r[1][0],
		vp->r[2][3] - vp->r[2][0],
		vp->r[3][3] - vp->r[3][0]);

	M_Plane(&f->planes[FRUSTUM_PLANE_TOP],
		vp->r[0][3] - vp->r[0][1],
		vp->r[1][3] - vp->r[1][1],
		vp->r[2][3] - vp->r[2][1],
		vp->r[3][3] - vp->r[3][1]);

	M_Plane(&f->planes[FRUSTUM_PLANE_BOTTOM],
		vp->r[0][3] + vp->r[0][1],
		vp->r[1][3] + vp->r[1][1],
		vp->r[2][3] + vp->r[2][1],
		vp->r[3][3] + vp->r[3][1]);

	M_Plane(&f->planes[FRUSTUM_PLANE_NEAR],
		vp->r[0][3] + vp->r[0][2],
		vp->r[1][3] + vp->r[1][2],
		vp->r[2][3] + vp->r[2][2],
		vp->r[3][3] + vp->r[3][2]);

	M_Plane(&f->planes[FRUSTUM_PLANE_FAR],
		vp->r[0][3] - vp->r[0][2],
		vp->r[1][3] - vp->r[1][2],
		vp->r[2][3] - vp->r[2][2],
		vp->r[3][3] - vp->r[3][2]);*/

//	f->planes[FRUSTUM_PLANE_NEAR].normal.z *= -1;

	for (uint32_t i = 0; i < sizeof(f->planes) / sizeof(f->planes[0]); ++i)
		M_NormalizePlane(&f->planes[i], &f->planes[i]);
}

static inline bool
M_FrustumContainsSphere(const struct NeFrustum *f, const struct NeVec3 *center, float radius)
{
	for (uint32_t i = 0; i < sizeof(f->planes) / sizeof(f->planes[0]); ++i)
		if (M_PlaneDistanceToPoint(&f->planes[i], center) < -radius)
			return false;
	return true;
}

static inline bool
M_FrustumContainsAABB(const struct NeFrustum *f, const struct NeAABB *box)
{
	uint32_t out = 0;

	for (uint32_t i = 0; i < (sizeof(f->planes) / sizeof(f->planes[0])) - 2; ++i) {
		struct NeVec3 v;

		out += M_DotVec3(&f->planes[i].normal, &box->min) < 0.f ? 1 : 0;
		out += M_DotVec3(&f->planes[i].normal, M_Vec3(&v, box->max.x, box->min.y, box->min.z)) < 0.f ? 1 : 0;
		out += M_DotVec3(&f->planes[i].normal, M_Vec3(&v, box->min.x, box->max.y, box->min.z)) < 0.f ? 1 : 0;
		out += M_DotVec3(&f->planes[i].normal, M_Vec3(&v, box->max.x, box->max.y, box->min.z)) < 0.f ? 1 : 0;
		out += M_DotVec3(&f->planes[i].normal, M_Vec3(&v, box->min.x, box->min.y, box->max.z)) < 0.f ? 1 : 0;
		out += M_DotVec3(&f->planes[i].normal, M_Vec3(&v, box->max.x, box->min.y, box->max.z)) < 0.f ? 1 : 0;
		out += M_DotVec3(&f->planes[i].normal, M_Vec3(&v, box->min.x, box->max.y, box->max.z)) < 0.f ? 1 : 0;
		out += M_DotVec3(&f->planes[i].normal, &box->max) < 0.f ? 1 : 0;

		if (out == 8)
			return false;
	}

	return true;
}

/*static inline bool
frustum_contains_bounds(const struct NeFrustum *f, )
{
	uint32_t out = 0;

	for (uint32_t i = 0; i < sizeof(f->planes) / sizeof(f->planes[0]); ++i) {
	}

	return false;
}*/

#endif /* _NE_MATH_FRUSTUM_H_ */

/* NekoEngine
 *
 * frustum.h
 * Author: Alexandru Naiman
 *
 * Frustum functions
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
