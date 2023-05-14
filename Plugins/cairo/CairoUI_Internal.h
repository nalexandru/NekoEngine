#ifndef NE_CAIRO_UI_INTERNAL
#define NE_CAIRO_UI_INTERNAL

#include <stdint.h>
#include <cairo/cairo.h>

#include <Render/Render.h>
#include <Engine/Component.h>
#include <Interfaces/ConsoleOutput.h>

#include "CairoUI.h"

struct NeCairoContext
{
	NE_COMPONENT_BASE;

	cairo_t *cairo;
	cairo_surface_t *surface;
	double fontSize;
};

#define NE_MSG_CAIROUI_SCREEN_RESIZED		0x000B0001

#ifdef __cplusplus
extern "C" {
#endif

extern cairo_t *CRUI_cairo[RE_NUM_FRAMES];
extern cairo_surface_t *CRUI_surface[RE_NUM_FRAMES];
extern uint8_t *CRUI_imageData;
extern NeBufferHandle CRUI_imageBuffer;
extern uint64_t CRUI_bufferSize;
extern struct NeConsoleOutput CRUI_consoleOutput;

void CRUI_DrawConsole(void);

#ifdef __cplusplus
}
#endif

#endif /* NE_CAIRO_UI_INTERNAL */