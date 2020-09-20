#include <Render/Model.h>
#include <Render/Render.h>
#include <Render/Device.h>
#include <Render/Texture.h>
#include <Render/Material.h>
#include <Render/ModelRender.h>
#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <Scene/Transform.h>
#include <Engine/Resource.h>
#include <Engine/ECSystem.h>

#include "D3D9Render.h"

struct Drawable
{
	IDirect3DVertexBuffer9 *vtxBuffer;
	IDirect3DIndexBuffer9 *idxBuffer;
	UINT firstVertex, firstIndex, vertexCount, indexCount;
	struct MaterialInstance *mat;
	struct mat4 mvp;
};

struct GetDrawablesArgs
{
	Array *drawables;

};

const size_t Re_SceneRenderDataSize = sizeof(struct SceneRenderData);

bool
Re_InitScene(struct Scene *scene)
{
	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;
	return Rt_InitArray(&srd->drawables, 10, sizeof(struct Drawable));
}

void
Re_TermScene(struct Scene *scene)
{
	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;
	Rt_TermArray(&srd->drawables);
}

void
D3D9_RenderScene(struct Scene *scene)
{
	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;

	struct GetDrawablesArgs args;
	args.drawables = &srd->drawables;

	Rt_ClearArray(args.drawables, false);
	E_ExecuteSystemS(scene, GET_DRAWABLES_SYS, &args);

	IDirect3DVertexBuffer9 *vtxBuffer = NULL;
	IDirect3DIndexBuffer9 *idxBuffer = NULL;
	IDirect3DVertexShader9 *vtxShader = NULL;
	IDirect3DPixelShader9 *pixelShader = NULL;

	for (size_t i = 0; i < srd->drawables.count; ++i) {
		struct Drawable *draw = (struct Drawable *)Rt_ArrayGet(&srd->drawables, i);
		struct Shader *s = (struct Shader *)draw->mat->shader;[[]]

		if (vtxBuffer != draw->vtxBuffer) {
			Re_Device.dev->SetStreamSource(0, draw->vtxBuffer, 0, sizeof(struct Vertex));
			Re_Device.dev->SetVertexDeclaration(D3D9_VertexDeclaration);
			vtxBuffer = draw->vtxBuffer;
		}

		if (idxBuffer != draw->idxBuffer) {
			Re_Device.dev->SetIndices(draw->idxBuffer);
			idxBuffer = draw->idxBuffer;
		}

		if (vtxShader != s->vs) {
			Re_Device.dev->SetVertexShader(s->vs);
			vtxShader = s->vs;
		}

		if (pixelShader != s->ps) {
			Re_Device.dev->SetPixelShader(s->ps);
			pixelShader = s->ps;
		}

		if (draw->mat->textures[MAP_DIFFUSE] != E_INVALID_HANDLE) {
			struct TextureRenderData *trd = (struct TextureRenderData *)&((struct Texture *)E_ResourcePtr(draw->mat->textures[MAP_DIFFUSE]))->renderDataStart;
			Re_Device.dev->SetTexture(0, trd->tex);
		}

		Re_Device.dev->SetVertexShaderConstantF(0, draw->mvp.m, 4);

		Re_Device.dev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, draw->firstVertex, 0, draw->vertexCount, draw->firstIndex, draw->indexCount / 3);
	}
}

void
D3D9_GetDrawables(void **comp, struct GetDrawablesArgs *args)
{
	struct Transform *xform = (struct Transform *)comp[0];
	struct ModelRender *modelRender = (struct ModelRender *)comp[1];

	struct Model *model = (struct Model *)E_ResourcePtr(modelRender->model);
	struct ModelRenderData *mrd = (struct ModelRenderData *)&model->renderDataStart;
	
	struct mat4 mvp;
	m4_mul(&mvp, &Scn_ActiveCamera->projMatrix, &Scn_ActiveCamera->viewMatrix);
	m4_mul(&mvp, &mvp, &xform->mat);

	for (uint32_t i = 0; i < model->numMeshes; ++i) {
		struct Drawable *draw = (struct Drawable *)Rt_ArrayAllocate(args->drawables);

		m4_copy(&draw->mvp, &mvp);
		draw->vtxBuffer = mrd->vtxBuffer;
		draw->idxBuffer = mrd->idxBuffer;
		draw->firstIndex = model->meshes[i].firstIndex;
		draw->firstVertex = model->meshes[i].firstVertex;
		draw->vertexCount = model->meshes[i].vertexCount;
		draw->indexCount = model->meshes[i].indexCount;
		draw->mat = &model->materialInstances[i];
	}
}

