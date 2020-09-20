#ifndef _GL_RENDER_H_
#define _GL_RENDER_H_

#include "glad.h"

#include <Engine/Types.h>
#include <Runtime/Runtime.h>
#include <System/AtomicLock.h>

#define GET_DRAWABLES_SYS	L"GL_GetDrawables"
#define GL_SHADER_COUNT		5

struct RenderDevice
{
	void *glContext;
	struct AtomicLock *loadLock;
	GLint verMajor, verMinor;
};

struct ModelRenderData
{
	GLuint vao, vbo, ibo;
};

struct SceneRenderData
{
	Array drawables;
	GLuint sceneUbo;
};

struct TextureRenderData
{
	GLuint id;
};

struct Shader
{
	GLuint program;
	uint64_t hash;
	GLuint shaders[GL_SHADER_COUNT];
};

struct GetDrawablesArgs;

bool GL_InitDevice(void);
void GL_SwapBuffers(void);
void GL_SwapInterval(int interval);
void *GL_InitLoadContext(void);
void GL_MakeCurrent(void *ctx);
void GL_TermLoadContext(void *ctx);
void GL_TermDevice(void);

bool GL_LoadShaders(void);
void GL_UnloadShaders(void);

void GL_RenderScene(struct Scene *scene);
void GL_GetDrawables(void **comp, struct GetDrawablesArgs *args);

#endif /* _GL_RENDER_H_ */