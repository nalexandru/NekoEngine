#ifndef _MTL_RENDER_H_
#define _MTL_RENDER_H_

#define Handle __CocoaHandle
#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#undef Handle

#include <Engine/Types.h>
#include <Runtime/Runtime.h>
#include <System/AtomicLock.h>

#define GET_DRAWABLES_SYS	L"MTL_GetDrawables"
#define LOAD_UI_CONTEXT		L"MTL_LoadUIContext"
#define DRAW_UI_CONTEXT		L"MTL_DrawUIContext"
#define GL_SHADER_COUNT		5

struct RenderDevice
{
	id<MTLDevice> dev;
	id<MTLCommandQueue> cmdQueue;
	
	MTLRenderPassDescriptor *screenPass;
};

struct ModelRenderData
{
	id<MTLBuffer> vtxBuff;
	id<MTLBuffer> idxBuff;
};

struct SceneRenderData
{
	Array drawables;
	//GLuint sceneUbo;
};

struct TextureRenderData
{
	id<MTLTexture> handle;
};

struct Shader
{
	//GLuint program;
	uint64_t hash;
	//GLuint shaders[GL_SHADER_COUNT];
};

struct GetDrawablesArgs;
struct UIContextLoadArgs;

bool  MTL_InitDevice(void);
void  GL_SwapBuffers(void);
void  GL_SwapInterval(int interval);
void  GL_ScreenResized(void);
void *GL_InitLoadContext(void);
void  GL_MakeCurrent(void *ctx);
void  GL_TermLoadContext(void *ctx);
void  GL_TermDevice(void);

bool  MTL_LoadShaders(void);
void  MTL_UnloadShaders(void);

bool  MTL_InitUI(void);
void  MTL_RenderUI(struct Scene *scene);
void  MTL_TermUI(void);

void  MTL_LoadUIContext(void **comp, struct UIContextLoadArgs *args);
void  MTL_DrawUIContext(void **comp, void *args);

bool  MTL_InitModel(const char *name, struct Model *m);
void  MTL_TermModel(struct Model *m);
bool  MTL_InitScene(struct Scene *scene);
void  MTL_TermScene(struct Scene *scene);
void  MTL_RenderScene(struct Scene *scene);
void  MTL_GetDrawables(void **comp, struct GetDrawablesArgs *args);

void *MTL_GetShader(uint64_t hash);
bool  MTL_InitTexture(const char *name, struct Texture *tex, Handle h);
bool  MTL_UpdateTexture(struct Texture *tex, const void *data, uint64_t dataSize, uint64_t offset);
void  MTL_TermTexture(struct Texture *tex);;

#endif /* _MTL_RENDER_H_ */
