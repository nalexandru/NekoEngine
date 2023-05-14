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

#include NE_ATOMIC_HDR

NE_SYSTEM(RE_COLLECT_DRAWABLES, ECSYS_GROUP_MANUAL, 0, false, struct NeCollectDrawablesArgs, 2, NE_TRANSFORM, NE_MODEL_RENDER)
{
	struct NeTransform *xform = (struct NeTransform *)comp[0];
	struct NeModelRender *mr = (struct NeModelRender *)comp[1];
	struct NeModel *mdl = NULL;
	struct NeMatrix mvp{};

	const uint32_t array = atomic_fetch_add(&args->nextArray, 1);
	struct NeArray *drawables = &args->opaqueDrawableArrays[array];
	struct NeArray *blendedDrawables = &args->blendedDrawableArrays[array];
	struct NeArray *instances = &args->instanceArrays[array];

	mdl = (struct NeModel *)E_ResourcePtr(mr->model);
	if (!mdl)
		return;

	M_Store(&mvp, XMMatrixMultiply(M_Load(&xform->mat), M_Load(&args->vp)));

	atomic_fetch_add(&args->totalDrawables, mr->meshCount);

	struct NeBounds bounds{};
	M_XformBounds(&mr->bounds, &xform->mat, &bounds);
	if (!M_FrustumContainsBounds(&args->camFrustum, &bounds))
		return;

	uint32_t visibleMeshes = 0;
	for (uint32_t i = 0; i < mr->meshCount; ++i) {
		M_XformBounds(&mr->meshBounds[i], &xform->mat, &bounds);
		if (!M_FrustumContainsBounds(&args->camFrustum, &bounds))
			continue;

		struct NeDrawable *d = NULL;

		if (!mr->materials[i].alphaBlend) {
			d = (struct NeDrawable *)Rt_ArrayAllocate(drawables);
		} else {
			d = (struct NeDrawable *)Rt_ArrayAllocate(blendedDrawables);
			d->distance = M_Vector3Distance(M_Load(&args->camPos), M_Load(&xform->position));
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

		memcpy(&d->bounds, &bounds, sizeof(d->bounds));
		d->modelMatrix = &xform->mat;

		memcpy(&d->mvp, &mvp, sizeof(d->mvp));
		memcpy(&mi->mvp, &mvp, sizeof(mi->mvp));
		memcpy(&mi->model, &xform->mat, sizeof(mi->model));
		M_Store(&mi->normal, XMMatrixInverse(NULL, M_Load(&xform->mat)));

		mi->vertexAddress = Re_BufferAddress(mdl->gpu.vertexBuffer, sizeof(struct NeVertex) * mdl->meshes[i].vertexOffset);
		mi->materialAddress = Re_MaterialAddress(&mr->materials[i]);
	}

	atomic_fetch_add(&args->visibleDrawables, visibleMeshes);
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
