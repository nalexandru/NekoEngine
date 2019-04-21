/* NekoEngine
 *
 * gl_gfx_win32.c
 * Author: Alexandru Naiman
 *
 * NekoEngine Win32 OpenGL Graphics Subsystem
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "glad.h"
#include <GL/wglext.h>

#include "debug.h"
#include "gl_gfx.h"

PIXELFORMATDESCRIPTOR pfd =
{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		0,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
};

static HDC _dc = INVALID_HANDLE_VALUE;
static HGLRC _ctx = INVALID_HANDLE_VALUE;
static HGLRC _load_ctx = INVALID_HANDLE_VALUE;
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;

ne_status
gfx_sys_init(bool debug)
{
	HWND window;
	HGLRC dummy = INVALID_HANDLE_VALUE;
	int fmt = 0, p_fmt = 0;
	unsigned n_fmt = 0;

	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;
	PFNGLDEBUGMESSAGECALLBACKPROC _glDebugMessageCallback = NULL;
	PFNGLDEBUGMESSAGECALLBACKARBPROC _glDebugMessageCallbackARB = NULL;
	PFNGLDEBUGMESSAGECALLBACKAMDPROC _glDebugMessageCallbackAMD = NULL;

	const int pfa[] =
	{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 0,
		WGL_SAMPLE_BUFFERS_ARB, 0,
		WGL_SAMPLES_ARB, 1,
		0
	};

	int attr[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 6,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		0
	};

	window = GetActiveWindow();
	_dc = GetDC(window);
	fmt = ChoosePixelFormat(_dc, &pfd);

	if (!fmt)
		return NE_NO_PIXEL_FMT;

	if (!SetPixelFormat(_dc, fmt, &pfd))
		return NE_NO_PIXEL_FMT;

	dummy = wglCreateContext(_dc);
	if (!dummy)
		return NE_GFX_CTX_CREATE_FAIL;

	if (!wglMakeCurrent(_dc, dummy)) {
		wglDeleteContext(dummy);
		return NE_GFX_CTX_CREATE_FAIL;
	}

	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
		wglGetProcAddress("wglCreateContextAttribsARB");
	if (!wglCreateContextAttribsARB) {
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(dummy);

		return NE_GFX_CTX_CREATE_FAIL;
	}

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(dummy);

	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)
		wglGetProcAddress("wglChoosePixelFormatARB");
	if (wglChoosePixelFormatARB) {
		wglChoosePixelFormatARB(_dc, pfa, NULL, 1, &p_fmt, &n_fmt);
		SetPixelFormat(_dc, p_fmt, &pfd);
	}

	if (debug)
		attr[8] |= WGL_CONTEXT_DEBUG_BIT_ARB;
	/*else
		attr[8] |= GL_KHR_no_error;*/

	_ctx = wglCreateContextAttribsARB(_dc, NULL, attr);
	if (!_ctx)
		return NE_GFX_CTX_CREATE_FAIL;

	_load_ctx = wglCreateContextAttribsARB(_dc, _ctx, attr);
	if (!_load_ctx) {
		wglDeleteContext(_ctx);
		return NE_GFX_CTX_CREATE_FAIL;
	}

	wglMakeCurrent(_dc, _ctx);

	if (!gladLoadGL())
		return NE_GFX_GL_LOAD_FAIL;

	if (debug) {
		_glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)
			wglGetProcAddress("glDebugMessageCallback");
		_glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)
			wglGetProcAddress("glDebugMessageCallbackARB");
		_glDebugMessageCallbackAMD = (PFNGLDEBUGMESSAGECALLBACKAMDPROC)
			wglGetProcAddress("glDebugMessageCallbackAMD");

		if (_glDebugMessageCallback != NULL)
			_glDebugMessageCallback(_gfx_dbg_cb, NULL);
		else if (_glDebugMessageCallbackARB != NULL)
			_glDebugMessageCallbackARB(_gfx_dbg_cb, NULL);
		else if (_glDebugMessageCallbackAMD, NULL)
			_glDebugMessageCallbackAMD(_gfx_dbg_cb_amd, NULL);
		else
			OutputDebugString("OpenGL debug callback not available.\n");

		if ((_glDebugMessageCallback != NULL) ||
			(_glDebugMessageCallbackARB != NULL))
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}

	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
		wglGetProcAddress("wglSwapIntervalEXT");

	return NE_OK;
}

void
gfx_sys_swap_interval(int swi)
{
	wglSwapIntervalEXT(swi);
}

void
gfx_sys_swap_buffers(void)
{
	SwapBuffers(_dc);
}

void
gfx_sys_destroy(void)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(_ctx);
	DeleteDC(_dc);
}
