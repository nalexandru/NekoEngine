#include <UI/UI.h>
#include <Math/Math.h>
#include <Render/Shader.h>
#include <Render/Device.h>
#include <Render/Texture.h>
#include <Engine/Resource.h>
#include <Engine/ECSystem.h>

#include "GLRender.h"

struct UIContextLoadArgs
{
	uint8_t *vtxPtr;
	uint8_t *idxPtr;
};

static GLuint _vao, _vbo, _ibo;
static struct Shader *_uiShader;

bool
GL_InitUIVAO(void)
{
	if (GLAD_GL_ARB_direct_state_access) {
		glCreateBuffers(1, &_vbo);
		glCreateBuffers(1, &_ibo);
		glCreateVertexArrays(1, &_vao);

		glNamedBufferStorage(_vbo, sizeof(struct UIVertex) * 400, NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
		glNamedBufferStorage(_ibo, sizeof(uint16_t) * 600, NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

		glVertexArrayVertexBuffer(_vao, 0, _vbo, 0, sizeof(struct UIVertex));
		glVertexArrayElementBuffer(_vao, _ibo);

		glEnableVertexArrayAttrib(_vao, 0);
		glVertexArrayAttribFormat(_vao, 0, 4, GL_FLOAT, GL_FALSE, offsetof(struct UIVertex, posUv));
		glVertexArrayAttribBinding(_vao, 0, 0);

		glEnableVertexArrayAttrib(_vao, 1);
		glVertexArrayAttribFormat(_vao, 1, 4, GL_FLOAT, GL_FALSE, offsetof(struct UIVertex, color));
		glVertexArrayAttribBinding(_vao, 1, 0);
	} else {
		glGenBuffers(1, &_vbo);
		glGenBuffers(1, &_ibo);
		glGenVertexArrays(1, &_vao);
		
		glBindVertexArray(_vao);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
		
		glBufferData(GL_ARRAY_BUFFER, sizeof(struct UIVertex) * 400, NULL, GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 600, NULL, GL_DYNAMIC_DRAW);
		
		if (GL_ShaderSupport) {
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(struct UIVertex), (void *)offsetof(struct UIVertex, posUv));
			
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(struct UIVertex), (void *)offsetof(struct UIVertex, color));
		} else {
			
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(2, GL_FLOAT, sizeof(struct UIVertex), (void *)offsetof(struct UIVertex, posUv));
			
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, sizeof(struct UIVertex), (void *)(offsetof(struct UIVertex, posUv) + (sizeof(float) * 2)));
			
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(4, GL_FLOAT, sizeof(struct UIVertex), (void *)offsetof(struct UIVertex, color));
		}
		
		glBindVertexArray(0);
	}

	_uiShader = (struct Shader *)Re.GetShader(Rt_HashStringW(L"UI"));

	return true;
}

void
GL_TermUIVAO(void)
{
	glDeleteVertexArrays(1, &_vao);
	glDeleteBuffers(1, &_ibo);
	glDeleteBuffers(1, &_vbo);
}

void
GL_RenderUIVAO(struct Scene *s)
{
	struct UIContextLoadArgs loadArgs;

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
	
	loadArgs.vtxPtr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	loadArgs.idxPtr = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

	E_ExecuteSystemS(s, LOAD_UI_CONTEXT, &loadArgs);

	glUnmapBuffer(GL_ARRAY_BUFFER);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

	glBindVertexArray(_vao);

	if (GL_ShaderSupport) {
		glUseProgram(_uiShader->program);
		
		glUniform1i(1, 0);
		glUniformMatrix4fv(0, 1, GL_FALSE, (const GLfloat *)&UI_Projection);		
	} else {
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(UI_Projection.m);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);
	
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_ALPHA);

	E_ExecuteSystemS(s, DRAW_UI_CONTEXT, NULL);
	
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glDisable(GL_BLEND);

	if (GL_ShaderSupport)
		glUseProgram(0);

	glBindVertexArray(0);
}

void
GL_LoadUIContext(void **comp, struct UIContextLoadArgs *args)
{
	struct UIContext *ctx = (struct UIContext *)comp[0];

	const size_t vtxSize = sizeof(struct UIVertex) * ctx->vertices.count;
	const size_t idxSize = sizeof(uint16_t) * ctx->indices.count;

	memcpy(args->vtxPtr, ctx->vertices.data, vtxSize);
	args->vtxPtr += vtxSize;

	memcpy(args->idxPtr, ctx->indices.data, idxSize);
	args->idxPtr += idxSize;
}

void
GL_DrawUIContext(void **comp, void *args)
{
	size_t i = 0;
	struct UIContext *ctx = (struct UIContext *)comp[0];

	for (i = 0; i < ctx->draws.count; ++i) {
		const struct UIDrawCall *drawCall = (struct UIDrawCall *)Rt_ArrayGet(&ctx->draws, i);

		struct TextureRenderData *trd = (struct TextureRenderData *)&((struct Texture *)E_ResourcePtr(drawCall->texture))->renderDataStart;
		glBindTexture(GL_TEXTURE_2D, trd->id);

		if (!GL_ShaderSupport) {
			
		}
		
		glDrawElements(GL_TRIANGLES, drawCall->idxCount, GL_UNSIGNED_SHORT, (void *)((uintptr_t)drawCall->idxOffset));
	}
}
