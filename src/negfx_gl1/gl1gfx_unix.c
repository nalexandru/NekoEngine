/* NekoEngine
 *
 * gl1gfx_unix.c
 * Author: Alexandru Naiman
 *
 * NekoEngine UNIX (X11) OpenGL 1 Graphics Subsystem
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

#include <stdio.h>

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include <gl1gfx.h>

extern Display *x_display;
extern Window x11_active_window;
static GLXContext _ctx;
static GLXContext _load_ctx;
static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = NULL;

ne_status
gfx_sys_init(bool debug)
{
	Window *window;
	GLXContext dummy;
	int fmt = 0, p_fmt = 0, glx_major = 0, glx_minor = 0, fbcount = 0;
        int best_fbc = -1, worst_fbc = -1, best_num_samp = -1;
	int worst_num_samp = 999;
	int samp_buf = 0, samples = 0;
	unsigned n_fmt = 0, i = 0;
        GLXFBConfig* fbc = NULL;
	XVisualInfo *vi = NULL;
	GLXFBConfig selected_fbc;

	PFNGLXCHOOSEFBCONFIGPROC glXChooseFBConfig = NULL;
	PFNGLXGETVISUALFROMFBCONFIGPROC glXGetVisualFromFBConfig = NULL;
	PFNGLXGETFBCONFIGATTRIBPROC glXGetFBConfigAttrib = NULL;

	static int visual_attribs[] =
        {
                GLX_X_RENDERABLE    , True,
                GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
                GLX_RENDER_TYPE     , GLX_RGBA_BIT,
                GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
                GLX_RED_SIZE        , 8,
                GLX_GREEN_SIZE      , 8,
                GLX_BLUE_SIZE       , 8,
                GLX_ALPHA_SIZE      , 0,
                GLX_DEPTH_SIZE      , 24,
                GLX_STENCIL_SIZE    , 0,
                GLX_SAMPLE_BUFFERS  , 0,
                GLX_SAMPLES         , 0,
                0
        };

	if (!glXQueryVersion(x_display, &glx_major, &glx_minor) ||
		((glx_major == 1) && (glx_minor < 3)) ||
		(glx_major < 1)) {
		//
		return NE_FAIL;
	}

	glXChooseFBConfig = (PFNGLXCHOOSEFBCONFIGPROC)
		glXGetProcAddress((const GLubyte*)"glXChooseFBConfig");
        glXGetVisualFromFBConfig = (PFNGLXGETVISUALFROMFBCONFIGPROC)
		glXGetProcAddress((const GLubyte*)"glXGetVisualFromFBConfig");
        glXGetFBConfigAttrib = (PFNGLXGETFBCONFIGATTRIBPROC)
		glXGetProcAddress((const GLubyte*)"glXGetFBConfigAttrib");

	if (glXChooseFBConfig == NULL ||
                glXGetVisualFromFBConfig == NULL ||
                glXGetFBConfigAttrib == NULL)
        {
                //Platform::MessageBox("Fatal Error", "Unable to load required GLX functions", MessageBoxButtons::OK, MessageBoxIcon::Error);
                return NE_FAIL;
        }

        fbc = glXChooseFBConfig(x_display, DefaultScreen(x_display), visual_attribs, &fbcount);
        if (!fbc)
                return NE_FAIL;

        for (i = 0; i < fbcount; ++i)
        {
                vi = glXGetVisualFromFBConfig(x_display, fbc[i]);
                if (vi) {
                        glXGetFBConfigAttrib(x_display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
                        glXGetFBConfigAttrib(x_display, fbc[i], GLX_SAMPLES, &samples);

                        if (best_fbc < 0 || (samp_buf && (samples > best_num_samp)))
                                best_fbc = i, best_num_samp = samples;
                        if (worst_fbc < 0 || !samp_buf || (samples < worst_num_samp))
                                worst_fbc = i, worst_num_samp = samples;
                }
                XFree(vi);
        }

	selected_fbc = fbc[best_fbc];
	XFree(fbc);

	vi = glXGetVisualFromFBConfig(x_display, selected_fbc);

	_ctx = glXCreateContext(x_display, vi, NULL, True);
	if (!_ctx)
		return NE_GFX_CTX_CREATE_FAIL;

	XFree(vi);

	XSync(x_display, False);

	glXMakeCurrent(x_display, x11_active_window, _ctx);

	glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)
		glXGetProcAddress("glXSwapIntervalEXT");

	return NE_OK;
}

void
gfx_sys_swap_interval(int swi)
{
	glXSwapIntervalEXT(x_display, x11_active_window, swi);
}

void
gfx_sys_swap_buffers(void)
{
	glXSwapBuffers(x_display, x11_active_window);
}

void
gfx_sys_destroy(void)
{
	glXMakeCurrent(NULL, 0, 0);
	glXDestroyContext(x_display, _load_ctx);
	glXDestroyContext(x_display, _ctx);
}
