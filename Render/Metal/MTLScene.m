#import "MTLRender.h"

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

struct Drawable
{
	/*void *indices;
	struct MaterialInstance *mat;
	GLsizei count;
	struct mat4 mvp;
	GLint baseVertex;
	GLuint vao;
	GLenum indexType;*/
};

struct GetDrawablesArgs
{
	Array *drawables;
};

// Vertex Arrays

bool
MTL_InitScene(struct Scene *scene)
{
	return true;
//	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;
//	return Rt_InitArray(&srd->drawables, 10, sizeof(struct Drawable));
}

void
MTL_TermScene(struct Scene *scene)
{
//	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;
//	Rt_TermArray(&srd->drawables);
}

void
MTL_RenderSceneVAO(struct Scene *scene)
{
	/*size_t i = 0;
	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;
	GLuint vao = 0, program = 0;

	struct GetDrawablesArgs args;
	args.drawables = &srd->drawables;

	Rt_ClearArray(args.drawables, false);
	E_ExecuteSystemS(scene, GET_DRAWABLES_SYS, &args);
	
	if (!GL_ShaderSupport) {
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(Scn_ActiveCamera->projMatrix.m);
		glMatrixMode(GL_MODELVIEW);
	}
	
	for (i = 0; i < srd->drawables.count; ++i) {
		struct DrawableVAO *draw = (struct DrawableVAO *)Rt_ArrayGet(&srd->drawables, i);
		struct Shader *s = draw->mat->shader;

		if (vao != draw->vao) {
			glBindVertexArray(draw->vao);
			vao = draw->vao;
		}
		
		if (GL_ShaderSupport) {
			if (program != s->program) {
				glUseProgram(s->program);
				program = s->program;
				
				glUniform1i(1, 0);
			}
			
			glUniformMatrix4fv(0, 1, GL_FALSE, draw->mvp.m);
		} else {
			glLoadMatrixf(draw->mvp.m);
		}

		if (draw->mat->textures[0] != E_INVALID_HANDLE) {
			struct TextureRenderData *trd = (struct TextureRenderData *)&((struct Texture *)E_ResourcePtr(draw->mat->textures[0]))->renderDataStart;

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, trd->id);			
		}
		
		if (glDrawElementsBaseVertex)
			glDrawElementsBaseVertex(GL_TRIANGLES, draw->count, draw->indexType, draw->indices, draw->baseVertex);
		else
			glDrawElements(GL_TRIANGLES, draw->count, draw->indexType, draw->indices);
	}

	glBindVertexArray(0);

	if (GL_ShaderSupport)
		glUseProgram(0);*/
}

void
MTL_GetDrawables(void **comp, struct GetDrawablesArgs *args)
{
	/*uint32_t i = 0;
	struct Transform *xform = (struct Transform *)comp[0];
	struct ModelRender *modelRender = (struct ModelRender *)comp[1];

	struct Model *model = (struct Model *)E_ResourcePtr(modelRender->model);
	struct ModelRenderData *mrd = (struct ModelRenderData *)&model->renderDataStart;
	
	struct mat4 mvp;
	
	if (GL_ShaderSupport) {
		m4_mul(&mvp, &Scn_ActiveCamera->projMatrix, &Scn_ActiveCamera->viewMatrix);
		m4_mul(&mvp, &mvp, &xform->mat);
	} else {
		m4_mul(&mvp, &Scn_ActiveCamera->viewMatrix, &xform->mat);
	}
	
	for (i = 0; i < model->numMeshes; ++i) {
		struct DrawableVAO *draw = (struct DrawableVAO *)Rt_ArrayAllocate(args->drawables);

		m4_copy(&draw->mvp, &mvp);
		draw->vao = mrd->vao;
		draw->count = model->meshes[i].indexCount;
		draw->baseVertex = model->meshes[i].firstVertex;
		draw->indices = (void *)(((uintptr_t)model->meshes[i].firstIndex) * sizeof(uint32_t));
		draw->mat = &model->materialInstances[i];
		draw->indexType = mrd->indexType;
	}*/
}
