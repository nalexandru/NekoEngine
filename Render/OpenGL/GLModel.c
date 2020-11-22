#include <stddef.h>

#include <Render/Model.h>
#include <Render/Device.h>

#include "GLRender.h"

static inline void _PrepareIndices(struct Model *m, GLsizei *size, GLenum *type, void **out);
static inline void _PrepareIndicesRewrite(struct Model *m, GLsizei *size, GLenum *type, void **out);

bool
GL_InitModelVAO(const char *name, struct Model *m)
{
	struct ModelRenderData *mrd = (struct ModelRenderData *)&m->renderDataStart;
	GLsizei vertexSize = sizeof(struct Vertex) * m->numVertices;
	GLsizei indexSize = 0;
	void *indices = NULL;

	if (Re_Device.loadLock) {
		Sys_AtomicLockWrite(Re_Device.loadLock);
		GL_MakeCurrent(Re_Device.glContext);
	}
	
	
	if (glDrawElementsBaseVertex)
		_PrepareIndices(m, &indexSize, &mrd->indexType, &indices);
	else
		_PrepareIndicesRewrite(m, &indexSize, &mrd->indexType, &indices);

	if (GLAD_GL_ARB_direct_state_access) {
		glCreateBuffers(1, &mrd->vbo);
		glCreateBuffers(1, &mrd->ibo);
		glCreateVertexArrays(1, &mrd->vao);

		glNamedBufferStorage(mrd->vbo, vertexSize, m->vertices, 0);
		glNamedBufferStorage(mrd->ibo, indexSize, indices, 0);

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
	} else {
		glGenBuffers(1, &mrd->vbo);
		glGenBuffers(1, &mrd->ibo);
		glGenVertexArrays(1, &mrd->vao);
		
		glBindVertexArray(mrd->vao);
		glBindBuffer(GL_ARRAY_BUFFER, mrd->vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mrd->ibo);
		
		glBufferData(GL_ARRAY_BUFFER, vertexSize, m->vertices, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, indices, GL_STATIC_DRAW);
		
		glBindVertexArray(mrd->vao);

		if (GL_ShaderSupport) {
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void *)offsetof(struct Vertex, x));
			
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void *)offsetof(struct Vertex, nx));
			
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void *)offsetof(struct Vertex, tx));
			
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void *)offsetof(struct Vertex, u));
		} else {
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, sizeof(struct Vertex), (void *)offsetof(struct Vertex, x));
			
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT, sizeof(struct Vertex), (void *)offsetof(struct Vertex, nx));
			
			//glVertexPointer(3, GL_FLOAT, sizeof(struct Vertex), (void *)offsetof(struct Vertex, x));
			
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, sizeof(struct Vertex), (void *)offsetof(struct Vertex, u));
		}
		
		glBindVertexArray(0);
	}

	if (Re_Device.loadLock)
		Sys_AtomicUnlockWrite(Re_Device.loadLock);

	return true;
}

void
GL_TermModelVAO(struct Model *m)
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

