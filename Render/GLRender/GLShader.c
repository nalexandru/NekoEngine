#include <Engine/IO.h>
#include <Engine/Config.h>
#include <System/Log.h>
#include <Runtime/Json.h>
#include <System/Memory.h>
#include <Render/Shader.h>
#include <Render/Device.h>

#include "GLRender.h"

#define GLSMOD	L"GLShader"

Array _shaders;

struct ShaderModule
{
	GLuint shader;
	GLenum stage;
};

static uint64_t _defaultHash;
static void _LoadProgram(const char *path);
static GLuint _LoadShader(const char *path, GLenum stage);
static int32_t _SortShaders(const struct Shader *a, const struct Shader *b);
static int32_t _CompShaders(const struct Shader *m, const uint64_t *hash);

void *
GL_GetShader(uint64_t hash)
{
	void *ret = NULL;

	if (!GL_ShaderSupport)
		return NULL;
		
	ret = Rt_ArrayBSearch(&_shaders, &hash, (RtCmpFunc)_CompShaders);
	if (!ret)
		ret = Rt_ArrayBSearch(&_shaders, &_defaultHash, (RtCmpFunc)_CompShaders);

	return ret;
}

bool
GL_LoadShaders(void)
{
	if (Re_Device.verMajor == 1 || CVAR_BOOL(L"GL_DisableShaders"))
		return true;

	GL_ShaderSupport = true;
	
	if (Re_Device.verMajor == 1) {
		glCreateShader = (PFNGLCREATESHADERPROC)glCreateShaderObjectARB;
		glShaderSource = (PFNGLSHADERSOURCEPROC)glShaderSourceARB;
		glCompileShader = (PFNGLCOMPILESHADERPROC)glCompileShaderARB;
		glGetShaderiv = (PFNGLGETPROGRAMIVPROC)glGetObjectParameterivARB;
		glGetShaderInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)glGetInfoLogARB;

		glCreateProgram = (PFNGLCREATEPROGRAMPROC)glCreateProgramObjectARB;
		glAttachShader = (PFNGLATTACHSHADERPROC)glAttachObjectARB;
		glLinkProgram = (PFNGLLINKPROGRAMPROC)glLinkProgramARB;
		glGetProgramiv = (PFNGLGETPROGRAMIVPROC)glGetObjectParameterivARB;
		glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)glGetInfoLogARB;

		glUseProgram = (PFNGLUSEPROGRAMPROC)glUseProgramObjectARB;
		
		glUniform1i = glUniform1iARB;
		glUniformMatrix4fv = glUniformMatrix4fvARB;
		
		glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)glGetUniformLocationARB;
		
		glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)glGetAttribLocationARB;
		glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)glBindAttribLocationARB;
	}
	
	Rt_InitArray(&_shaders, 10, sizeof(struct Shader));
	E_ProcessFiles("/Shaders/GL", "shader", true, _LoadProgram);

	Rt_ArraySort(&_shaders, (RtSortFunc)_SortShaders);

	_defaultHash = Rt_HashStringW(L"Default");

	return true;
}

void
GL_UnloadShaders(void)
{
	size_t i;
	struct Shader *s;
	
	if (!GL_ShaderSupport)
		return;

	if (glDeleteShader && glDeleteProgram)
		for (i = 0; i < _shaders.count; ++i) {
			s = Rt_ArrayGet(&_shaders, i);

			for (i = 0; i < GL_SHADER_COUNT; ++i)
				if (s->shaders[i])
					glDeleteShader(s->shaders[i]);

			glDeleteProgram(s->program);
		}
		
	Rt_TermArray(&_shaders);
}

