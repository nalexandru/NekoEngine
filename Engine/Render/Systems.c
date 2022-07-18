#include <stdatomic.h>

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

E_SYSTEM(RE_COLLECT_DRAWABLES, ECSYS_GROUP_MANUAL, 0, false, struct NeCollectDrawablesArgs, 2, TRANSFORM_COMP, MODEL_RENDER_COMP)
{
	struct NeTransform *xform = comp[0];
	struct NeModelRender *mr = comp[1];
	struct NeModel *mdl = NULL;
	struct NeMatrix mvp;

	const uint32_t array = atomic_fetch_add(&args->nextArray, 1);
	struct NeArray *drawables = &args->opaqueDrawableArrays[array];
	struct NeArray *blendedDrawables = &args->blendedDrawableArrays[array];
	struct NeArray *instances = &args->instanceArrays[array];

	mdl = E_ResourcePtr(mr->model);
	if (!mdl)
		return;

//	if (!frustum_contains_aabb3(&args->camFrustum, &mdl->bounds.aabb))
//		return;

	M_MulMatrix(&mvp, &args->vp, &xform->mat);

	uint32_t visibleMeshes = 0;
	for (uint32_t i = 0; i < mdl->meshCount; ++i) {
		struct NeBounds bounds;

		struct NeVec3 edge;
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

//		if (!M_FrustumContainsSphere(&args->camFrustum, &bounds.sphere.center, bounds.sphere.radius))
//		if (!M_FrustumContainsAABB(&args->camFrustum, &mdl->meshes[i].bounds.aabb))
//			continue;

		++visibleMeshes;

		struct NeDrawable *d = NULL;

		if (!mr->materials[i].alphaBlend) {
			d = Rt_ArrayAllocate(drawables);
		} else {
			d = Rt_ArrayAllocate(blendedDrawables);
			d->distance = M_Vec3Distance(&args->camPos, &xform->position);
		}

		d->instanceId = (uint32_t)instances->count;
		struct NeModelInstance *mi = Rt_ArrayAllocate(instances);

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

		d->bounds = &mdl->meshes[i].bounds;
		d->modelMatrix = &xform->mat;

		M_Copy(&d->mvp, &mvp);

		M_Copy(&mi->mvp, &mvp);
		M_Copy(&mi->model, &xform->mat);
		M_Copy(&mi->normal, M_InverseMatrix(&mi->normal, &xform->mat));

		mi->vertexAddress = Re_BufferAddress(mdl->gpu.vertexBuffer, sizeof(struct NeVertex) * mdl->meshes[i].vertexOffset);
		mi->materialAddress = Re_MaterialAddress(&mr->materials[i]);
	}

	atomic_fetch_add(&args->totalDrawables, mdl->meshCount);
	atomic_fetch_add(&args->visibleDrawables, visibleMeshes);
//	Sys_LogEntry("Culling", LOG_DEBUG, "%d visible out of %d", visibleMeshes, mdl->meshCount);
}
