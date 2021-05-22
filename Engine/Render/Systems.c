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

	struct Array *drawables = &args->arrays[atomic_fetch_add(&args->nextArray, 1)];
	mdl = E_ResourcePtr(mr->model);
	if (!mdl)
		return;

	m4_mul(&mvp, &args->vp, &xform->mat);

	for (uint32_t i = 0; i < mdl->meshCount; ++i) {
		// TODO: visibility

		struct Drawable *d = Rt_ArrayAllocate(drawables);

		d->vertexBuffer = mdl->gpu.vertexBuffer;
		d->indexBuffer = mdl->gpu.indexBuffer;
		d->indexType = mdl->indexType;

		d->vertexCount = mdl->meshes[i].vertexCount;
		d->firstIndex = mdl->meshes[i].indexOffset;
		d->indexCount = mdl->meshes[i].indexCount;

		d->material = &mr->materials[i];
		d->materialAddress = Re_MaterialAddress(&mr->materials[i]);

		m4_copy(&d->mvp, &mvp);
	}
}
