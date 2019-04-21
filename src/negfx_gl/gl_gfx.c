/* NekoEngine
 *
 * gl_gfx.c
 * Author: Alexandru Naiman
 *
 * NekoEngine OpenGL Graphics Subsystem
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

#include <system/log.h>
#include <system/config.h>
#include <engine/resource.h>
#include <graphics/texture.h>
#include <graphics/graphics.h>

#include "glad.h"
#include "gl_gfx.h"

#define GL_GFX_MODULE	"OpenGL_Graphics"

GLuint _color_tex = 0,
	_normal_tex = 0,
	_depth_stencil_tex = 0,
	_brightness_tex = 0;
GLuint64 _color_tex_h = 0,
	_normal_tex_h = 0,
	_depth_stencil_tex_h = 0,
	_brightness_tex_h = 0;
GLuint _fbo = 0,
	_depth_fbo = 0;

uint16_t _width = 0,
	_height = 0;

ne_status gl_gfx_init(bool);
void gl_gfx_draw(void);
void gl_gfx_destroy(void);
void gl_gfx_screen_resized(uint16_t, uint16_t);
void gl_gfx_wait_idle(void) { }

ne_gfx_module _gl_gfx_module =
{
	NE_GFX_API_VER,
	gl_gfx_init,
	gl_gfx_draw,
	gl_gfx_screen_resized,
	gfx_sys_swap_interval,
	gl_gfx_wait_idle,
	gl_gfx_destroy
};

const ne_gfx_module *
create_gfx_module(void)
{
	return &_gl_gfx_module;
}

ne_status
_gfx_init_textures(void)
{
	GLuint samples = 4;
	GLuint int_fmt = GL_RGBA32F;
	GLuint tex_type = GL_TEXTURE_2D;

	if (samples > 1)
		tex_type = GL_TEXTURE_2D_MULTISAMPLE;

	glCreateTextures(tex_type, 1, &_color_tex);
	glCreateTextures(tex_type, 1, &_normal_tex);
	glCreateTextures(tex_type, 1, &_depth_stencil_tex);
	glCreateTextures(tex_type, 1, &_brightness_tex);

	if (samples > 1) {
		glTextureStorage2DMultisample(_color_tex,
			samples, int_fmt, _width, _height, 1);
		glTextureStorage2DMultisample(_normal_tex,
			samples, int_fmt, _width, _height, 1);
		glTextureStorage2DMultisample(_depth_stencil_tex,
			samples, GL_DEPTH24_STENCIL8, _width, _height, 1);
		glTextureStorage2DMultisample(_brightness_tex,
			samples, int_fmt, _width, _height, 1);
	} else {
	}

	_color_tex_h = glGetTextureHandleARB(_color_tex);
	glMakeTextureHandleResidentARB(_color_tex_h);

	_normal_tex_h = glGetTextureHandleARB(_normal_tex);
	glMakeTextureHandleResidentARB(_normal_tex_h);

	_depth_stencil_tex_h = glGetTextureHandleARB(_depth_stencil_tex);
	glMakeTextureHandleResidentARB(_depth_stencil_tex_h);

	_brightness_tex_h = glGetTextureHandleARB(_brightness_tex);
	glMakeTextureHandleResidentARB(_brightness_tex_h);

	return NE_OK;
}

ne_status
_gfx_init_framebuffers(void)
{
	glCreateFramebuffers(1, &_fbo);
	glCreateFramebuffers(1, &_depth_fbo);

	return NE_OK;
}

ne_status
_gfx_attach_textures(void)
{
	GLenum status;

	glNamedFramebufferTexture(_fbo, GL_COLOR_ATTACHMENT0,
		_color_tex, 0);
	glNamedFramebufferTexture(_fbo, GL_COLOR_ATTACHMENT1,
		_normal_tex, 0);
	glNamedFramebufferTexture(_fbo, GL_COLOR_ATTACHMENT2,
		_brightness_tex, 0);
	glNamedFramebufferTexture(_fbo, GL_DEPTH_STENCIL_ATTACHMENT,
		_depth_stencil_tex, 0);

	//glNamedFramebufferDrawBuffers(_fbo, 1, 

	status = glCheckNamedFramebufferStatus(_fbo, GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		return NE_FAIL;

	glNamedFramebufferTexture(_depth_fbo, GL_DEPTH_STENCIL_ATTACHMENT,
		_depth_stencil_tex, 0);

	return status == GL_FRAMEBUFFER_COMPLETE ? NE_OK : NE_FAIL;
}

void
_gfx_destroy_textures(void)
{
	glMakeTextureHandleNonResidentARB(_color_tex_h);
	glDeleteTextures(1, &_brightness_tex);

	glMakeTextureHandleNonResidentARB(_normal_tex_h);
	glDeleteTextures(1, &_normal_tex);

	glMakeTextureHandleNonResidentARB(_depth_stencil_tex_h);
	glDeleteTextures(1, &_depth_stencil_tex);

	glMakeTextureHandleNonResidentARB(_brightness_tex_h);
	glDeleteTextures(1, &_brightness_tex);
}

void
_gfx_destroy_framebuffers(void)
{
	glDeleteFramebuffers(1, &_depth_fbo);
	glDeleteFramebuffers(1, &_fbo);
}

ne_status
gl_gfx_init(bool debug)
{
	GLint ver_major = 0, ver_minor = 0;
	ne_status ret = NE_FAIL;

	log_entry(GL_GFX_MODULE, LOG_INFORMATION, "Initializing...");

	_width = (uint16_t)sys_config_get_int("width", 0);
	_height = (uint16_t)sys_config_get_int("height", 0);

	if ((ret = gfx_sys_init(debug)) != NE_OK)
		return ret;

	glGetIntegerv(GL_MAJOR_VERSION, &ver_major);
	glGetIntegerv(GL_MINOR_VERSION, &ver_minor);

	log_entry(GL_GFX_MODULE, LOG_INFORMATION, "Context: OpenGL %d.%d", ver_major, ver_minor);
	log_entry(GL_GFX_MODULE, LOG_INFORMATION, "Vendor: %s", glGetString(GL_VENDOR));
	log_entry(GL_GFX_MODULE, LOG_INFORMATION, "Renderer: %s", glGetString(GL_RENDERER));
	log_entry(GL_GFX_MODULE, LOG_INFORMATION, "Version: %s", glGetString(GL_VERSION));
	log_entry(GL_GFX_MODULE, LOG_INFORMATION, "Shading Language: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	_gfx_init_textures();
	_gfx_init_framebuffers();
	_gfx_attach_textures();

	glClearColor(.8f, 0.f, .6f, 1.f);

	return register_systems();
}

static inline void
_gfx_z_prepass(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _depth_fbo);

	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// get visible drawables from scene
	// render depth
}

static inline void
_gfx_lighting_pass(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glClear(GL_COLOR_BUFFER_BIT);
}

static inline void
_gfx_postprocess(void)
{
	//
}

void
gl_gfx_draw(void)
{
	_gfx_z_prepass();
	_gfx_lighting_pass();
	_gfx_postprocess();

	// Blit

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBlitNamedFramebuffer(_fbo, 0,
		0, 0, _width, _height,
		0, 0, _width, _height,
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
		GL_NEAREST);

	gfx_sys_swap_buffers();
}

void
gl_gfx_screen_resized(uint16_t width,
	uint16_t height)
{
	_width = width;
	_height = height;

	// recreate textures
	_gfx_destroy_textures();
	_gfx_init_textures();

	// reattach textures
	_gfx_attach_textures();

	// set framebuffer viewports
	glBindFramebuffer(GL_FRAMEBUFFER, _depth_fbo);
	glViewport(0, 0, _width, _height);

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glViewport(0, 0, _width, _height);
}

void
gl_gfx_destroy(void)
{
	log_entry(GL_GFX_MODULE, LOG_INFORMATION, "Shutting down...");

	_gfx_destroy_framebuffers();
	_gfx_destroy_textures();

	gfx_sys_destroy();

	log_entry(GL_GFX_MODULE, LOG_INFORMATION, "Shut down complete");
}