static void
_LoadProgram(const char *path)
{
	int32_t i = 0, j = 0;
	struct Shader *s;
	File f;
	int64_t size;
	char *data, *shPath;
	int32_t tokCount;
	jsmntok_t *tokens, tok;
	jsmn_parser p;
	wchar_t *buff;
	GLenum stage = GL_INVALID_ENUM;

	s = Rt_ArrayAllocate(&_shaders);

	f = E_OpenFile(path, IO_READ);
	data = E_ReadFileText(f, &size, true);
	E_CloseFile(f);

	jsmn_init(&p);
	tokCount = jsmn_parse(&p, data, (size_t)size, NULL, 0);
	tokens = Sys_Alloc(sizeof(jsmntok_t), tokCount, MH_Transient);
	
	jsmn_init(&p);
	jsmn_parse(&p, data, (size_t)size, tokens, tokCount);

	if (tokens[0].type != JSMN_OBJECT) {
		Sys_LogEntry(GLSMOD, LOG_WARNING, L"Invalid shader definition file %S", path);
		return;
	}

	buff = Sys_Alloc(sizeof(wchar_t), 256, MH_Transient);
	shPath = Sys_Alloc(sizeof(char), 256, MH_Transient);

	for (i = 1; i < tokCount; ++i) {
		tok = tokens[i];

		if (JSON_STRING("name", tok, data)) {
			tok = tokens[++i];
			mbstowcs(buff, data + tok.start, (size_t)tok.end - (size_t)tok.start);
		} else if (JSON_STRING("vertex", tok, data)) {
			tok = tokens[++i];
			stage = GL_VERTEX_SHADER;
		} else if (JSON_STRING("fragment", tok, data)) {
			tok = tokens[++i];
			stage = GL_FRAGMENT_SHADER;
	#ifdef GL_GEOMETRY_SHADER
		} else if (JSON_STRING("geometry", tok, data)) {
			tok = tokens[++i];
			stage = GL_GEOMETRY_SHADER;
	#endif
	#ifdef GL_TESS_CONTROL_SHADER
		} else if (JSON_STRING("tessControl", tok, data)) {
			tok = tokens[++i];
			stage = GL_TESS_CONTROL_SHADER;
	#endif
	#ifdef GL_TESS_EVALUATION_SHADER
		} else if (JSON_STRING("tessEval", tok, data)) {
			tok = tokens[++i];
			stage = GL_TESS_EVALUATION_SHADER;
	#endif
		}
		
		if (stage != GL_INVALID_ENUM) {
			snprintf(shPath, 13 + (size_t)tok.end - (size_t)tok.start, "/Shaders/GL/%s", data + tok.start);
			s->shaders[j++] = _LoadShader(shPath, stage);
		}

		stage = GL_INVALID_ENUM;
	}

	s->hash = Rt_HashStringW(buff);
	s->program = glCreateProgram();

	for (i = 0; i < GL_SHADER_COUNT; ++i)
		if (s->shaders[i])
			glAttachShader(s->program, s->shaders[i]);

	glLinkProgram(s->program);
	glGetProgramiv(s->program, GL_LINK_STATUS, &i);
	
	if (!i)
		goto LinkError;
	
	if (Re_Device.verMajor < 3 || (Re_Device.verMajor == 3 && Re_Device.verMinor < 3)) {
		if (glGetAttribLocation(s->program, "a_position") != -1) {
			glBindAttribLocation(s->program, 0, "a_position");
			glBindAttribLocation(s->program, 1, "a_normal");
			glBindAttribLocation(s->program, 2, "a_tangent");
			glBindAttribLocation(s->program, 3, "a_uv");
		} else if (glGetAttribLocation(s->program, "a_posUv") != -1) {
			glBindAttribLocation(s->program, 0, "a_posUv");
			glBindAttribLocation(s->program, 1, "a_color");
		}

		glLinkProgram(s->program);
		glGetProgramiv(s->program, GL_LINK_STATUS, &i);

		if (!i)
			goto LinkError;
	}

	return;

LinkError:
	glGetProgramiv(s->program, GL_INFO_LOG_LENGTH, &i);

	data = Sys_Alloc(sizeof(char), i, MH_Transient);
	glGetProgramInfoLog(s->program, i, NULL, data);

	Sys_LogEntry(GLSMOD, LOG_CRITICAL, L"Failed to link program %hs: %hs", path, data);

	for (i = 0; i < GL_SHADER_COUNT; ++i)
		if (s->shaders[i])
			glDeleteShader(s->shaders[i]);

	glDeleteProgram(s->program);

	memset(s, 0x0, sizeof(*s));
	--_shaders.count;
}

static GLuint
_LoadShader(const char *path, GLenum stage)
{
	GLuint s;
	char *source;
	int64_t sourceSize;
	GLint len = 0;
	File f;

	f = E_OpenFile(path, IO_READ);
	if (!f)
		return 0;

	source = E_ReadFileText(f, &sourceSize, true);
	E_CloseFile(f);

	if (!(s = glCreateShader(stage)))
		return 0;
	
	len = (GLint)sourceSize;
	glShaderSource(s, 1, (const GLchar **)&source, &len);
	glCompileShader(s);
	glGetShaderiv(s, GL_COMPILE_STATUS, &len);
	
	if (!len) {
		glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);

		source = Sys_Alloc(sizeof(char), len, MH_Transient);
		glGetShaderInfoLog(s, len, NULL, source);

		Sys_LogEntry(GLSMOD, LOG_CRITICAL, L"Failed to compile shader %hs: %hs", path, source);

		if (glDeleteShader)
			glDeleteShader(s);
	
		return 0;
	}

	return s;
}

static int32_t
_SortShaders(const struct Shader *a, const struct Shader *b)
{
	return a->hash > b->hash ? 1 : (a->hash < b->hash ? -1 : 0);
}

static int32_t
_CompShaders(const struct Shader *m, const uint64_t *hash)
{
	return m->hash > *hash ? 1 : (m->hash < *hash ? -1 : 0);
}

