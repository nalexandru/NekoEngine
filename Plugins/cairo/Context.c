#include <Engine/Engine.h>
#include <Engine/ECSystem.h>
#include <Engine/Component.h>
#include <Script/Interface.h>

#include "CairoUI_Internal.h"

#define NE_CAIRO_UI_RESET_CONTEXT	"NeCairoUI_ResetContext"

static bool
InitContext(struct NeCairoContext *ctx, const char **args)
{
	const char *fontFamily = "Lucida Sans Unicode";
	cairo_font_slant_t textSlant = CAIRO_FONT_SLANT_NORMAL;
	cairo_font_weight_t textWeight = CAIRO_FONT_WEIGHT_NORMAL;

	cairo_font_options_t *fopt = cairo_font_options_create();
	ctx->fontSize = 20.0;

	cairo_font_options_set_antialias(fopt, CAIRO_ANTIALIAS_NONE);
	cairo_font_options_set_hint_style(fopt, CAIRO_HINT_STYLE_FULL);

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strnlen(arg, UINT16_MAX);

		if (!strncmp(arg, "FontFamily", len)) {
			fontFamily = *(++args);
		} else if (!strncmp(arg, "TextSlant", len)) {

			const char *val = *(++args);
			size_t vlen = strnlen(val, UINT16_MAX);
			if (!strncmp(*args, "Normal", vlen))
				textSlant = CAIRO_FONT_SLANT_NORMAL;
			else if (!strncmp(*args, "Italic", vlen))
				textSlant = CAIRO_FONT_SLANT_ITALIC;
			else if (!strncmp(*args, "Oblique", vlen))
				textSlant = CAIRO_FONT_SLANT_OBLIQUE;

		} else if (!strncmp(arg, "TextWeight", len)) {

			const char *val = *(++args);
			size_t vlen = strnlen(val, UINT16_MAX);
			if (!strncmp(*args, "Normal", vlen))
				textWeight = CAIRO_FONT_WEIGHT_NORMAL;
			else if (!strncmp(*args, "Bold", vlen))
				textWeight = CAIRO_FONT_WEIGHT_BOLD;

		} else if (!strncmp(arg, "HintStyle", len)) {

			const char *val = *(++args);
			size_t vlen = strnlen(val, UINT16_MAX);
			if (!strncmp(*args, "None", vlen))
				cairo_font_options_set_hint_style(fopt, CAIRO_HINT_STYLE_NONE);
			else if (!strncmp(*args, "Slight", vlen))
				cairo_font_options_set_hint_style(fopt, CAIRO_HINT_STYLE_SLIGHT);
			else if (!strncmp(*args, "Medium", vlen))
				cairo_font_options_set_hint_style(fopt, CAIRO_HINT_STYLE_MEDIUM);
			else if (!strncmp(*args, "Full", vlen))
				cairo_font_options_set_hint_style(fopt, CAIRO_HINT_STYLE_FULL);

		} else if (!strncmp(arg, "TextAntialias", len)) {
			const char *val = *(++args);
			size_t vlen = strnlen(val, UINT16_MAX);
			if (!strncmp(*args, "None", vlen))
				cairo_font_options_set_antialias(fopt, CAIRO_ANTIALIAS_NONE);
			else if (!strncmp(*args, "Gray", vlen))
				cairo_font_options_set_antialias(fopt, CAIRO_ANTIALIAS_GRAY);
			else if (!strncmp(*args, "Subpixel", vlen))
				cairo_font_options_set_antialias(fopt, CAIRO_ANTIALIAS_SUBPIXEL);
			else if (!strncmp(*args, "Fast", vlen))
				cairo_font_options_set_antialias(fopt, CAIRO_ANTIALIAS_FAST);
			else if (!strncmp(*args, "Good", vlen))
				cairo_font_options_set_antialias(fopt, CAIRO_ANTIALIAS_GOOD);
			else if (!strncmp(*args, "Best", vlen))
				cairo_font_options_set_antialias(fopt, CAIRO_ANTIALIAS_BEST);

		} else if (!strncmp(arg, "SubpixelOrder", len)) {
			const char *val = *(++args);
			size_t vlen = strnlen(val, UINT16_MAX);
			if (!strncmp(*args, "RGB", vlen))
				cairo_font_options_set_subpixel_order(fopt, CAIRO_SUBPIXEL_ORDER_RGB);
			else if (!strncmp(*args, "BGR", vlen))
				cairo_font_options_set_subpixel_order(fopt, CAIRO_SUBPIXEL_ORDER_BGR);
			else if (!strncmp(*args, "VRGB", vlen))
				cairo_font_options_set_subpixel_order(fopt, CAIRO_SUBPIXEL_ORDER_VRGB);
			else if (!strncmp(*args, "VBGR", vlen))
				cairo_font_options_set_subpixel_order(fopt, CAIRO_SUBPIXEL_ORDER_VBGR);

		} else if (!strncmp(arg, "FontSize", len)) {
			ctx->fontSize = atof(*(++args));
		}
	}

	if (!(ctx->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, *E_screenWidth, *E_screenHeight)))
		return false;

	if (!(ctx->cairo = cairo_create(ctx->surface))) {
		cairo_surface_destroy(ctx->surface);
		return false;
	}

	cairo_select_font_face(ctx->cairo, fontFamily, textSlant, textWeight);
	cairo_set_font_size(ctx->cairo, ctx->fontSize);
	cairo_set_font_options(ctx->cairo, fopt);
	cairo_font_options_destroy(fopt);

	return true;
}

static void
TermContext(struct NeCairoContext *ctx)
{
	cairo_destroy(ctx->cairo);
	cairo_surface_destroy(ctx->surface);
}
NE_REGISTER_COMPONENT(NE_CAIRO_CONTEXT, struct NeCairoContext, 1, InitContext, NULL, TermContext);

void *CairoUI_Cairo(struct NeCairoContext *ctx) { return ctx->cairo; }

float CairoUI_GetLineWidth(struct NeCairoContext *ctx) { return (float)cairo_get_line_width(ctx->cairo); }
void CairoUI_SetLineWidth(struct NeCairoContext *ctx, float width) { cairo_set_line_width(ctx->cairo, width); }
void CairoUI_MoveTo(struct NeCairoContext *ctx, float x, float y) { cairo_move_to(ctx->cairo, x, y); }

void
CairoUI_Text(struct NeCairoContext *ctx, const char *text, float px, float py, float size, const char *font)
{
	if (size > .1f) {
		ctx->fontSize = size;
		cairo_set_font_size(ctx->cairo, size);
	}

	cairo_move_to(ctx->cairo, px, py + ctx->fontSize);

	if (font)
		cairo_select_font_face(ctx->cairo, font, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

	cairo_show_text(ctx->cairo, text);
}

/*void
CairoUI_TextExtents(struct NeCairoContext *ctx, const char *text, float size, const char *font)
{
	if (size > .1f) {
		ctx->fontSize = size;
		cairo_set_font_size(ctx->cr, size);
	}

	if (font)
		cairo_select_font_face(ctx->cr, font, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

	cairo_text_extents_t extents;
	cairo_text_extents(ctx->cr, text, &extents);
}*/

/* NekoEngine CairoUI Plugin
 *
 * Context.c
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