bool
GL_InitModelBuffers(const char *name, struct Model *m)
{
	struct ModelRenderData *mrd = (struct ModelRenderData *)&m->renderDataStart;
	GLsizei vertexSize = sizeof(struct Vertex) * m->numVertices;
	GLsizei indexSize = 0;
	void *indices = NULL;
	
	if (Re_Device.loadLock) {
		Sys_AtomicLockWrite(Re_Device.loadLock);
		GL_MakeCurrent(Re_Device.glContext);
	}
	
	_PrepareIndicesRewrite(m, &indexSize, &mrd->indexType, &indices);

	glGenBuffers(1, &mrd->vbo);
	glGenBuffers(1, &mrd->ibo);
	
	glBindBuffer(GL_ARRAY_BUFFER, mrd->vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexSize, m->vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mrd->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (Re_Device.loadLock)
		Sys_AtomicUnlockWrite(Re_Device.loadLock);

	return true;
}

void
GL_TermModelBuffers(struct Model *m)
{
	struct ModelRenderData *mrd = (struct ModelRenderData *)&m->renderDataStart;

	if (Re_Device.loadLock) {
		Sys_AtomicLockWrite(Re_Device.loadLock);
		GL_MakeCurrent(Re_Device.glContext);
	}
	
	glDeleteBuffers(1, &mrd->vbo);
	glDeleteBuffers(1, &mrd->ibo);
	
	if (Re_Device.loadLock)
		Sys_AtomicUnlockWrite(Re_Device.loadLock);
}

bool
GL_InitModelImmediate(const char *name, struct Model *m)
{
	/*struct ModelRenderData *mrd = (struct ModelRenderData *)&m->renderDataStart;
	GLsizei vertexSize = sizeof(struct Vertex) * m->numVertices;
	GLsizei indexSize = sizeof(uint32_t) * m->numIndices;

	if (Re_Device.loadLock) {
		Sys_AtomicLockWrite(Re_Device.loadLock);
		GL_MakeCurrent(Re_Device.glContext);
	}

	if (Re_Device.loadLock)
		Sys_AtomicUnlockWrite(Re_Device.loadLock);*/

	return true;
}

void
GL_TermModelImmediate(struct Model *m)
{
	/*struct ModelRenderData *mrd = (struct ModelRenderData *)&m->renderDataStart;

	if (Re_Device.loadLock) {
		Sys_AtomicLockWrite(Re_Device.loadLock);
		GL_MakeCurrent(Re_Device.glContext);
	}
	
	if (Re_Device.loadLock)
		Sys_AtomicUnlockWrite(Re_Device.loadLock);*/
}

void
_PrepareIndices(struct Model *m, GLsizei *size, GLenum *type, void **out)
{
	uint32_t i;
	
	if (m->numIndices < UINT16_MAX) {
		uint16_t *idx = Sys_Alloc(sizeof(uint16_t), m->numIndices, MH_Transient);
		
		for (i = 0; i < m->numIndices; ++i)
			idx[i] = (uint16_t)m->indices[i];
		
		*out = idx;
		*type = GL_UNSIGNED_SHORT;
		*size = sizeof(uint16_t) * m->numIndices;
	} else {
		*out = m->indices;
		*type = GL_UNSIGNED_INT;
		*size = sizeof(uint32_t) * m->numIndices;
	}
}

void
_PrepareIndicesRewrite(struct Model *m, GLsizei *size, GLenum *type, void **out)
{
	uint32_t i, j;
	
	if (m->numIndices < UINT16_MAX) {
		uint16_t *idx = Sys_Alloc(sizeof(uint16_t), m->numIndices, MH_Transient);
		
		for (i = 0; i < m->numMeshes; ++i) {
			const struct Mesh *mesh = &m->meshes[i];
			const uint32_t lastIndex = mesh->firstIndex + mesh->indexCount;
			
			for (j = mesh->firstIndex; j < lastIndex; ++j)
				idx[j] = (uint16_t)(m->indices[j] + mesh->firstVertex);
		}
		
		*out = idx;
		*type = GL_UNSIGNED_SHORT;
		*size = sizeof(uint16_t) * m->numIndices;
	} else {
		uint32_t *idx = Sys_Alloc(sizeof(uint32_t), m->numIndices, MH_Transient);
		
		for (i = 0; i < m->numMeshes; ++i) {
			const struct Mesh *mesh = &m->meshes[i];
			const uint32_t lastIndex = mesh->firstIndex + mesh->indexCount;
			
			for (j = mesh->firstIndex; j < lastIndex; ++j)
				idx[j] = (uint32_t)(m->indices[j] + mesh->firstVertex);
		}
		
		*out = idx;
		*type = GL_UNSIGNED_INT;
		*size = sizeof(uint32_t) * m->numIndices;
	}
}
