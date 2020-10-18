#ifndef _GL_RENDER_H_
#define _GL_RENDER_H_

#include "glad.h"

#include <Engine/Types.h>
#include <Runtime/Runtime.h>
#include <System/AtomicLock.h>

#define GET_DRAWABLES_SYS	L"GL_GetDrawables"
#define LOAD_UI_CONTEXT		L"GL_LoadUIContext"
#define DRAW_UI_CONTEXT		L"GL_DrawUIContext"
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
	GLenum indexType;
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

struct GLRenderProcs
{
	void (*RenderScene)(struct Scene *s);
	void (*RenderUI)(struct Scene *s);
	
	bool (*InitUI)(void);
	void (*TermUI)(void);
};

struct GetDrawablesArgs;
struct UIContextLoadArgs;

extern bool GL_ShaderSupport;
extern struct GLRenderProcs GLProcs;

bool  GL_InitDevice(void);
void  GL_SwapBuffers(void);
void  GL_SwapInterval(int interval);
void  GL_ScreenResized(void);
void *GL_InitLoadContext(void);
void  GL_MakeCurrent(void *ctx);
void  GL_TermLoadContext(void *ctx);
void  GL_TermDevice(void);

bool  GL_LoadShaders(void);
void  GL_UnloadShaders(void);

bool  GL_InitUIVAO(void);
void  GL_RenderUIVAO(struct Scene *scene);
void  GL_TermUIVAO(void);

void  GL_LoadUIContext(void **comp, struct UIContextLoadArgs *args);
void  GL_DrawUIContext(void **comp, void *args);

bool  GL_InitModelImmediate(const char *name, struct Model *m);
void  GL_TermModelImmediate(struct Model *m);
bool  GL_InitSceneImmediate(struct Scene *scene);
void  GL_TermSceneImmediate(struct Scene *scene);
void  GL_RenderSceneImmediate(struct Scene *scene);
void  GL_GetDrawablesImmediate(void **comp, struct GetDrawablesArgs *args);

bool  GL_InitModelBuffers(const char *name, struct Model *m);
void  GL_TermModelBuffers(struct Model *m);
bool  GL_InitSceneBuffers(struct Scene *scene);
void  GL_TermSceneBuffers(struct Scene *scene);
void  GL_RenderSceneBuffers(struct Scene *scene);
void  GL_GetDrawablesBuffers(void **comp, struct GetDrawablesArgs *args);

bool  GL_InitModelVAO(const char *name, struct Model *m);
void  GL_TermModelVAO(struct Model *m);
bool  GL_InitSceneVAO(struct Scene *scene);
void  GL_TermSceneVAO(struct Scene *scene);
void  GL_RenderSceneVAO(struct Scene *scene);
void  GL_GetDrawablesVAO(void **comp, struct GetDrawablesArgs *args);

void *GL_GetShader(uint64_t hash);
bool  GL_InitTexture(const char *name, struct Texture *tex, Handle h);
bool  GL_UpdateTexture(struct Texture *tex, const void *data, uint64_t dataSize, uint64_t offset);
void  GL_TermTexture(struct Texture *tex);;
void  GL_InitTextureFormats(void);

#endif /* _GL_RENDER_H_ */