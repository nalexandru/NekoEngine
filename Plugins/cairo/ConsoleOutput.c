#include <Engine/Engine.h>
#include <Engine/Config.h>
#include "CairoUI_Internal.h"

static cairo_t *f_cairo;
static cairo_surface_t *f_surface;

void
CRUI_DrawConsole(void)
{
	cairo_set_source_surface(CRUI_cairo[Re_frameId], f_surface, 0.0, 0.0);
	cairo_paint(CRUI_cairo[Re_frameId]);

	cairo_set_operator(f_cairo, CAIRO_OPERATOR_CLEAR);
	cairo_paint(f_cairo);
	cairo_set_operator(f_cairo, CAIRO_OPERATOR_OVER);

/*	cairo_set_source_rgba(f_cairo, .2, .2, .2, .5);
	cairo_set_operator(f_cairo, CAIRO_OPERATOR_SOURCE);
	cairo_paint(f_cairo);
	cairo_set_operator(f_cairo, CAIRO_OPERATOR_OVER);*/
}

static bool
Init(uint32_t maxLines)
{
	f_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, *E_screenWidth, *E_screenHeight);
	f_cairo = cairo_create(f_surface);

	const char *font = E_GetCVarStr("CairoUI_ConsoleFont", "Iosevka")->str;
	uint32_t fontSize = (uint32_t)E_GetCVarI32("CairoUI_ConsoleFontSize", 20)->i32;

	cairo_select_font_face(f_cairo, font, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(f_cairo, fontSize);

	cairo_font_options_t *opt = cairo_font_options_create();
	cairo_get_font_options(f_cairo, opt);

	cairo_font_options_set_hint_style(opt, CAIRO_HINT_STYLE_FULL);
	cairo_font_options_set_antialias(opt, CAIRO_ANTIALIAS_NONE);

	cairo_set_font_options(f_cairo, opt);
	cairo_font_options_destroy(opt);

	cairo_set_source_rgba(f_cairo, 1.0, 1.0, 1.0, 1.0);

	return f_surface && f_cairo;
}

static void
Puts(const char *text, uint32_t x, uint32_t y)
{
	cairo_move_to(f_cairo, x, y);
	cairo_show_text(f_cairo, text);
}

static uint32_t
LineHeight(void)
{
	return 20;
}

static void
Term(void)
{
	cairo_destroy(f_cairo);
	cairo_surface_destroy(f_surface);
}

struct NeConsoleOutput CRUI_consoleOutput =
{
	.Init = Init,
	.Puts = Puts,
	.LineHeight = LineHeight,
	.Term = Term,
};
