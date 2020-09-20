#ifndef _UI_TEXT_H_
#define _UI_TEXT_H_

#include <Engine/Types.h>

struct TextVertex
{
	float posUv[4];
	float color[4];
	float data;
};

void UI_DrawText(float x, float y, float size, const wchar_t *text);

bool UI_InitText(void);
void UI_TermText(void);

#endif /* _UI_TEXT_H_ */
