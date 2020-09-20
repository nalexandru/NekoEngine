#ifndef _UI_TEXT_H_
#define _UI_TEXT_H_

#include <Engine/Types.h>

void UI_DrawText(struct UIContext *ctx, const wchar_t *text, float px, float py, float size, struct Font *font);

bool UI_InitText(void);
void UI_TermText(void);

#endif /* _UI_TEXT_H_ */
