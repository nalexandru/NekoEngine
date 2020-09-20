#include <X1s.h>

#include <System/Log.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Render/Render.h>
#include <Render/Device.h>

#include "GLRender.h"
#include <GL/glxext.h>

#define GLRMOD	L"OpenGLX11"


static int _visualAttribs[] =
{
	GLX_X_RENDERABLE, True,
	GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
	GLX_RENDER_TYPE, GLX_RGBA_BIT,
	GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
	GLX_RED_SIZE, 8,
	GLX_GREEN_SIZE, 8,
	GLX_BLUE_SIZE, 8,
	GLX_ALPHA_SIZE, 8,
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

static PFNWGLSWAPINTERVALEXTPROC _wglSwapIntervalEXT;

static inline bool
_checkExtension(const char *extension)
{
	if (Re_Device.verMajor >= 3) {
		GLint i;
		size_t len;
		static PFNGLGETSTRINGIPROC GetStringi = NULL;
		static GLint numExtensions = 0;

		if (!GetStringi) {
			GetStringi = (PFNGLGETSTRINGIPROC)wglGetProcAddress("glGetStringi");
			((PFNGLGETINTEGERVPROC)GetProcAddress(_libGL, "glGetIntegerv"))(GL_NUM_EXTENSIONS, &numExtensions);
		}

		len = strlen(extension);

		for (i = 0; i < numExtensions; ++i)
			if (!strncmp(GetStringi(GL_EXTENSIONS, i), extension, len))
				return true;

		return false;
	} else {
		static const char *ext = NULL;

		if (!ext)
			ext = ((PFNGLGETSTRINGPROC)GetProcAddress(_libGL, "glGetString"))(GL_EXTENSIONS);

		return strstr(ext, extension);
	}
}

static inline void
_LogError(const wchar_t *message)
{
	LPWSTR buffer;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&buffer, 0, NULL);

	Sys_LogEntry(GLRMOD, LOG_CRITICAL, L"%s: %s", message, buffer);

	LocalFree(buffer);
}

bool
GL_InitDevice(void)
{
	int glXMajor, glXMinor;
	PFNGLXCHOOSEFBCONFIGPROC glXChooseFBConfig = NULL;
	PFNGLXGETVISUALFROMFBCONFIGPROC glXGetVisualFromFBConfig = NULL;
	PFNGLXGETFBCONFIGATTRIBPROC glXGetFBConfigAttrib = NULL;
	PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = NULL;

	if (!glXQueryVersion(

//	_libGL = LoadLibrary(L"opengl32");
//	if (!_libGL)
//		return false;

	_dc = GetDC((HWND)E_Screen);

	pixelFormat = ChoosePixelFormat(_dc, &_pfd);
	SetPixelFormat(_dc, pixelFormat, &_pfd);

	dummy = wglCreateContext(_dc);
	if (!dummy) {
		_LogError(L"Failed to create dummy context");
		return false;
	}

	if (!wglMakeCurrent(_dc, dummy)) {
		_LogError(L"Failed to activate dummy context");
		wglDeleteContext(dummy);
		return false;
	}

	if (CVAR_BOOL(L"GL_Debug"))
		_ctxAttribs[7] |= GLX_CONTEXT_DEBUG_BIT_ARB;

	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	if (wglCreateContextAttribsARB) {
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(dummy);

		Re_Device.glContext = wglCreateContextAttribsARB(_dc, NULL, _ctxAttribs);
		if (!Re_Device.glContext) {
			_LogError(L"Failed to create context");
			return false;
		}
	} else {
		// Use the already created OpenGL context
		Re_Device.glContext = dummy;
		Re_Device.loadLock = Sys_InitAtomicLock();
	}

	if (!wglMakeCurrent(_dc, Re_Device.glContext)) {
		_LogError(L"Failed to activate context");
		wglDeleteContext(Re_Device.glContext);
		return false;
	}

	((PFNGLGETINTEGERVPROC)GetProcAddress(_libGL, "glGetIntegerv"))(GL_MAJOR_VERSION, &Re_Device.verMajor);
	((PFNGLGETINTEGERVPROC)GetProcAddress(_libGL, "glGetIntegerv"))(GL_MINOR_VERSION, &Re_Device.verMinor);

	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	if (wglChoosePixelFormatARB) {
		if (CVAR_BOOL(L"Render_Multisampling")) {
			if (_checkExtension("GL_ARB_multisample") || _checkExtension("GL_EXT_multisample")) {
				_pixelFormatAttribs[12] = WGL_SAMPLE_BUFFERS_ARB;
				_pixelFormatAttribs[13] = 1;
				_pixelFormatAttribs[14] = WGL_SAMPLES_ARB;
				_pixelFormatAttribs[15] = CVAR_INT32(L"Render_Samples");
			} else if (_checkExtension("GL_3DFX_multisample")) {
				_pixelFormatAttribs[12] = WGL_SAMPLE_BUFFERS_3DFX;
				_pixelFormatAttribs[13] = 1;
				_pixelFormatAttribs[14] = WGL_SAMPLES_3DFX;
				_pixelFormatAttribs[15] = CVAR_INT32(L"Render_Samples");
			} else {
				Sys_LogEntry(GLRMOD, LOG_WARNING, L"Multisampling requested, but no multisampling extensions present");
			}
		}

		wglChoosePixelFormatARB(_dc, _pixelFormatAttribs, NULL, 1, &pixelFormat, &numFormats);
	}

	SetPixelFormat(_dc, pixelFormat, &_pfd);

	_wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

	return true;
}

void
GL_SwapBuffers(void)
{
	SwapBuffers(_dc);
}

void
GL_SwapInterval(int interval)
{
	if (_wglSwapIntervalEXT)
		_wglSwapIntervalEXT(interval);
}

void *
GL_InitLoadContext(void)
{
	void *ctx = NULL;
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	if (!wglCreateContextAttribsARB)
		return NULL;

	ctx = wglCreateContextAttribsARB(_dc, Re_Device.glContext, _ctxAttribs);
	if (!ctx)
		_LogError(L"Failed to load context");

	return ctx;
}

void
GL_MakeCurrent(void *ctx)
{
	wglMakeCurrent(_dc, (HGLRC)ctx);
}

void
GL_TermLoadContext(void *ctx)
{
	wglDeleteContext((HGLRC)ctx);
}

void
GL_TermDevice(void)
{
	if (Re_Device.loadLock)
		Sys_TermAtomicLock(Re_Device.loadLock);

	wglDeleteContext((HGLRC)Re_Device.glContext);

	FreeLibrary(_libGL);
}
