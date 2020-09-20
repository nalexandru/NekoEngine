#include <System/Log.h>
#include <Scene/Scene.h>
#include <Render/Render.h>
#include <Render/Device.h>
#include <Engine/Job.h>
#include <Engine/Config.h>
#include <Engine/ECSystem.h>
#include <Scene/Components.h>

#include "GLRender.h"

#define GLRMOD	L"OpenGLRender"

struct RenderInfo Re_RenderInfo = 
{
	{ L"OpenGL" },
	{ 0x0 },
	true
};
struct RenderFeatures Re_Features = { false, false, false };
struct RenderDevice Re_Device = { 0 };

static void _InitThreadContext(int worker, void *args);
static void _TermThreadContext(int worker, void *args);

static uint32_t _workerKey = 0;

#ifdef _DEBUG
static void _DebugCallbackAMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam);
static void _DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);
static inline void _LogDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg);
#endif

bool
Re_Init(void)
{
	const char *version;
	GLint major, minor;

	if (!GL_InitDevice())
		return false;

	if (!Re_Device.loadLock) {
		_workerKey = Sys_TlsAlloc();
		E_DispatchJobs(E_JobWorkerThreads(), _InitThreadContext, NULL, NULL);
	}

	E_SetCVarBln(L"Engine_SingleThreadSceneLoad", true);

	gladLoadGL();

	version = glGetString(GL_VERSION);
	if (version) {
		if (strstr(version, "OpenGL"))
			swprintf(Re_RenderInfo.name, 64, L"%S", version);
		else
			swprintf(Re_RenderInfo.name, 64, L"OpenGL %S", version);
	} else {
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);
		swprintf(Re_RenderInfo.name, 64, L"OpenGL %d.%d", major, minor);
	}

	mbstowcs(Re_RenderInfo.device, glGetString(GL_RENDERER), 256);

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

	const wchar_t *comp[] = { TRANSFORM_COMP, MODEL_RENDER_COMP };
	E_RegisterSystem(GET_DRAWABLES_SYS, ECSYS_GROUP_MANUAL, comp, _countof(comp), (ECSysExecProc)GL_GetDrawables, 0);

	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);

	GL_SwapInterval(CVAR_BOOL(L"Render_VerticalSync"));

	return GL_LoadShaders();;
}

void
Re_Term(void)
{
	GL_UnloadShaders();

	if (!Re_Device.loadLock) {
		E_DispatchJobs(E_JobWorkerThreads(), _TermThreadContext, NULL, NULL);
		Sys_TlsFree(_workerKey);
	}

	GL_TermDevice();
}

void
Re_WaitIdle(void)
{
	//
}

void
Re_ScreenResized(void)
{
	//
}

void
Re_RenderFrame(void)
{
	glClearColor(.8f, .4f, .2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	GL_RenderScene(Scn_ActiveScene);

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

#ifdef _DEBUG

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

#endif
