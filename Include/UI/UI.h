#ifndef _NE_UI_UI_H_
#define _NE_UI_UI_H_

#include <UI/Text.h>
#include <Engine/Types.h>
#include <Engine/Component.h>
#include <Runtime/Runtime.h>

#define UI_UPDATE_BUFFERS		"UI_UpdateBuffers"
#define UI_DRAW_CONTEXT			"UI_DrawContext"

struct NeUIContext
{
	NE_COMPONENT_BASE;

	struct NeArray vertices, indices, draws;
};

bool UI_InitUI(void);
void UI_TermUI(void);

void UI_Update(struct NeScene *s);

struct NeUIContext *UI_CreateStandaloneContext(uint32_t vertexCount, uint32_t indexCount, uint32_t drawCallCount);
void UI_DestroyStandaloneContext(struct NeUIContext *ctx);

bool UI_ResizeBuffers(uint32_t maxVertices, uint32_t maxIndices);

#endif /* _NE_UI_UI_H_ */
