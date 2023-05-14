#include <Engine/IO.h>
#include <System/Log.h>
#include <Runtime/Json.h>
#include <Runtime/Runtime.h>

#include "GLBackend.h"

struct HeaderFile
{
	uint64_t hash;
	char *text;
};

struct ShaderModuleInfo
{
	uint64_t hash;
	struct GLBkShaderModule module;
};

static struct NeArray f_modules, f_shaderSource, f_headerFiles;

static void Load(const char *path);
static const char *LoadHeader(const char *path);
static void ReadShaderDef(struct NeStream *stm, struct ShaderModuleInfo *info, char *line, int64_t lineSize);

void *
Re_ShaderModule(const char *name)
{
	uint64_t hash = Rt_HashString(name);

	struct ShaderModuleInfo *info = Rt_ArrayBSearch(&f_modules, &hash, Rt_U64CmpFunc);
	if (!info)
		return NULL;

	return &info->module;
}

bool
GLBk_LoadShaders(void)
{
	if (!Rt_InitArray(&f_modules, 10, sizeof(struct ShaderModuleInfo), MH_RenderBackend))
		return false;

	if (!Rt_InitPtrArray(&f_shaderSource, 10, MH_RenderBackend))
		return false;

	if (!Rt_InitArray(&f_headerFiles, 10, sizeof(struct HeaderFile), MH_RenderBackend))
		return false;

	/*if (GLBk_contextVersion.major == 2)*/
		E_ProcessFiles("/Shaders/GL2", "glsl", true, Load);
	/*else if (GLBk_contextVersion.major == 3)
		E_ProcessFiles("/Shaders/GL3", "glsl", true, Load);
	else if (GLBk_contextVersion.major == 4)
		E_ProcessFiles("/Shaders/GL4", "glsl", true, Load);
	else
		return false;*/

	Rt_ArraySort(&f_modules, Rt_U64CmpFunc);

	/*struct HeaderFile *hf;
	Rt_ArrayForEach(hf, &f_headerFiles)
		Sys_Free(hf->text);*/

	Rt_TermArray(&f_headerFiles);
	Rt_TermArray(&f_shaderSource);

	return true;
}

void
GLBk_UnloadShaders(void)
{
	struct ShaderModuleInfo *info;
	Rt_ArrayForEach(info, &f_modules) {
		Rt_TermArray(&info->module.uniforms);
		GL_TRACE(glDeleteShader(info->module.id));
	}
	Rt_TermArray(&f_modules);
}

void
Load(const char *path)
{
	struct ShaderModuleInfo info;

	if (!Rt_InitArray(&info.module.uniforms, 4, sizeof(struct GLBkShaderUniform), MH_RenderBackend))
		return;

	char *name = Rt_TransientStrDup(path);

	GLenum type = GL_INVALID_ENUM;
	if (strstr(path, "_VS.")) {
		type = GL_VERTEX_SHADER;
	} else if (strstr(path, "_FS.")) {
		type = GL_FRAGMENT_SHADER;
	} else if (strstr(path, "_CS.")) {
		type = GL_COMPUTE_SHADER;
	} else if (strstr(path, "_GS.")) {
		type = GL_GEOMETRY_SHADER_ARB;
	}
	/*} else if (strstr(path, "_HS.glsl")) {
		type = GL_TESSELATION
	} else if (strstr(path, "_DS.glsl")) {
	}*/

	info.module.id = GL_TRACE(glCreateShader(type));
	if (!info.module.id)
		goto error;

	Rt_ClearArray(&f_shaderSource, false);
	{
		char line[256];
		struct NeStream stm;
		if (!E_FileStream(name, IO_READ, &stm))
			goto error;

		int64_t size = E_StreamLength(&stm);
		char *code = Sys_Alloc(size, 1, MH_Transient);
		if (!code) {
			E_CloseStream(&stm);
			goto error;
		}

		char *cptr = code;
		while (!E_EndOfStream(&stm)) {
			if (!E_ReadStreamLine(&stm, line, sizeof(line)))
				break;

			if (!strnlen(line, sizeof(line)))
				continue;

			if (strstr(line, "#include")) {
				char *s = strchr(line, '"'); ++s;
				char *e = strrchr(s, '"'); *e = 0x0;

				const char *text = LoadHeader(s);
				if (!text) {
					Sys_LogEntry(GLBK_MOD, LOG_CRITICAL, "Failed to read include file %s while loading %s", s, path);
					goto error;
				}

				Rt_ArrayAddPtr(&f_shaderSource, text);
			} else if (strstr(line, "#version")) {
				Rt_ArrayAddPtr(&f_shaderSource, cptr);

				size_t lLen = strnlen(line, sizeof(line));
				memcpy(cptr, line, lLen);
				cptr[lLen] = 0x0;
				cptr += lLen;
			} else if (strstr(line, "#begin_shader_def")) {
				ReadShaderDef(&stm, &info, line, sizeof(line));
			} else {
				size_t lLen = strnlen(line, sizeof(line));
				memcpy(cptr, line, lLen);
				cptr[lLen] = '\n';
				cptr += lLen;
			}
		}

		E_CloseStream(&stm);

		Rt_ArrayAddPtr(&f_shaderSource, code);
	}

	GL_TRACE(glShaderSource(info.module.id, (GLsizei)f_shaderSource.count, (const GLchar * const *)f_shaderSource.data, NULL));
	GL_TRACE(glCompileShader(info.module.id));

	GLint rc;
	GL_TRACE(glGetShaderiv(info.module.id, GL_COMPILE_STATUS, &rc));
	if (!rc) {
		GL_TRACE(glGetShaderiv(info.module.id, GL_INFO_LOG_LENGTH, &rc));

		char *infoLog = Sys_Alloc(sizeof(*infoLog), rc, MH_Transient);
		GL_TRACE(glGetShaderInfoLog(info.module.id, rc, NULL, infoLog));

		Sys_LogEntry(GLBK_MOD, LOG_CRITICAL, "Failed to compile shader %s: %s", path, infoLog);
		goto error;
	}

	name = strrchr(name, '/');
	*name++ = 0x0;

	char *ext = strrchr(name, '.');
	*ext = 0x0;

	info.hash = Rt_HashString(name);

	Rt_ArrayAdd(&f_modules, &info);

error:
	Rt_TermArray(&info.module.uniforms);
	glDeleteShader(info.module.id);
}

