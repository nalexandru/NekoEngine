#include <UI/UI.h>
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
GL_InitUI(void)
{
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

	_uiShader = (struct Shader *)Re_GetShader(Rt_HashStringW(L"UI"));

	return true;
}

void
GL_TermUI(void)
{
	glDeleteVertexArrays(1, &_vao);
	glDeleteBuffers(1, &_ibo);
	glDeleteBuffers(1, &_vbo);
}

void
GL_RenderUI(struct Scene *s)
{
	struct UIContextLoadArgs loadArgs;

	loadArgs.vtxPtr = glMapNamedBuffer(_vbo, GL_WRITE_ONLY);
	loadArgs.idxPtr = glMapNamedBuffer(_ibo, GL_WRITE_ONLY);

	E_ExecuteSystemS(s, LOAD_UI_CONTEXT, &loadArgs);

	glUnmapNamedBuffer(_vbo);
	glUnmapNamedBuffer(_ibo);

	glBindVertexArray(_vao);

	glUseProgram(_uiShader->program);

	glUniform1i(1, 0);
	glUniformMatrix4fv(0, 1, GL_FALSE, (const GLfloat *)&UI_Projection);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);

	E_ExecuteSystemS(s, DRAW_UI_CONTEXT, NULL);

	glDisable(GL_BLEND);

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
	struct UIContext *ctx = (struct UIContext *)comp[0];

	for (size_t i = 0; i < ctx->draws.count; ++i) {
		const struct UIDrawCall *drawCall = (struct UIDrawCall *)Rt_ArrayGet(&ctx->draws, i);

		struct TextureRenderData *trd = (struct TextureRenderData *)&((struct Texture *)E_ResourcePtr(drawCall->texture))->renderDataStart;
		glBindTexture(GL_TEXTURE_2D, trd->id);

		glDrawElements(GL_TRIANGLES, drawCall->idxCount, GL_UNSIGNED_SHORT, (void *)drawCall->idxOffset);
	}
}
