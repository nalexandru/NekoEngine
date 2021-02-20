#ifndef _RE_SHADER_BINDING_TABLE_H_
#define _RE_SHADER_BINDING_TABLE_H_

struct ShaderBindingTable;

enum ShaderEntryType
{
	SET_RayGen,
	SET_Miss,
	SET_HitGroup,
	SET_Callable
};

struct ShaderBindingTable *Re_CreateShaderBindingTable(void);
void Re_SBTAddShader(struct ShaderBindingTable *sbt, enum ShaderEntryType type, struct Shader *sh);
void Re_BuildShaderBindingTable(struct ShaderBindingTable *sbt);
void Re_DestroyShaderBindingTable(struct ShaderBindingTable *sbt);

#endif /* _RE_SHADER_BINDING_TABLE_H_ */
