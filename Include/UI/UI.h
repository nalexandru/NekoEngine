#ifndef _UI_UI_H_
#define _UI_UI_H_

#include <UI/Text.h>
#include <Engine/Types.h>
#include <Engine/Component.h>
#include <Runtime/Runtime.h>

#ifdef __cplusplus
extern "C" {
#endif

struct UIVertex
{
	float posUv[4];
	float color[4];
};

struct UIDrawCall
{
	uint16_t vtxOffset;
	uint16_t vtxCount;
	uint16_t idxOffset;
	uint16_t idxCount;
	Handle texture;
};

struct UIContext
{
	COMPONENT_BASE;

	Array vertices, indices, draws;
};

ENGINE_API extern struct mat4 UI_Projection;

bool UI_InitUI();
void UI_TermUI();

bool UI_InitContext(struct UIContext *ctx, const void **);
void UI_TermContext(struct UIContext *ctx);

void UI_ResetContext(void **comp, void *args);

#ifdef __cplusplus
}
#endif

#endif /* _UI_UI_H_ */
