#include <System/Log.h>
#include <Scene/Scene.h>
#include <Render/Render.h>
#include <Render/Device.h>
#include <Engine/Job.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Engine/ECSystem.h>
#include <Scene/Components.h>

#include "GLRender.h"

#define GLRMOD	L"OpenGLRender"

#if defined(_WIN32)
#	define EXPORT	__declspec(dllexport)
#else
#	define EXPORT
#endif

#ifndef _countof
#	define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

struct RenderDevice Re_Device = { 0 };

bool GL_ShaderSupport = false;
struct GLRenderProcs GLProcs = { 0 };

static void _InitThreadContext(int worker, void *args);
static void _TermThreadContext(int worker, void *args);

static uint32_t _workerKey = 0;

static bool _Init(void);
static void _Term(void);
static void _WaitIdle(void);
static void _ScreenResized(void);
static void _RenderFrame(void);
static void _DebugCallbackAMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam);
static void _DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);
static inline void _LogDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);

static void _InitVAO(void);
static void _InitBuffers(void);
static void _InitImmediate(void);

EXPORT uint32_t Re_ApiVersion = RE_API_VERSION;
EXPORT bool
Re_InitLibrary(void)
{
	memset(&Re, 0x0, sizeof(Re));

	Re.Init = _Init;
	Re.Term = _Term;
	Re.WaitIdle = _WaitIdle;
	Re.ScreenResized = _ScreenResized;
	Re.RenderFrame = _RenderFrame;
	Re.GetShader = GL_GetShader;
	Re.InitTexture = GL_InitTexture;
	Re.UpdateTexture = GL_UpdateTexture;
	Re.TermTexture = GL_TermTexture;

	Re.sceneRenderDataSize = sizeof(struct SceneRenderData);
	Re.modelRenderDataSize = sizeof(struct ModelRenderData);
	Re.textureRenderDataSize = sizeof(struct TextureRenderData);

	return true;
}

bool
_Init(void)
{
	const char *version;
	const wchar_t *comp[] = { UI_CONTEXT_COMP };
	GLint tmp;

	if (!GL_InitDevice())
		return false;
	
	if (!Re_Device.loadLock) {
		_workerKey = Sys_TlsAlloc();
		E_DispatchJobs(E_JobWorkerThreads(), _InitThreadContext, NULL, NULL);
	}

	E_SetCVarBln(L"Engine_SingleThreadSceneLoad", true);

	gladLoadGL();

	glClear(GL_COLOR_BUFFER_BIT);
	GL_SwapBuffers();
	
	// These variables *might* be set by GL_InitDevice
	glGetIntegerv(GL_MAJOR_VERSION, &Re_Device.verMajor);
	glGetIntegerv(GL_MINOR_VERSION, &Re_Device.verMinor);

	version = (const char *)glGetString(GL_VERSION);
	if (version) {
		if (strstr(version, "OpenGL"))
			swprintf(Re.info.name, 64, L"%hs", version);
		else
			swprintf(Re.info.name, 64, L"OpenGL %hs", version);
	} else {
		swprintf(Re.info.name, 64, L"OpenGL %d.%d", Re_Device.verMajor, Re_Device.verMinor);
	}

	if (Re_Device.verMajor == 0) {
		while (version && !isdigit(*version))
			++version;

		sscanf(version, "%d.%d", &Re_Device.verMajor, &Re_Device.verMinor);
	}

	mbstowcs(Re.info.device, (const char *)glGetString(GL_RENDERER), 256);

	if (CVAR_BOOL(L"GL_Debug")) {
		if (glad_glDebugMessageCallback)
			glDebugMessageCallback((GLDEBUGPROC)_DebugCallback, NULL);
		else if (glad_glDebugMessageCallbackARB)
			glDebugMessageCallbackARB((GLDEBUGPROCARB)_DebugCallback, NULL);
		else if (glad_glDebugMessageCallbackKHR)
			glDebugMessageCallbackKHR((GLDEBUGPROCKHR)_DebugCallback, NULL);
		else if (glad_glDebugMessageCallbackAMD)
			glDebugMessageCallbackAMD((GLDEBUGPROCAMD)_DebugCallbackAMD, NULL);
		else
			Sys_LogEntry(GLRMOD, LOG_WARNING, L"Debug callback extension not available");
	}

	if (!GL_LoadShaders())
		return false;

	if (Re_Device.verMajor >= 3 ||
		GLAD_GL_ARB_vertex_array_object ||
		GLAD_GL_APPLE_vertex_array_object) {
		_InitVAO();
	} else if (Re_Device.verMajor >= 2 ||
			   GLAD_GL_ARB_vertex_buffer_object) {
		_InitBuffers();
	} else {
		_InitImmediate();
	}
	
	if (!GLProcs.InitUI())
		return false;

	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);

	GL_SwapInterval(CVAR_BOOL(L"Render_VerticalSync"));

	E_RegisterSystem(LOAD_UI_CONTEXT, ECSYS_GROUP_MANUAL, comp, 1, (ECSysExecProc)GL_LoadUIContext, 0);
	E_RegisterSystem(DRAW_UI_CONTEXT, ECSYS_GROUP_MANUAL, comp, 1, (ECSysExecProc)GL_DrawUIContext, 0);

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &tmp);
	Re.limits.maxTextureSize = (uint32_t)tmp;

	GL_InitTextureFormats();
	
	return true;
}

