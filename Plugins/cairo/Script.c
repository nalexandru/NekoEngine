#include <Script/Interface.h>

#include "CairoUI_Internal.h"

SIF_FUNC(Text)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	CairoUI_Text(cc, luaL_checkstring(vm, 2), luaL_checknumber(vm, 3), luaL_checknumber(vm, 4),
					luaL_optnumber(vm, 5, 0.f), luaL_optstring(vm, 6, NULL));
	return 0;
}

SIF_FUNC(SelectFont)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	cairo_select_font_face(cc->cairo, luaL_checkstring(vm, 2),
						   luaL_optinteger(vm, 3, CAIRO_FONT_SLANT_NORMAL),
						   luaL_optinteger(vm, 3, CAIRO_FONT_WEIGHT_NORMAL));
	return 0;
}

SIF_FUNC(Source)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	if (lua_islightuserdata(vm, 2))
		cairo_set_source(cc->cairo, lua_touserdata(vm, 2));
	else
		cairo_set_source_rgba(cc->cairo, luaL_checknumber(vm, 2), luaL_checknumber(vm, 3), luaL_checknumber(vm, 4),
							  luaL_optnumber(vm, 5, 1.f));
	return 0;
}

SIF_FUNC(LineWidth)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	if (lua_isnumber(vm, 2)) {
		cairo_set_line_width(cc->cairo, lua_tonumber(vm, 2));
		return 0;
	} else {
		lua_pushnumber(vm, (lua_Number)cairo_get_line_width(cc->cairo));
		return 1;
	}
}

SIF_FUNC(LineJoin)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	if (lua_isinteger(vm, 2)) {
		cairo_set_line_join(cc->cairo, lua_tointeger(vm, 2));
		return 0;
	} else {
		lua_pushinteger(vm, cairo_get_line_join(cc->cairo));
		return 1;
	}
}

SIF_FUNC(LineCap)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	if (lua_isinteger(vm, 2)) {
		cairo_set_line_cap(cc->cairo, lua_tointeger(vm, 2));
		return 0;
	} else {
		lua_pushinteger(vm, cairo_get_line_cap(cc->cairo));
		return 1;
	}
}

SIF_FUNC(FillRule)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	if (lua_isinteger(vm, 2)) {
		cairo_set_fill_rule(cc->cairo, lua_tointeger(vm, 2));
		return 0;
	} else {
		lua_pushinteger(vm, cairo_get_fill_rule(cc->cairo));
		return 1;
	}
}

SIF_FUNC(Arc)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	const bool negative = luaL_optinteger(vm, 7, 0);
	if (negative)
		cairo_arc_negative(cc->cairo, luaL_checknumber(vm, 2), luaL_checknumber(vm, 3), luaL_checknumber(vm, 4),
						   luaL_checknumber(vm, 5), luaL_checknumber(vm, 6));
	else
		cairo_arc(cc->cairo, luaL_checknumber(vm, 2), luaL_checknumber(vm, 3), luaL_checknumber(vm, 4),
				  luaL_checknumber(vm, 5), luaL_checknumber(vm, 6));
	return 0;
}

SIF_FUNC(Fill)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	if (lua_isboolean(vm, 2) && lua_toboolean(vm, 2))
		cairo_fill_preserve(cc->cairo);
	else
		cairo_fill(cc->cairo);
	return 0;
}

SIF_FUNC(Line)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	if (lua_isnumber(vm, 4)) {
		cairo_move_to(cc->cairo, luaL_checknumber(vm, 2), luaL_checknumber(vm, 3));
		cairo_line_to(cc->cairo, luaL_checknumber(vm, 4), luaL_checknumber(vm, 5));
	} else {
		cairo_line_to(cc->cairo, luaL_checknumber(vm, 2), luaL_checknumber(vm, 3));
	}
	return 0;
}

SIF_FUNC(Stroke)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	if (lua_isboolean(vm, 2) && lua_toboolean(vm, 2))
		cairo_stroke_preserve(cc->cairo);
	else
		cairo_stroke(cc->cairo);
	return 0;
}

SIF_FUNC(Curve)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	if (lua_isnumber(vm, 8)) {
		cairo_move_to(cc->cairo, luaL_checknumber(vm, 2), luaL_checknumber(vm, 3));
		cairo_curve_to(cc->cairo, luaL_checknumber(vm, 4), luaL_checknumber(vm, 5),
					   luaL_checknumber(vm, 6), luaL_checknumber(vm, 7),
					   luaL_checknumber(vm, 8), luaL_checknumber(vm, 9));
	} else {
		cairo_curve_to(cc->cairo, luaL_checknumber(vm, 2), luaL_checknumber(vm, 3),
					   luaL_checknumber(vm, 4), luaL_checknumber(vm, 5),
					   luaL_checknumber(vm, 6), luaL_checknumber(vm, 7));
	}
	return 0;
}

SIF_FUNC(Clip)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	if (lua_isboolean(vm, 2) && lua_toboolean(vm, 2))
		cairo_clip_preserve(cc->cairo);
	else
		cairo_clip(cc->cairo);
	return 0;
}

