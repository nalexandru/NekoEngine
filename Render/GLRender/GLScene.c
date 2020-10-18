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

struct DrawableImmediate
{
	struct MaterialInstance *mat;
	GLsizei count;
	struct mat4 mvp;
	GLint baseVertex, startIndex;
	struct Vertex *vertices;
	uint32_t *indices;
	GLenum indexType;
};

struct DrawableBuffers
{
	void *indices;
	struct MaterialInstance *mat;
	GLsizei count;
	struct mat4 mvp;
	GLint baseVertex;
	GLuint vbo, ibo;
	GLenum indexType;
};

struct DrawableVAO
{
	void *indices;
	struct MaterialInstance *mat;
	GLsizei count;
	struct mat4 mvp;
	GLint baseVertex;
	GLuint vao;
	GLenum indexType;
};

struct GetDrawablesArgs
{
	Array *drawables;
};

// Vertex Arrays

bool
GL_InitSceneVAO(struct Scene *scene)
{
	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;
	return Rt_InitArray(&srd->drawables, 10, sizeof(struct DrawableVAO));
}

void
GL_TermSceneVAO(struct Scene *scene)
{
	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;
	Rt_TermArray(&srd->drawables);
}

void
GL_RenderSceneVAO(struct Scene *scene)
{
	size_t i = 0;
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
		glUseProgram(0);
}

void
GL_GetDrawablesVAO(void **comp, struct GetDrawablesArgs *args)
{
	uint32_t i = 0;
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
	}
}

// Vertex & Index Buffers

bool
GL_InitSceneBuffers(struct Scene *scene)
{
	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;
	return Rt_InitArray(&srd->drawables, 10, sizeof(struct DrawableBuffers));
}

void
GL_TermSceneBuffers(struct Scene *scene)
{
	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;
	Rt_TermArray(&srd->drawables);
}

void
GL_RenderSceneBuffers(struct Scene *scene)
{
	size_t i = 0;
	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;
	
	GLuint vbo = 0, ibo = 0, program = 0;

	struct GetDrawablesArgs args;
	args.drawables = &srd->drawables;

	Rt_ClearArray(args.drawables, false);
	E_ExecuteSystemS(scene, GET_DRAWABLES_SYS, &args);

	for (i = 0; i < srd->drawables.count; ++i) {
		struct DrawableBuffers *draw = (struct DrawableBuffers *)Rt_ArrayGet(&srd->drawables, i);
		struct Shader *s = draw->mat->shader;

		if (vbo != draw->vbo || ibo != draw->ibo) {
			glBindBuffer(GL_ARRAY_BUFFER, draw->vbo); vbo = draw->vbo;
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, draw->ibo); ibo = draw->ibo;
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

		if (GLAD_GL_ARB_draw_elements_base_vertex)
			glDrawElementsBaseVertex(GL_TRIANGLES, draw->count, draw->indexType, draw->indices, draw->baseVertex);
		else
			glDrawElements(GL_TRIANGLES, draw->count, draw->indexType, draw->indices);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(0);
}

void
GL_GetDrawablesBuffers(void **comp, struct GetDrawablesArgs *args)
{
	uint32_t i = 0;
	struct Transform *xform = (struct Transform *)comp[0];
	struct ModelRender *modelRender = (struct ModelRender *)comp[1];

	struct Model *model = (struct Model *)E_ResourcePtr(modelRender->model);
	struct ModelRenderData *mrd = (struct ModelRenderData *)&model->renderDataStart;
	
	struct mat4 mvp;
	m4_mul(&mvp, &Scn_ActiveCamera->projMatrix, &Scn_ActiveCamera->viewMatrix);
	m4_mul(&mvp, &mvp, &xform->mat);

	for (i = 0; i < model->numMeshes; ++i) {
		struct DrawableBuffers *draw = (struct DrawableBuffers *)Rt_ArrayAllocate(args->drawables);

		m4_copy(&draw->mvp, &mvp);
		draw->vbo = mrd->vbo;
		draw->ibo = mrd->ibo;
		draw->count = model->meshes[i].indexCount;
		draw->baseVertex = model->meshes[i].firstVertex;
		draw->indices = (void *)(((uintptr_t)model->meshes[i].firstIndex) * sizeof(uint32_t));
		draw->mat = &model->materialInstances[i];
		draw->indexType = mrd->indexType;
	}
}

// Immediate mode

bool
GL_InitSceneImmediate(struct Scene *scene)
{
	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;
	return Rt_InitArray(&srd->drawables, 10, sizeof(struct DrawableImmediate));
}

void
GL_TermSceneImmediate(struct Scene *scene)
{
	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;
	Rt_TermArray(&srd->drawables);
}

void
GL_RenderSceneImmediate(struct Scene* scene)
{
	size_t i = 0, j = 0;
	GLuint program = 0;
	struct SceneRenderData *srd = (struct SceneRenderData *)&scene->renderDataStart;

	struct GetDrawablesArgs args;
	args.drawables = &srd->drawables;

	Rt_ClearArray(args.drawables, false);
	E_ExecuteSystemS(scene, GET_DRAWABLES_SYS, &args);

	for (i = 0; i < srd->drawables.count; ++i) {
		struct DrawableImmediate *draw = (struct DrawableImmediate *)Rt_ArrayGet(&srd->drawables, i);
		struct Shader *s = draw->mat->shader;

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

		// TODO: cache results
		glBegin(GL_TRIANGLES);
		for (j = draw->startIndex; j < draw->count; ++j) {
			struct vec4 pos;
			const struct Vertex *v = &draw->vertices[draw->indices[j] + draw->baseVertex];

			v4_mul_m4(&pos, v4(&pos, v->x, v->y, v->z, 1.f), &draw->mvp);

			glVertex3f(pos.x, pos.y, pos.z);
			glNormal3f(v->nx, v->ny, v->nz);
			glTexCoord2f(v->u, v->v);

			if (glTangent3fEXT)
				glTangent3fEXT(v->tx, v->ty, v->tz);
		}
		glEnd();
	}

	glUseProgram(0);
}

void
GL_GetDrawablesImmediate(void **comp, struct GetDrawablesArgs *args)
{
	uint32_t i = 0;
	struct Transform *xform = (struct Transform *)comp[0];
	struct ModelRender *modelRender = (struct ModelRender *)comp[1];

	struct Model *model = (struct Model *)E_ResourcePtr(modelRender->model);
//	struct ModelRenderData *mrd = (struct ModelRenderData *)&model->renderDataStart;
	
	struct mat4 mvp;
	m4_mul(&mvp, &Scn_ActiveCamera->projMatrix, &Scn_ActiveCamera->viewMatrix);
	m4_mul(&mvp, &mvp, &xform->mat);

	for (i = 0; i < model->numMeshes; ++i) {
		struct DrawableImmediate *draw = (struct DrawableImmediate *)Rt_ArrayAllocate(args->drawables);

		m4_copy(&draw->mvp, &mvp);
		
		draw->count = model->meshes[i].indexCount;
		draw->baseVertex = model->meshes[i].firstVertex;
		draw->indices = (void *)(((uintptr_t)model->meshes[i].firstIndex) * sizeof(uint32_t));
		draw->mat = &model->materialInstances[i];
	//	draw->indexType = m->indexType;
	}
}

