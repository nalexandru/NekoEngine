#include <UI/UI.h>

bool
UI_InitUI(void)
{
	return UI_InitText();
}

void
UI_TermUI(void)
{
	UI_TermText();
}