void
_Term(void)
{
	GLProcs.TermUI();

	GL_UnloadShaders();

	if (!Re_Device.loadLock) {
		E_DispatchJobs(E_JobWorkerThreads(), _TermThreadContext, NULL, NULL);
		Sys_TlsFree(_workerKey);
	}

	GL_TermDevice();
}

void
_WaitIdle(void)
{
	//
}

void
_ScreenResized(void)
{
	GL_ScreenResized();
	glViewport(0, 0, *E_ScreenWidth, *E_ScreenHeight);
}

void
_RenderFrame(void)
{
	glClearColor(.8f, .4f, .2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	GLProcs.RenderScene(Scn_ActiveScene);
	GLProcs.RenderUI(Scn_ActiveScene);

	GL_SwapBuffers();
}

void
_InitThreadContext(int worker, void *args)
{
	void *ctx = GL_InitLoadContext();
	GL_MakeCurrent(ctx);
	Sys_TlsSet(_workerKey, ctx);
}

void
_TermThreadContext(int worker, void *args)
{
	void *ctx = Sys_TlsGet(_workerKey);
	GL_MakeCurrent(NULL);
	GL_TermLoadContext(ctx);
}

void
_DebugCallbackAMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	_LogDebugOutput(category, category, id, severity, message);
}
 
void
_DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam)
{
	_LogDebugOutput(source, type, id, severity, message);
}