static const char *
LoadHeader(const char *path)
{
	uint64_t hash = Rt_HashString(path);
	struct HeaderFile *hf = Rt_ArrayFind(&f_headerFiles, &hash, Rt_U64CmpFunc);
	if (hf)
		return hf->text;

	hf = Sys_Alloc(sizeof(*hf), 1, MH_Transient);
	hf->hash = hash;

	struct NeStream stm;
	if (!E_FileStream(path, IO_READ, &stm))
		return NULL;

	int64_t size = E_StreamLength(&stm);
	hf->text = Sys_Alloc(size, 1, MH_Transient);
	if (!hf->text) {
		E_CloseStream(&stm);
		return NULL;
	}

	if (E_ReadStream(&stm, hf->text, size) != size) {
		E_CloseStream(&stm);
		return NULL;
	}

	E_CloseStream(&stm);

	Rt_ArrayAdd(&f_headerFiles, hf);
	return hf->text;
}

static void
ReadShaderDef(struct NeStream *stm, struct ShaderModuleInfo *info, char *line, int64_t lineSize)
{
	while (!E_EndOfStream(stm)) {
		if (!E_ReadStreamLine(stm, line, lineSize))
			break;

		if (!strnlen(line, lineSize))
			continue;

		struct GLBkShaderUniform *u = Rt_ArrayAllocate(&info->module.uniforms);

		if (strstr(line, "#vec3")) {
			u->type = UT_VEC3;
			u->size = sizeof(float) * 3;
		} else if (strstr(line, "#vec4")) {
			u->type = UT_VEC4;
			u->size = sizeof(float) * 4;
		} else if (strstr(line, "#mat4")) {
			u->type = UT_MAT4;
			u->size = sizeof(float) * 16;
		} else if (strstr(line, "#mat3")) {
			u->type = UT_MAT3;
			u->size = sizeof(float) * 9;
		} else if (strstr(line, "#tex2D")) {
			u->type = UT_TEX2D;
			u->size = sizeof(uint32_t);
		} else if (strstr(line, "#tex3D")) {
			u->type = UT_TEX3D;
			u->size = sizeof(uint32_t);
		} else if (strstr(line, "#float")) {
			u->type = UT_FLOAT;
			u->size = sizeof(float);
		} else if (strstr(line, "#uint")) {
			u->type = UT_UINT;
			u->size = sizeof(uint32_t);
		} else if (strstr(line, "#int")) {
			u->type = UT_INT;
			u->size = sizeof(int32_t);
		} else if (strstr(line, "#skip")) {
			u->type = UT_NONE;
			u->size = atoi(strchr(line, ' ') + 1);
			continue;
		} else if (strstr(line, "#end_shader_def")) {
			break;
		} else {
			Sys_LogEntry(GLBK_MOD, LOG_WARNING, "Unknown uniform type: %s", line);
			--info->module.uniforms.count;
			continue;
		}

		char *p = strchr(line, ' ');
		*p++ = 0x0;

		strlcpy(u->name, p, sizeof(u->name));
	}
}

/* NekoEngine
 *
 * GLShader.c
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
