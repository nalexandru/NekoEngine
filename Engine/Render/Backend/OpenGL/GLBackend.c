#include <stdio.h>
#include <stdlib.h>

#include <Engine/XR.h>
#include <Engine/Job.h>
#include <System/Log.h>
#include <System/Memory.h>
#include <Runtime/Runtime.h>
#include <Engine/Config.h>
#include <Engine/Version.h>
#include <Engine/Application.h>
#include <Render/Backend.h>
#include <Render/Device.h>

#include "GLBackend.h"

const char *Re_backendName = "OpenGL";
struct GLBkVersion GLBk_contextVersion = { 0, 0 };

bool
Re_InitBackend(void)
{
	if (!GLBk_InitContext())
		return false;

	glGetError(); // clear any errors that resulted from gladLoadGL()

	const GLubyte *glVersion = GL_TRACE(glGetString(GL_VERSION));
	sscanf((const char *)glVersion, "%d.%d", &GLBk_contextVersion.major, &GLBk_contextVersion.minor);

	Sys_LogEntry(GLBK_MOD, LOG_INFORMATION, "OpenGL context version %d.%d", GLBk_contextVersion.major, GLBk_contextVersion.minor);

	if (GLBk_contextVersion.major < 2) {
		Sys_MessageBox("Unsupported hardware", "A video card with OpenGL 2.0 support is required.", MSG_ICON_ERROR);
		return false;
	}

	if (CVAR_BOOL("OpenGL_DisableBindlessTextures"))
		GLAD_GL_ARB_bindless_texture = false;

	if (CVAR_BOOL("OpenGL_DisableDirectStateAccess"))
		GLAD_GL_ARB_direct_state_access = false;

	if (CVAR_BOOL("OpenGL_DisableMeshShader"))
		GLAD_GL_NV_mesh_shader = false;

	if (CVAR_BOOL("OpenGL_DisableComputeShader"))
		GLAD_GL_ARB_compute_shader = false;

	if (CVAR_BOOL("OpenGL_DisableTextureStorage"))
		GLAD_GL_ARB_texture_storage = false;

	return true;
}

void
Re_TermBackend(void)
{
	GLBk_TermContext();
}

bool
Re_EnumerateDevices(uint32_t *count, struct NeRenderDeviceInfo *info)
{
	if (!*count || !info) {
		*count = 1;
		return true;
	}

	const GLubyte *renderer = GL_TRACE(glGetString(GL_RENDERER));
	snprintf(info[0].deviceName, sizeof(info[0].deviceName), "%s", renderer);

	info[0].limits.maxPushConstantsSize = 256;

	GLint val;
	GL_TRACE(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &val));
	info[0].limits.maxTextureSize = val;

	info[0].features.canPresent = true;

	info[0].features.rayTracing = false;
	info[0].features.indirectRayTracing = false;
	info[0].features.directIO = false;

	// depends on extensions
	info[0].features.meshShading = GLAD_GL_NV_mesh_shader;
	info[0].features.coherentMemory = GLBk_contextVersion.major >= 4 && GLBk_contextVersion.minor >= 4;
	info[0].features.drawIndirectCount = false;
	info[0].features.multiDrawIndirect = GLAD_GL_ARB_multi_draw_indirect;
	info[0].features.bcTextureCompression = GLAD_GL_ARB_texture_compression_bptc;
	info[0].features.astcTextureCompression = GLAD_GL_KHR_texture_compression_astc_hdr && GLAD_GL_KHR_texture_compression_astc_ldr;

	// TODO
	info[0].features.discrete = true;
	info[0].features.unifiedMemory = false;
	info[0].features.secondaryCommandBuffers = false;

	GLBk_HardwareInfo(&info[0]);

	if (GLAD_GL_NVX_gpu_memory_info) {
		GLint vram;
		glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &vram);
		info[0].localMemorySize = vram / 1024;
	}

	return true;
}

struct NeSurface *
Re_CreateSurface(void *window)
{
	return (struct NeSurface *)1;
}

void
Re_DestroySurface(struct NeSurface *surface)
{
}

void
Re_WaitIdle(void)
{
	if (glFenceSync) {
		GLsync sync = GL_TRACE(glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));

		while (1) {
			GLenum rc = GL_TRACE(glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, UINT64_MAX));
			if (rc == GL_ALREADY_SIGNALED || rc == GL_CONDITION_SATISFIED)
				break;
		}

		GL_TRACE(glDeleteSync(sync));
	}
}

struct NeRenderDevice *Re_CreateDevice(struct NeRenderDeviceInfo *info) { return (struct NeRenderDevice *)GLBk_LoadShaders(); }
void Re_DestroyDevice(struct NeRenderDevice *dev) { GLBk_UnloadShaders(); }

/* NekoEngine
 *
 * GLBackend.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
