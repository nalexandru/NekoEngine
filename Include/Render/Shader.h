#ifndef _RE_SHADER_H_
#define _RE_SHADER_H_

#include <Engine/Types.h>
#include <Render/Device.h>

struct ShaderStageDesc
{
	enum ShaderStage stage;
	void *module;
};

struct Shader
{
	uint64_t hash;
	enum ShaderType type;
	uint32_t stageCount;
	struct ShaderStageDesc *stages;
	char name[256];
};

bool Re_LoadShaders(void);
void Re_UnloadShaders(void);

struct Shader *Re_GetShader(const char *name);

#endif /* _RE_SHADER_H_ */
