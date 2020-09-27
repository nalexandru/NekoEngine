#ifndef _RE_RENDER_H_
#define _RE_RENDER_H_

#include <stdarg.h>

#include <Engine/Types.h>

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

extern struct RenderInfo Re_RenderInfo;
extern struct RenderFeatures Re_Features;

extern const size_t Re_SceneRenderDataSize;
extern const size_t Re_ModelRenderDataSize;
extern const size_t Re_TextureRenderDataSize;

bool Re_Init(void);
void Re_Term(void);

void Re_WaitIdle(void);

void Re_ScreenResized(void);
void Re_RenderFrame(void);

bool Re_InitScene(struct Scene *scene);
void Re_TermScene(struct Scene *scene);

#ifdef __cplusplus
}
#endif

#endif /* _RE_RENDER_H_ */