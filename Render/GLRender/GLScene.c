#include <Math/Math.h>
#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <Render/Model.h>
#include <Render/Render.h>
#include <Render/Device.h>
#include <Render/Texture.h>
#include <Render/Material.h>
#include <Render/ModelRender.h>
#include <Scene/Transform.h>
#include <Engine/Resource.h>
#include <Engine/ECSystem.h>

#include "GLRender.h"

struct Drawable
{
	struct mat4 mvp;
	GLuint vao;
	GLsizei count;
	void *indices;
	GLint baseVertex;
	struct MaterialInstance *mat;
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
GL_RenderScene(struct Scene *scene)
{
	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;
	GLuint vao = 0, program = 0;

	struct GetDrawablesArgs args;
	args.drawables = &srd->drawables;

	Rt_ClearArray(args.drawables, false);
	E_ExecuteSystemS(scene, GET_DRAWABLES_SYS, &args);

	for (size_t i = 0; i < srd->drawables.count; ++i) {
		struct Drawable *draw = (struct Drawable *)Rt_ArrayGet(&srd->drawables, i);
		struct Shader *s = draw->mat->shader;

		if (vao != draw->vao) {
			glBindVertexArray(draw->vao);
			vao = draw->vao;
		}

		if (program != s->program) {
			glUseProgram(s->program);
			program = s->program;
		}

		if (draw->mat->textures[0] != E_INVALID_HANDLE) {
			struct TextureRenderData *trd = (struct TextureRenderData *)&((struct Texture *)E_ResourcePtr(draw->mat->textures[0]))->renderDataStart;

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, trd->id);

			glUniform1i(1, 0);
		}

		glUniformMatrix4fv(0, 1, GL_FALSE, draw->mvp.m);

		glDrawElementsBaseVertex(GL_TRIANGLES, draw->count, GL_UNSIGNED_INT, draw->indices, draw->baseVertex);
	}

	glBindVertexArray(0);
	glUseProgram(0);
}

void
GL_GetDrawables(void **comp, struct GetDrawablesArgs *args)
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
		draw->vao = mrd->vao;
		draw->count = model->meshes[i].indexCount;
		draw->baseVertex = model->meshes[i].firstVertex;
		draw->indices = (void *)(((uintptr_t)model->meshes[i].firstIndex) * sizeof(uint32_t));
		draw->mat = &model->materialInstances[i];
	}
}