SIF_FUNC(NewPath)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	cairo_new_path(cc->cairo);
	return 0;
}

SIF_FUNC(NewSubPath)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	cairo_new_sub_path(cc->cairo);
	return 0;
}

SIF_FUNC(ClosePath)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	cairo_close_path(cc->cairo);
	return 0;
}

SIF_FUNC(Translate)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	cairo_translate(cc->cairo, luaL_checknumber(vm, 2), luaL_checknumber(vm, 3));
	return 0;
}

SIF_FUNC(Rotate)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	cairo_rotate(cc->cairo, luaL_checknumber(vm, 2));
	return 0;
}

SIF_FUNC(Scale)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	cairo_scale(cc->cairo, luaL_checknumber(vm, 2), luaL_checknumber(vm, 3));
	return 0;
}

SIF_FUNC(Move)
{
	SIF_CHECKCOMPONENT(1, cc, NE_CAIRO_CONTEXT, struct NeCairoContext *);
	if (lua_isboolean(vm, 4) && lua_toboolean(vm, 4))
		cairo_rel_move_to(cc->cairo, luaL_checknumber(vm, 2), luaL_checknumber(vm, 3));
	else
		cairo_move_to(cc->cairo, luaL_checknumber(vm, 2), luaL_checknumber(vm, 3));
	return 0;
}

SIF_FUNC(__tostring)
{
	lua_pushliteral(vm, "NeCairoUI");
	return 1;
}

NE_SCRIPT_INTEFACE(NeCairoUI)
{
	luaL_Reg meta[] = {
		{ "__index",     NULL },
		SIF_REG(__tostring),
		SIF_ENDREG()
	};

	luaL_Reg meth[] = {
		SIF_REG(Text),
		SIF_REG(SelectFont),
		SIF_REG(Source),
		SIF_REG(LineWidth),
		SIF_REG(LineJoin),
		SIF_REG(LineCap),
		SIF_REG(FillRule),
		SIF_REG(Arc),
		SIF_REG(Fill),
		SIF_REG(Line),
		SIF_REG(Stroke),
		SIF_REG(Curve),
		SIF_REG(Clip),
		SIF_REG(NewPath),
		SIF_REG(NewSubPath),
		SIF_REG(ClosePath),
		SIF_REG(Translate),
		SIF_REG(Rotate),
		SIF_REG(Scale),
		SIF_REG(Move),
		SIF_ENDREG()
	};

	luaL_newmetatable(vm, NE_CAIRO_CONTEXT);
	luaL_setfuncs(vm, meta, 0);
	luaL_newlibtable(vm, meth);
	luaL_setfuncs(vm, meth, 0);
	lua_setfield(vm, -2, "__index");
	lua_pop(vm, 1);

	lua_newtable(vm);
	{
		lua_pushinteger(vm, 0);
		lua_setfield(vm, -2, "Positive");
		lua_pushinteger(vm, 1);
		lua_setfield(vm, -2, "Negative");
	}
	lua_setglobal(vm, "ArcType");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, CAIRO_LINE_JOIN_MITER);
		lua_setfield(vm, -2, "Miter");
		lua_pushinteger(vm, CAIRO_LINE_JOIN_ROUND);
		lua_setfield(vm, -2, "Round");
		lua_pushinteger(vm, CAIRO_LINE_JOIN_BEVEL);
		lua_setfield(vm, -2, "Bevel");
	}
	lua_setglobal(vm, "LineJoin");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, CAIRO_LINE_CAP_BUTT);
		lua_setfield(vm, -2, "Butt");
		lua_pushinteger(vm, CAIRO_LINE_CAP_ROUND);
		lua_setfield(vm, -2, "Round");
		lua_pushinteger(vm, CAIRO_LINE_CAP_SQUARE);
		lua_setfield(vm, -2, "Square");
	}
	lua_setglobal(vm, "LineCap");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, CAIRO_FILL_RULE_WINDING);
		lua_setfield(vm, -2, "Winding");
		lua_pushinteger(vm, CAIRO_FILL_RULE_EVEN_ODD);
		lua_setfield(vm, -2, "EvenOdd");
	}
	lua_setglobal(vm, "FillRule");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, CAIRO_FONT_SLANT_NORMAL);
		lua_setfield(vm, -2, "Normal");
		lua_pushinteger(vm, CAIRO_FONT_SLANT_ITALIC);
		lua_setfield(vm, -2, "Italic");
	}
	lua_setglobal(vm, "TextSlant");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, CAIRO_FONT_WEIGHT_NORMAL);
		lua_setfield(vm, -2, "Normal");
		lua_pushinteger(vm, CAIRO_FONT_WEIGHT_BOLD);
		lua_setfield(vm, -2, "Bold");
	}
	lua_setglobal(vm, "TextWeight");

	return 1;
}

/* NekoEngine CairoUI Plugin
 *
 * Script.c
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
