#ifndef _NE_UI_UI_H_
#define _NE_UI_UI_H_

#include <UI/Text.h>
#include <Engine/Types.h>
#include <Engine/Component.h>
#include <Runtime/Runtime.h>

#define UI_UPDATE_BUFFERS			L"UI_UpdateBuffers"
#define UI_DRAW_CONTEXT			L"UI_DrawContext"

struct UIContext
{
	COMPONENT_BASE;

	struct Array vertices, indices, draws;
};

bool UI_InitUI(void);
void UI_TermUI(void);

void UI_Update(struct Scene *s);

bool UI_InitContext(struct UIContext *ctx, const void **);
void UI_TermContext(struct UIContext *ctx);

void UI_ResetContext(void **comp, void *args);
void UI_UpdateBuffers(void **comp, void *a);
void UI_DrawContext(void **comp, void *a);

#endif /* _NE_UI_UI_H_ */
