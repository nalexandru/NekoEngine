#ifndef _NE_UI_INTERNAL_H_
#define _NE_UI_INTERNAL_H_

#include <UI/UI.h>
#include <UI/Font.h>
#include <UI/Text.h>

#pragma pack(push,1)
struct UIVertex
{
	float posUv[4];
	float color[4];
};
#pragma pack(pop)

struct UIDrawCmd
{
	uint16_t vtxOffset;
	uint16_t vtxCount;
	uint16_t idxOffset;
	uint16_t idxCount;
	Handle texture;
};

extern struct Font UI_sysFont;
extern BufferHandle UI_vertexBuffer, UI_indexBuffer;
extern uint64_t UI_vertexBufferSize, UI_indexBufferSize;

#endif /* _NE_UI_INTERNAL_H_ */
