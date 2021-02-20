#ifndef _RE_SHADER_H_
#define _RE_SHADER_H_

enum ShaderType
{
	ST_GRAPHICS,
	ST_MESH,
	ST_COMPUTE,
	ST_RAY_TRACING
};

enum ShaderStage
{
	SS_VERTEX		= 0x00000001,
	SS_TESS_CTRL	= 0x00000002,
	SS_TESS_EVAL	= 0x00000004,
	SS_GEOMETRY		= 0x00000008,
	SS_FRAGMENT		= 0x00000010,
	SS_COMPUTE		= 0x00000020,
	SS_ALL_GRAPHICS	= 0x0000001F,
	SS_RAYGEN		= 0x00000100,
	SS_ANY_HIT		= 0x00000200,
	SS_CLOSEST_HIT	= 0x00000400,
	SS_MISS			= 0x00000800,
	SS_INTERSECTION = 0x00001000,
	SS_CALLABLE		= 0x00002000,
	SS_TASK			= 0x00000040,
	SS_MESH			= 0x00000080
};

static inline bool Re_LoadShaders(void);
static inline void Re_UnloadShaders(void);

static inline struct Shader *Re_GetShader(const wchar_t *name, enum ShaderType type);

#endif /* _RE_SHADER_H_ */
