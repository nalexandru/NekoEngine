#include <stddef.h>

#include <Render/Model.h>
#include <Render/Device.h>

#include "GLRender.h"

const size_t Re_ModelRenderDataSize = sizeof(struct ModelRenderData);

bool
Re_InitModel(const char *name, struct Model *m)
{
	struct ModelRenderData *mrd = (struct ModelRenderData *)&m->renderDataStart;
	GLsizei vertexSize = sizeof(struct Vertex) * m->numVertices;
	GLsizei indexSize = sizeof(uint32_t) * m->numIndices;

	if (Re_Device.loadLock) {
		Sys_AtomicLockWrite(Re_Device.loadLock);
		GL_MakeCurrent(Re_Device.glContext);
	}

	glCreateBuffers(1, &mrd->vbo);
	glCreateBuffers(1, &mrd->ibo);
	glCreateVertexArrays(1, &mrd->vao);

	glNamedBufferStorage(mrd->vbo, vertexSize, m->vertices, 0);
	glNamedBufferStorage(mrd->ibo, indexSize, m->indices, 0);

	glVertexArrayVertexBuffer(mrd->vao, 0, mrd->vbo, 0, sizeof(struct Vertex));
	glVertexArrayElementBuffer(mrd->vao, mrd->ibo);

	glEnableVertexArrayAttrib(mrd->vao, 0);
	glVertexArrayAttribFormat(mrd->vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(struct Vertex, x));
	glVertexArrayAttribBinding(mrd->vao, 0, 0);

	glEnableVertexArrayAttrib(mrd->vao, 1);
	glVertexArrayAttribFormat(mrd->vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(struct Vertex, nx));
	glVertexArrayAttribBinding(mrd->vao, 1, 0);

	glEnableVertexArrayAttrib(mrd->vao, 2);
	glVertexArrayAttribFormat(mrd->vao, 2, 3, GL_FLOAT, GL_FALSE, offsetof(struct Vertex, tx));
	glVertexArrayAttribBinding(mrd->vao, 2, 0);

	glEnableVertexArrayAttrib(mrd->vao, 3);
	glVertexArrayAttribFormat(mrd->vao, 3, 2, GL_FLOAT, GL_FALSE, offsetof(struct Vertex, u));
	glVertexArrayAttribBinding(mrd->vao, 3, 0);

	if (Re_Device.loadLock)
		Sys_AtomicUnlockWrite(Re_Device.loadLock);

	return true;
}

void
Re_TermModel(struct Model *m)
{
	struct ModelRenderData *mrd = (struct ModelRenderData *)&m->renderDataStart;

	if (Re_Device.loadLock) {
		Sys_AtomicLockWrite(Re_Device.loadLock);
		GL_MakeCurrent(Re_Device.glContext);
	}
	
	glDeleteBuffers(1, &mrd->vbo);
	glDeleteBuffers(1, &mrd->ibo);
	glDeleteVertexArrays(1, &mrd->vao);

	if (Re_Device.loadLock)
		Sys_AtomicUnlockWrite(Re_Device.loadLock);
}
