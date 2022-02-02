#ifndef _NE_UI_TEXT_H_
#define _NE_UI_TEXT_H_

#include <Engine/Types.h>

void UI_DrawText(struct NeUIContext *ctx, const char *text, float px, float py, float size, struct NeFont *font);

bool UI_InitText(void);
void UI_TermText(void);

#endif /* _NE_UI_TEXT_H_ */
