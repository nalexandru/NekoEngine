#include <Render/Model.h>
#include <Render/Device.h>

#include "D3D9Render.h"

const size_t Re_ModelRenderDataSize = sizeof(struct ModelRenderData);

#ifdef _XBOX
#	define D3DLOCK_DISCARD	0
#endif

bool
Re_InitModel(const char *name, struct Model *m)
{
	struct ModelRenderData *mrd = (struct ModelRenderData *)&m->renderDataStart;

	UINT vertexSize = sizeof(struct Vertex) * m->numVertices;
	UINT indexSize = sizeof(uint32_t) * m->numIndices;

	HRESULT hr;
	void *dst;

	hr = Re_Device.dev->CreateVertexBuffer(vertexSize, D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &mrd->vtxBuffer, NULL);
	mrd->vtxBuffer->Lock(0, 0, &dst, D3DLOCK_DISCARD);
	memcpy(dst, m->vertices, vertexSize);
	mrd->vtxBuffer->Unlock();

	hr = Re_Device.dev->CreateIndexBuffer(indexSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &mrd->idxBuffer, NULL);
	mrd->idxBuffer->Lock(0, 0, &dst, D3DLOCK_DISCARD);
	memcpy(dst, m->indices, indexSize);
	mrd->idxBuffer->Unlock();

	return true;
}

void
Re_TermModel(struct Model *m)
{
	struct ModelRenderData *mrd = (struct ModelRenderData *)&m->renderDataStart;
	
	mrd->vtxBuffer->Release();
	mrd->idxBuffer->Release();
}
