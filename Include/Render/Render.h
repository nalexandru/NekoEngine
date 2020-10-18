#ifndef _RE_RENDER_H_
#define _RE_RENDER_H_

#include <Engine/Types.h>

#define RE_API_VERSION	1
#define RE_NUM_BUFFERS	3
#define RE_APPEND_DATA_SIZE(a, b) (sizeof(a) - sizeof(uint8_t) + b)

#ifdef __cplusplus
extern "C" {
#endif

struct RenderInfo
{
	wchar_t name[64];
	wchar_t device[256];
	bool negativeDepth;
};

struct RenderFeatures
{
	bool rayTracing;
	bool meshShading;
	bool variableRateShading;
	bool physicallyBased;
};

struct RenderLimits
{
	uint16_t maxTextureSize;
};

struct RenderEngine
{
	void (*RenderFrame)(void);

	bool (*Init)(void);
	void (*Term)(void);

	void (*WaitIdle)(void);

	void (*ScreenResized)(void);

	bool (*InitScene)(struct Scene *scene);
	void (*TermScene)(struct Scene *scene);

	void *(*GetShader)(uint64_t hash);

	bool (*InitTexture)(const char *name, struct Texture *tex, Handle h);
	bool (*UpdateTexture)(struct Texture *tex, const void *data, uint64_t offset, uint64_t size);
	void (*TermTexture)(struct Texture *tex);

	bool (*InitModel)(const char *name, struct Model *m);
	void (*TermModel)(struct Model *m);

	size_t sceneRenderDataSize;
	size_t modelRenderDataSize;
	size_t textureRenderDataSize;

	struct RenderLimits limits;
	struct RenderFeatures features;
	struct RenderInfo info;
};

ENGINE_API extern struct RenderEngine Re;

bool Re_Init(void);
void Re_Term(void);

#ifdef __cplusplus
}
#endif

#endif /* _RE_RENDER_H_ */