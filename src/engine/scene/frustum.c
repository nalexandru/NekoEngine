/* NekoEngine
 *
 * frustum.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Frustum
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
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

#include <scene/frustum.h>

void
frustum_from_vp(
	ne_frustum *f,
	kmMat4 *mat)
{
	float len = 0.f;
	kmVec3 tmp;

	f->planes[0].normal.x = mat->mat[3] + mat->mat[0];
	f->planes[0].normal.y = mat->mat[7] + mat->mat[4];
	f->planes[0].normal.z = mat->mat[11] + mat->mat[8];
	f->planes[0].distance = mat->mat[15] + mat->mat[12];

	f->planes[1].normal.x = mat->mat[3] - mat->mat[0];
	f->planes[1].normal.y = mat->mat[7] - mat->mat[4];
	f->planes[1].normal.z = mat->mat[11] - mat->mat[8];
	f->planes[1].distance = mat->mat[15] - mat->mat[12];

	f->planes[2].normal.x = mat->mat[3] - mat->mat[1];
	f->planes[2].normal.y = mat->mat[7] - mat->mat[5];
	f->planes[2].normal.z = mat->mat[11] - mat->mat[9];
	f->planes[2].distance = mat->mat[15] - mat->mat[13];

	f->planes[3].normal.x = mat->mat[3] + mat->mat[1];
	f->planes[3].normal.y = mat->mat[7] + mat->mat[5];
	f->planes[3].normal.z = mat->mat[11] + mat->mat[9];
	f->planes[3].distance = mat->mat[15] + mat->mat[13];

	f->planes[4].normal.x = mat->mat[3] + mat->mat[2];
	f->planes[4].normal.y = mat->mat[7] + mat->mat[6];
	f->planes[4].normal.z = mat->mat[11] + mat->mat[10];
	f->planes[4].distance = mat->mat[15] + mat->mat[14];

	f->planes[5].normal.x = mat->mat[3] - mat->mat[2];
	f->planes[5].normal.y = mat->mat[7] - mat->mat[6];
	f->planes[5].normal.z = mat->mat[11] - mat->mat[10];
	f->planes[5].distance = mat->mat[15] - mat->mat[14];

	for (uint8_t i = 0; i < 6; ++i) {
		len = kmVec3Length(&f->planes[i].normal);
		kmVec3Fill(&tmp, len, len, len);
		kmVec3Div(&f->planes[i].normal, &f->planes[i].normal, &tmp);
		f->planes[i].distance /= len;
	}
}

static inline bool
_frustum_contains_sphere(
	ne_frustum *f,
	ne_bounding_sphere *s)
{
	float dist = 0.f;
	float rad = -s->radius;

	for (uint8_t i = 0; i < 6; ++i) {
		dist = kmVec3Dot(&f->planes[i].normal, &s->center)
			+ f->planes[i].distance;

		if (dist < rad)
			return false;
	}

	return true;
}

static inline bool
_frustum_contains_box(
	ne_frustum *f,
	kmAABB3 *b)
{
	/*
	
	uint8_t out = 0;

	for (uint8_t i = 0; i < 6; ++i)
	{
		out += glm::dot(_frustumPlanes[i].normal, box.GetMin()) < 0.f ? 1 : 0;
		out += glm::dot(_frustumPlanes[i].normal, glm::vec3(box.GetMax().x, box.GetMin().y, box.GetMin().z)) < 0.f ? 1 : 0;
		out += glm::dot(_frustumPlanes[i].normal, glm::vec3(box.GetMin().x, box.GetMax().y, box.GetMin().z)) < 0.f ? 1 : 0;
		out += glm::dot(_frustumPlanes[i].normal, glm::vec3(box.GetMax().x, box.GetMax().y, box.GetMin().z)) < 0.f ? 1 : 0;
		out += glm::dot(_frustumPlanes[i].normal, glm::vec3(box.GetMin().x, box.GetMin().y, box.GetMax().z)) < 0.f ? 1 : 0;
		out += glm::dot(_frustumPlanes[i].normal, glm::vec3(box.GetMax().x, box.GetMin().y, box.GetMax().z)) < 0.f ? 1 : 0;
		out += glm::dot(_frustumPlanes[i].normal, glm::vec3(box.GetMin().x, box.GetMax().y, box.GetMax().z)) < 0.f ? 1 : 0;
		out += glm::dot(_frustumPlanes[i].normal, glm::vec3(box.GetMax().x, box.GetMax().y, box.GetMax().z)) < 0.f ? 1 : 0;

		if (out == 8)
			return false;
	}
	
	*/

	return true;
}

bool
frustum_contains(
	ne_frustum *f,
	ne_bounds *b)
{
	if (b->have_sphere && !_frustum_contains_sphere(f, &b->sphere))
		return false;

	if (b->have_box && !_frustum_contains_box(f, &b->box))
		return false;

	return true;
}

