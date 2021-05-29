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

static THREAD_LOCAL size_t _transparentDrawableCount = 10;

void
Re_CollectDrawables(void **comp, struct CollectDrawablesArgs *args)
{
	struct Transform *xform = comp[0];
	struct ModelRender *mr = comp[1];
	struct Model *mdl = NULL;
	struct mat4 mvp;
	struct Array transparentDrawables;
	size_t usedTransparentDrawables = 0;

	Rt_InitArray(&transparentDrawables, _transparentDrawableCount, sizeof(struct Drawable), MH_Transient);

	// TODO: visibility
	struct Array *drawables = &args->arrays[atomic_fetch_add(&args->nextArray, 1)];
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
			++usedTransparentDrawables;
			
			if (transparentDrawables.count == transparentDrawables.size)
				continue;

			d = Rt_ArrayAllocate(&transparentDrawables);
		}

		d->vertexAddress = Re_BufferAddress(mdl->gpu.vertexBuffer, sizeof(struct Vertex) * mdl->meshes[i].vertexOffset);
		d->indexBuffer = mdl->gpu.indexBuffer;
		d->indexType = mdl->indexType;

		d->vertexCount = mdl->meshes[i].vertexCount;
		d->firstIndex = mdl->meshes[i].indexOffset;
		d->indexCount = mdl->meshes[i].indexCount;

		d->material = &mr->materials[i];
		d->materialAddress = Re_MaterialAddress(&mr->materials[i]);

		m4_copy(&d->mvp, &mvp);
	}

	if (transparentDrawables.count) {
		// TODO: sort transparent drawables
		Rt_ArrayAddArray(drawables, &transparentDrawables);
	}
}
