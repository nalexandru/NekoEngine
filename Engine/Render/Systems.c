#include <stdatomic.h>

#include <Math/Math.h>
#include <Render/Render.h>
#include <Render/Model.h>
#include <Render/Systems.h>
#include <Render/Material.h>
#include <Render/Components/ModelRender.h>
#include <Engine/Resource.h>
#include <Scene/Transform.h>
#include <System/Thread.h>

void
Re_CollectDrawables(void **comp, struct CollectDrawablesArgs *args)
{
	struct Transform *xform = comp[0];
	struct ModelRender *mr = comp[1];
	struct Model *mdl = NULL;
	struct mat4 mvp;

	// TODO: visibility
	const uint32_t array = atomic_fetch_add(&args->nextArray, 1);
	struct Array *drawables = &args->opaqueDrawableArrays[array];
	struct Array *blendedDrawables = &args->blendedDrawableArrays[array];
	struct Array *instances = &args->instanceArrays[array];

	mdl = E_ResourcePtr(mr->model);
	if (!mdl)
		return;

	m4_mul(&mvp, &args->vp, &xform->mat);

	for (uint32_t i = 0; i < mdl->meshCount; ++i) {
		// TODO: visibility

		struct Drawable *d = NULL;

		if (!mr->materials[i].alphaBlend) {
			d = Rt_ArrayAllocate(drawables);
		} else {
			d = Rt_ArrayAllocate(blendedDrawables);
			d->distance = v3_distance(&args->camPos, &xform->position);
		}

		d->instanceId = (uint32_t)instances->count;
		struct ModelInstance *mi = Rt_ArrayAllocate(instances);

		d->vertexAddress = Re_BufferAddress(mdl->gpu.vertexBuffer, sizeof(struct Vertex) * mdl->meshes[i].vertexOffset);
		d->indexBuffer = mdl->gpu.indexBuffer;
		d->indexType = mdl->indexType;

		d->vertexCount = mdl->meshes[i].vertexCount;
		d->firstIndex = mdl->meshes[i].indexOffset;
		d->indexCount = mdl->meshes[i].indexCount;

		d->material = &mr->materials[i];
		d->materialAddress = Re_MaterialAddress(&mr->materials[i]);

		m4_copy(&d->mvp, &mvp);

		m4_copy(&mi->mvp, &mvp);
		m4_copy(&mi->model, &xform->mat);
		m4_transpose(&mi->normal, m4_inverse(&mi->normal, &xform->mat));

		mi->vertexAddress = Re_BufferAddress(mdl->gpu.vertexBuffer, sizeof(struct Vertex) * mdl->meshes[i].vertexOffset);
		mi->materialAddress = Re_MaterialAddress(&mr->materials[i]);
	}
}
