#include <Math/Math.h>
#include <Render/Render.h>
#include <Render/Model.h>
#include <Render/Systems.h>
#include <Render/Material.h>
#include <Render/Components/ModelRender.h>
#include <Engine/Resource.h>
#include <Engine/ECSystem.h>
#include <Scene/Transform.h>
#include <Scene/Components.h>
#include <System/Thread.h>

#include NE_ATOMIC_HDR

E_SYSTEM(RE_COLLECT_DRAWABLES, ECSYS_GROUP_MANUAL, 0, false, struct NeCollectDrawablesArgs, 2, TRANSFORM_COMP, MODEL_RENDER_COMP)
{
	struct NeTransform *xform = (struct NeTransform *)comp[0];
	struct NeModelRender *mr = (struct NeModelRender *)comp[1];
	struct NeModel *mdl = NULL;
	struct NeMatrix mvp {};

	const uint32_t array = atomic_fetch_add(&args->nextArray, 1);
	struct NeArray *drawables = &args->opaqueDrawableArrays[array];
	struct NeArray *blendedDrawables = &args->blendedDrawableArrays[array];
	struct NeArray *instances = &args->instanceArrays[array];

	mdl = (struct NeModel *)E_ResourcePtr(mr->model);
	if (!mdl)
		return;

//	if (!frustum_contains_aabb3(&args->camFrustum, &mdl->bounds.aabb))
//		return;

//	XMMATRIX tmp = XMMatrixMultiply(XMLoadFloat4x4A((XMFLOAT4X4A *)&xform->mat),
//		XMLoadFloat4x4A((XMFLOAT4X4A *)&Scn_activeCamera->viewMatrix));
//	tmp = XMMatrixMultiply(tmp, XMLoadFloat4x4A((XMFLOAT4X4A *)&Scn_activeCamera->projMatrix));

	M_Store(&mvp, XMMatrixMultiply(M_Load(&xform->mat), M_Load(&args->vp)));

	uint32_t visibleMeshes = 0;
	for (uint32_t i = 0; i < mdl->meshCount; ++i) {
		//struct NeBounds bounds {};
		
	/*	struct NeVec3 edge;
		M_Add(&edge, &mdl->meshes[i].bounds.sphere.center, M_MulVec3S(&edge, &M_Vec3PositiveX, mdl->meshes[i].bounds.sphere.radius));

		struct NeVec4 tv4;
		M_MulMatrixVec4(&tv4, &mvp,
				M_Vec4(&tv4,
					mdl->meshes[i].bounds.sphere.center.x,
					mdl->meshes[i].bounds.sphere.center.y,
					mdl->meshes[i].bounds.sphere.center.z, 1.f));
		M_Vec3(&bounds.sphere.center, tv4.x, tv4.y, tv4.z);

		M_MulMatrixVec4(&tv4, &mvp, M_Vec4(&tv4, edge.x, edge.y, edge.z, 1.f));
		M_Vec3(&edge, tv4.x, tv4.y, tv4.z);
		
		bounds.sphere.radius = M_Vec3Distance(&bounds.sphere.center, &edge);

		M_TransformAABB(&bounds.aabb, &mdl->meshes[i].bounds.aabb, &xform->mat);*/

//		if (!M_FrustumContainsSphere(&args->camFrustum, &bounds.sphere.center, bounds.sphere.radius))
//		if (!M_FrustumContainsAABB(&args->camFrustum, &mdl->meshes[i].bounds.aabb))
//			continue;

		++visibleMeshes;

		struct NeDrawable *d = NULL;

		if (!mr->materials[i].alphaBlend) {
			d = (struct NeDrawable *)Rt_ArrayAllocate(drawables);
		} else {
			d = (struct NeDrawable *)Rt_ArrayAllocate(blendedDrawables);
			d->distance = XMVectorGetX(
				XMVector3Length(
					XMVectorSubtract(
						XMLoadFloat3A((XMFLOAT3A *)&args->camPos), XMLoadFloat3A((XMFLOAT3A *)&xform->position))
				)
			);
		}

		d->instanceId = (uint32_t)instances->count;
		struct NeModelInstance *mi = (struct NeModelInstance *)Rt_ArrayAllocate(instances);

		d->vertexBuffer = mr->vertexBuffer;
		d->vertexOffset = sizeof(struct NeVertex) * mdl->meshes[i].vertexOffset;

		d->vertexAddress = Re_BufferAddress(mr->vertexBuffer, sizeof(struct NeVertex) * mdl->meshes[i].vertexOffset);
		d->indexBuffer = mdl->gpu.indexBuffer;
		d->indexType = mdl->indexType;

		d->vertexCount = mdl->meshes[i].vertexCount;
		d->firstIndex = mdl->meshes[i].indexOffset;
		d->indexCount = mdl->meshes[i].indexCount;

		d->material = &mr->materials[i];
		d->materialAddress = Re_MaterialAddress(&mr->materials[i]);

	//	M_TransformAABB(&d->bounds.aabb, &mdl->meshes[i].bounds.aabb, &xform->mat);
	//	d->bounds = &mdl->meshes[i].bounds;
		d->modelMatrix = &xform->mat;

		memcpy(&d->mvp, &mvp, sizeof(d->mvp));
		memcpy(&mi->mvp, &mvp, sizeof(mi->mvp));
		memcpy(&mi->model, &xform->mat, sizeof(mi->model));
		M_Store(&mi->normal, XMMatrixInverse(NULL, M_Load(&xform->mat)));

		mi->vertexAddress = Re_BufferAddress(mdl->gpu.vertexBuffer, sizeof(struct NeVertex) * mdl->meshes[i].vertexOffset);
		mi->materialAddress = Re_MaterialAddress(&mr->materials[i]);
	}

	atomic_fetch_add(&args->totalDrawables, mdl->meshCount);
	atomic_fetch_add(&args->visibleDrawables, visibleMeshes);
	//Sys_LogEntry("Culling", LOG_DEBUG, "%d visible out of %d", visibleMeshes, mdl->meshCount);
}

/* NekoEngine
 *
 * Systems.c
 * Author: Alexandru Naiman
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
