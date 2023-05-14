#include "Internal.h"
#include <Engine/Plugin.h>
#include <Interfaces/ConsoleOutput.h>

static struct NeUIContext *f_ctx;

static bool UICO_Init(uint32_t maxLines) { f_ctx = UI_CreateStandaloneContext(4000, 6000, maxLines); return f_ctx != NULL; }
static void UICO_Puts(const char *text, uint32_t x, uint32_t y) { UI_DrawText(f_ctx, text, (float)x, (float)y, 20, NULL); }
static uint32_t UICO_LineHeight(void) { return 20; }
static void UICO_Term(void) { UI_DestroyStandaloneContext(f_ctx); }

static struct NeConsoleOutput f_uiConsoleOutput =
{
	.Init = UICO_Init,
	.Puts = UICO_Puts,
	.LineHeight = UICO_LineHeight,
	.Term = UICO_Term,
};

void
UI_p_InitConsoleOutput(void)
{
	E_RegisterInterface(NEIF_CONSOLE_OUTPUT, &f_uiConsoleOutput);
}