void
_LogDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg)
{
	const wchar_t *sourceString, *typeString, *severityString;
	
	// The AMD variant of this extension provides a less detailed classification of the error,
	// which is why some arguments might be "Unknown".
	switch (source)
	{
	case GL_DEBUG_CATEGORY_API_ERROR_AMD:
	case GL_DEBUG_SOURCE_API:
		sourceString = L"OpenGL|API";
	break;
	case GL_DEBUG_CATEGORY_APPLICATION_AMD:
	case GL_DEBUG_SOURCE_APPLICATION:
		sourceString = L"OpenGL|Application";
	break;
	case GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD:
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		sourceString = L"OpenGL|Window System";
	break;
	case GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD:
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		sourceString = L"OpenGL|Shader Compiler";
	break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		sourceString = L"OpenGL|Third Party";
	break;
	case GL_DEBUG_CATEGORY_OTHER_AMD:
	case GL_DEBUG_SOURCE_OTHER:
		sourceString = L"OpenGL|Other";
	break;
	default:
		sourceString = L"OpenGL|Unknown";
	break;
	}
	
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:
		typeString = L"Error";
	break;
	case GL_DEBUG_CATEGORY_DEPRECATION_AMD:
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		typeString = L"Deprecated Behavior";
	break;
	case GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD:
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		typeString = L"Undefined Behavior";
	break;
	case GL_DEBUG_TYPE_PORTABILITY_ARB:
		typeString = L"Portability";
	break;
	case GL_DEBUG_CATEGORY_PERFORMANCE_AMD:
	case GL_DEBUG_TYPE_PERFORMANCE:
		typeString = L"Performance";
	break;
	case GL_DEBUG_CATEGORY_OTHER_AMD:
	case GL_DEBUG_TYPE_OTHER:
		typeString = L"Other";
	break;
	default:
		typeString = L"Unknown";
	break;
	}
	
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		severityString = L"High";
	break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		severityString = L"Medium";
	break;
	case GL_DEBUG_SEVERITY_LOW:
		severityString = L"Low";
	break;
	default:
		severityString = L"Unknown";
	break;
	}
	
	Sys_LogEntry(sourceString, LOG_DEBUG, L"[%s][%s][%u]: %S", typeString, severityString, id, msg);
}

void
_InitVAO(void)
{
	const wchar_t *comp[] = { TRANSFORM_COMP, MODEL_RENDER_COMP };
	E_RegisterSystem(GET_DRAWABLES_SYS, ECSYS_GROUP_MANUAL, comp, _countof(comp), (ECSysExecProc)GL_GetDrawablesVAO, 0);

	Re.InitModel = GL_InitModelVAO;
	Re.TermModel = GL_TermModelVAO;
	Re.InitScene = GL_InitSceneVAO;
	Re.TermScene = GL_TermSceneVAO;
	
	GLProcs.RenderScene = GL_RenderSceneVAO;
	GLProcs.RenderUI = GL_RenderUIVAO;
	GLProcs.InitUI = GL_InitUIVAO;
	GLProcs.TermUI = GL_TermUIVAO;
	
	if (!GLAD_GL_ARB_vertex_array_object && GLAD_GL_APPLE_vertex_array_object) {
		glGenVertexArrays = glGenVertexArraysAPPLE;
		glBindVertexArray = glBindVertexArrayAPPLE;
		glDeleteVertexArrays = glDeleteVertexArraysAPPLE;
	}
}
#include <Render/Model.h>
void
_InitBuffers(void)
{
	const wchar_t *comp[] = { TRANSFORM_COMP, MODEL_RENDER_COMP };
	E_RegisterSystem(GET_DRAWABLES_SYS, ECSYS_GROUP_MANUAL, comp, _countof(comp), (ECSysExecProc)GL_GetDrawablesBuffers, 0);

	Re.InitModel = GL_InitModelBuffers;
	Re.TermModel = GL_TermModelBuffers;
	Re.InitScene = GL_InitSceneBuffers;
	Re.TermScene = GL_TermSceneBuffers;
	GLProcs.RenderScene = GL_RenderSceneBuffers;

	/*glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void *)offsetof(struct Vertex, x));
		
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void *)offsetof(struct Vertex, nx));
		
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void *)offsetof(struct Vertex, tx));
		
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void *)offsetof(struct Vertex, u));*/
}

void
_InitImmediate(void)
{
	const wchar_t *comp[] = { TRANSFORM_COMP, MODEL_RENDER_COMP };
	E_RegisterSystem(GET_DRAWABLES_SYS, ECSYS_GROUP_MANUAL, comp, _countof(comp), (ECSysExecProc)GL_GetDrawablesImmediate, 0);

	Re.InitModel = GL_InitModelImmediate;
	Re.TermModel = GL_TermModelImmediate;
	Re.InitScene = GL_InitSceneImmediate;
	Re.TermScene = GL_TermSceneImmediate;
	GLProcs.RenderScene = GL_RenderSceneImmediate;
}
