#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <System/Log.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Render/Render.h>
#include <Render/Device.h>

#include "GLRender.h"

#include <GL/glx.h>
#include <GL/glxext.h>

#define GLRMOD	L"OpenGLX11"

extern Display *X11_Display;
extern XVisualInfo X11_VisualInfo;

static int _visualAttribs[] =
{
	GLX_X_RENDERABLE, True,
	GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
	GLX_RENDER_TYPE, GLX_RGBA_BIT,
	GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
	GLX_RED_SIZE, 8,
	GLX_GREEN_SIZE, 8,
	GLX_BLUE_SIZE, 8,
	GLX_ALPHA_SIZE, 0,
	GLX_DEPTH_SIZE, 24,
	GLX_STENCIL_SIZE, 0,
	0, 0,
	0, 0,
	0
};

static int _ctxAttribs[] =
{
	GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
	GLX_CONTEXT_MINOR_VERSION_ARB, 6,
	GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
	GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
	0
};

static PFNGLXSWAPINTERVALEXTPROC _glXSwapIntervalEXT;
static GLXFBConfig _fbConfig;

static inline bool
_checkExtension(const char *extension)
{
	if (Re_Device.verMajor >= 3) {
		GLint i;
		size_t len;
		static PFNGLGETSTRINGIPROC GetStringi = NULL;
		static GLint numExtensions = 0;

		if (!GetStringi) {
			GetStringi = (PFNGLGETSTRINGIPROC)glXGetProcAddress("glGetStringi");
			((PFNGLGETINTEGERVPROC)glXGetProcAddress("glGetIntegerv"))(GL_NUM_EXTENSIONS, &numExtensions);
		}

		len = strlen(extension);

		for (i = 0; i < numExtensions; ++i)
			if (!strncmp(GetStringi(GL_EXTENSIONS, i), extension, len))
				return true;

		return false;
	} else {
		static const char *ext = NULL;

		if (!ext)
			ext = ((PFNGLGETSTRINGPROC)glXGetProcAddress("glGetString"))(GL_EXTENSIONS);

		return strstr(ext, extension);
	}
}

static inline void
_LogError(const wchar_t *message)
{
	/*LPWSTR buffer;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&buffer, 0, NULL);

	Sys_LogEntry(GLRMOD, LOG_CRITICAL, L"%s: %s", message, buffer);

	LocalFree(buffer);*/
}

bool
GL_InitDevice(void)
{
	GLXFBConfig *fbc = NULL;
	int glXMajor, glXMinor, fbCount, bestFbc, worstFbc, i;
	PFNGLXCHOOSEFBCONFIGPROC glXChooseFBConfig = NULL;
	PFNGLXGETVISUALFROMFBCONFIGPROC glXGetVisualFromFBConfig = NULL;
	PFNGLXGETFBCONFIGATTRIBPROC glXGetFBConfigAttrib = NULL;
	PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = NULL;

	if (!glXQueryVersion(X11_Display, &glXMajor, &glXMinor) || ((glXMajor == 1) && (glXMinor < 4)) || (glXMajor < 1)) {
		//Platform::MessageBox("Fatal Error", "This program requires a newer version of GLX", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}

	glXChooseFBConfig = (PFNGLXCHOOSEFBCONFIGPROC)glXGetProcAddress((const GLubyte*)"glXChooseFBConfig");
	glXGetVisualFromFBConfig = (PFNGLXGETVISUALFROMFBCONFIGPROC)glXGetProcAddress((const GLubyte*)"glXGetVisualFromFBConfig");
	glXGetFBConfigAttrib = (PFNGLXGETFBCONFIGATTRIBPROC)glXGetProcAddress((const GLubyte*)"glXGetFBConfigAttrib");
	glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress("glXCreateContextAttribsARB");

	if (!glXChooseFBConfig || !glXGetVisualFromFBConfig || !glXGetFBConfigAttrib || !glXCreateContextAttribsARB) {
		//Platform::MessageBox("Fatal Error", "Unable to load required GLX functions", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}

	fbc = glXChooseFBConfig(X11_Display, DefaultScreen(X11_Display), _visualAttribs, &fbCount);
	if (!fbc) {
		_LogError(L"Failed to find fb config");
		return false;
	}

	_fbConfig = fbc[0];
	XFree(fbc);

	if (CVAR_BOOL(L"GL_Debug"))
		_ctxAttribs[7] |= GLX_CONTEXT_DEBUG_BIT_ARB;

	Re_Device.glContext = glXCreateContextAttribsARB(X11_Display, _fbConfig, 0, True, _ctxAttribs);

	if (!Re_Device.glContext) {
		_LogError(L"Failed to create context");
		return false;
	}

	XSync(X11_Display, False);
	Re_Device.loadLock = Sys_InitAtomicLock();

	if (!glXMakeCurrent(X11_Display, (GLXDrawable)E_Screen, (GLXContext)Re_Device.glContext)) {
		_LogError(L"Failed to activate context");
		glXDestroyContext(X11_Display, (GLXContext)Re_Device.glContext);
		return false;
	}

	((PFNGLGETINTEGERVPROC)glXGetProcAddress("glGetIntegerv"))(GL_MAJOR_VERSION, &Re_Device.verMajor);
	((PFNGLGETINTEGERVPROC)glXGetProcAddress("glGetIntegerv"))(GL_MINOR_VERSION, &Re_Device.verMinor);

	_glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress("glXSwapIntervalEXT");

	return true;
}

void
GL_SwapBuffers(void)
{
	glXSwapBuffers(X11_Display, (GLXDrawable)E_Screen);
}

void
GL_SwapInterval(int interval)
{
	if (_glXSwapIntervalEXT)
		_glXSwapIntervalEXT(X11_Display, (GLXDrawable)E_Screen, interval);
}

void *
GL_InitLoadContext(void)
{
	void *ctx = NULL;
	PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = NULL;
	glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress("glXCreateContextAttribsARB");
	if (!glXCreateContextAttribsARB)
		return NULL;

	ctx = glXCreateContextAttribsARB(X11_Display, _fbConfig, Re_Device.glContext, True, _ctxAttribs);
	if (!ctx)
		_LogError(L"Failed to load context");

	return ctx;
}

void
GL_MakeCurrent(void *ctx)
{
	glXMakeCurrent(X11_Display, (GLXDrawable)E_Screen, (GLXContext)ctx);
}

void
GL_TermLoadContext(void *ctx)
{
	glXDestroyContext(X11_Display, (GLXContext)ctx);
}


void
GL_TermDevice(void)
{
	if (Re_Device.loadLock)
		Sys_TermAtomicLock(Re_Device.loadLock);

	glXDestroyContext(X11_Display, (GLXContext)Re_Device.glContext);
}
