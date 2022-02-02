#ifndef _NE_UI_INTERNAL_H_
#define _NE_UI_INTERNAL_H_

#include <UI/UI.h>
#include <UI/Font.h>
#include <UI/Text.h>
#include <Runtime/Runtime.h>

#pragma pack(push,1)
struct NeUIVertex
{
	float posUv[4];
	float color[4];
};
#pragma pack(pop)

struct NeUIDrawCmd
{
	uint16_t vtxOffset;
	uint16_t vtxCount;
	uint16_t idxOffset;
	uint16_t idxCount;
	NeHandle texture;
};

extern struct NeFont UI_sysFont;
extern NeBufferHandle UI_vertexBuffer, UI_indexBuffer;
extern uint64_t UI_vertexBufferSize, UI_indexBufferSize;
extern struct NeArray UI_standaloneContexts;

void _UI_DrawContext(void **comp, void *a);

#endif /* _NE_UI_INTERNAL_H_ */
