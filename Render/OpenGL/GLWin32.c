#include <Windows.h>

#include <System/Log.h>
#include <Engine/Engine.h>
#include <Engine/Config.h>
#include <Render/Render.h>
#include <Render/Device.h>

#include "GLRender.h"
#include <GL/wglext.h>

#define GLRMOD	L"OpenGLWin32"

static HDC _dc;
static HMODULE _libGL;
static PIXELFORMATDESCRIPTOR _pfd =
{
	sizeof(PIXELFORMATDESCRIPTOR),
	1,
	PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
	PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
	32,                   // Colordepth of the framebuffer.
	0, 0, 0, 0, 0, 0,
	0,
	0,
	0,
	0, 0, 0, 0,
	24,                   // Number of bits for the depthbuffer
	8,                    // Number of bits for the stencilbuffer
	0,                    // Number of Aux buffers in the framebuffer.
	PFD_MAIN_PLANE,
	0,
	0, 0, 0
};

static int _pixelFormatAttribs[] =
{
	WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
	WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
	WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
	WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
	WGL_COLOR_BITS_ARB, 32,
	WGL_DEPTH_BITS_ARB, 24,
	0, 0,
	0, 0,
	0
};

static int _ctxAttribs[] =
{
	WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
	WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
	0, 0,
	0, 0,
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

		return strstr(ext, extension) != NULL;
	}
}

static inline void
_LogError(const wchar_t *message)
{
	LPWSTR buffer;

	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&buffer, 0, NULL);

	Sys_LogEntry(GLRMOD, LOG_CRITICAL, L"%s: %s", message, buffer);

	LocalFree(buffer);
}

bool
GL_InitDevice(void)
{
	int pixelFormat;
	UINT numFormats;
	HGLRC dummy;
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;

	_libGL = LoadLibraryW(L"opengl32");
	if (!_libGL)
		return false;

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
		_ctxAttribs[3] |= WGL_CONTEXT_DEBUG_BIT_ARB;

	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	if (wglCreateContextAttribsARB) {
		wglMakeCurrent(NULL, NULL);
		
		// attempt to create a forward-compatible core profile context first
		Re_Device.glContext = wglCreateContextAttribsARB(_dc, NULL, _ctxAttribs);
		if (!Re_Device.glContext) {
			memset(_ctxAttribs, 0x0, sizeof(_ctxAttribs));
			Re_Device.glContext = wglCreateContextAttribsARB(_dc, NULL, _ctxAttribs);
		}

		if (Re_Device.glContext) {
			wglDeleteContext(dummy);
			dummy = NULL;
		} else {
			_LogError(L"wglCreateContextAttribsARB failed");
			Re_Device.glContext = dummy;
		}
	} else {
		// Use the already created OpenGL context
		Re_Device.glContext = dummy;
	}

	if (!wglMakeCurrent(_dc, Re_Device.glContext)) {
		_LogError(L"Failed to activate context");
		wglDeleteContext(Re_Device.glContext);
		return false;
	}

	if (Re_Device.glContext == dummy) {
		Re_Device.loadLock = Sys_AlignedAlloc(sizeof(*Re_Device.loadLock), 16);
		Sys_InitAtomicLock(Re_Device.loadLock);
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

void
GL_ScreenResized(void)
{
	//
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
		Sys_AlignedFree(Re_Device.loadLock);

	wglDeleteContext((HGLRC)Re_Device.glContext);

	FreeLibrary(_libGL);
}
