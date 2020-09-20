#ifndef _UI_UI_H_
#define _UI_UI_H_

#include <UI/Text.h>

struct UIVertex
{
	float posUv[4];
	float color[4];
};

bool UI_InitUI();
void UI_TermUI();

#endif /* _UI_UI_H_ */
