#import "MTLRender.h"

#include <System/Log.h>
#include <System/Thread.h>
#include <Scene/Scene.h>
#include <Render/Render.h>
#include <Render/Device.h>
#include <Engine/Job.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Engine/ECSystem.h>
#include <Scene/Components.h>

#define GLRMOD	L"MetalRender"

#ifndef _countof
#	define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

struct RenderDevice Re_Device = { 0 };

static void _InitThreadContext(int worker, void *args);
static void _TermThreadContext(int worker, void *args);

//static uint32_t _workerKey = 0;

static bool _Init(void);
static void _Term(void);
static void _WaitIdle(void);
static void _ScreenResized(void);
static void _RenderFrame(void);

uint32_t Re_ApiVersion = RE_API_VERSION;

bool
Re_InitLibrary(void)
{
	memset(&Re, 0x0, sizeof(Re));

	Re.Init = _Init;
	Re.Term = _Term;
	Re.WaitIdle = _WaitIdle;
	Re.ScreenResized = _ScreenResized;
	Re.RenderFrame = _RenderFrame;
	Re.GetShader = MTL_GetShader;
	Re.InitTexture = MTL_InitTexture;
	Re.UpdateTexture = MTL_UpdateTexture;
	Re.TermTexture = MTL_TermTexture;
	Re.InitModel = MTL_InitModel;
	Re.TermModel = MTL_TermModel;
	Re.InitScene = MTL_InitScene;
	Re.TermScene = MTL_TermScene;

	Re.sceneRenderDataSize = sizeof(struct SceneRenderData);
	Re.modelRenderDataSize = sizeof(struct ModelRenderData);
	Re.textureRenderDataSize = sizeof(struct TextureRenderData);

	return true;
}

bool
_Init(void)
{
	const wchar_t *comp[] = { UI_CONTEXT_COMP };
	
	Re_Device.dev = MTLCreateSystemDefaultDevice();
	if (!Re_Device.dev)
		return false;
	
	[(NSWindow *)E_Screen contentView].wantsLayer = true;
	
	Re_Device.cmdQueue = [Re_Device.dev newCommandQueue];
	
	swprintf(Re.info.name, sizeof(Re.info.name) / sizeof(wchar_t), L"Metal");
	swprintf(Re.info.device, sizeof(Re.info.device) / sizeof(wchar_t), L"%hs", [[Re_Device.dev name] UTF8String]);

	if (!MTL_LoadShaders())
		return false;
	
	Re.limits.maxTextureSize = 4096;
	
	if (!MTL_InitUI())
		return false;
	
	E_RegisterSystem(LOAD_UI_CONTEXT, ECSYS_GROUP_MANUAL, comp, 1, (ECSysExecProc)MTL_LoadUIContext, 0);
	E_RegisterSystem(DRAW_UI_CONTEXT, ECSYS_GROUP_MANUAL, comp, 1, (ECSysExecProc)MTL_DrawUIContext, 0);
	
	CAMetalLayer *layer = (CAMetalLayer *)[(NSWindow *)E_Screen contentView].layer;
	layer.device = Re_Device.dev;
	
	id<CAMetalDrawable> drawable = [(CAMetalLayer *)[(NSWindow *)E_Screen contentView].layer nextDrawable];
	
	Re_Device.screenPass = [MTLRenderPassDescriptor renderPassDescriptor];
	Re_Device.screenPass.colorAttachments[0].texture = drawable.texture;
	Re_Device.screenPass.colorAttachments[0].loadAction = MTLLoadActionClear;
	Re_Device.screenPass.colorAttachments[0].storeAction = MTLStoreActionStore;
	Re_Device.screenPass.colorAttachments[0].clearColor = MTLClearColorMake(0.867, 0.456, 1.0, 1.0);
	
	id<MTLCommandBuffer> cmdBuffer = [Re_Device.cmdQueue commandBuffer];
	id <MTLRenderCommandEncoder> cmdEncoder = [cmdBuffer renderCommandEncoderWithDescriptor:Re_Device.screenPass];
	[cmdEncoder endEncoding];
	
	[cmdBuffer presentDrawable:drawable];
	[cmdBuffer commit];
	
	//E_SetCVarBln(L"Engine_SingleThreadSceneLoad", true);

	//GL_SwapInterval(CVAR_BOOL(L"Render_VerticalSync"));
	
	return true;
}

void
_Term(void)
{
	MTL_TermUI();

	MTL_UnloadShaders();

	//if (!Re_Device.loadLock) {
	//	E_DispatchJobs(E_JobWorkerThreads(), _TermThreadContext, NULL, NULL);
	//	Sys_TlsFree(_workerKey);
	//}

	//MTL_TermDevice();
}

void
_WaitIdle(void)
{
	//
}

void
_ScreenResized(void)
{
	//GL_ScreenResized();
	//glViewport(0, 0, *E_ScreenWidth, *E_ScreenHeight);
}

void
_RenderFrame(void)
{
	id<CAMetalDrawable> drawable = [(CAMetalLayer *)[(NSWindow *)E_Screen contentView].layer nextDrawable];
	Re_Device.screenPass.colorAttachments[0].texture = drawable.texture;

	id<MTLCommandBuffer> cmdBuffer = [Re_Device.cmdQueue commandBuffer];
	id <MTLRenderCommandEncoder> cmdEncoder = [cmdBuffer renderCommandEncoderWithDescriptor:Re_Device.screenPass];
	[cmdEncoder endEncoding];
	
	[cmdBuffer presentDrawable:drawable];
	[cmdBuffer commit];
	
	/*glClearColor(.8f, .4f, .2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	MTL_RenderScene(Scn_ActiveScene);
	MTL_RenderUI(Scn_ActiveScene);

	GL_SwapBuffers();*/
}
