#include <System/Memory.h>
#include <Render/Render.h>
#include <Engine/Plugin.h>
#include <Engine/Engine.h>
#include <Engine/Events.h>
#include <Engine/Config.h>
#include <Engine/Component.h>
#include <UI/UI.h>

#include "CairoUI_Internal.h"

#ifdef _WIN32
#	define EXPORT __declspec(dllexport)
#else
#	define EXPORT
#endif

cairo_t *CRUI_cairo[RE_NUM_FRAMES] = { NULL };
cairo_surface_t *CRUI_surface[RE_NUM_FRAMES] = { NULL };
uint8_t *CRUI_imageData = NULL;
NeBufferHandle CRUI_imageBuffer = -1;
uint64_t CRUI_bufferSize = 0;

static uint64_t f_resizeHandler;

EXPORT struct NePlugin PluginInfo =
{
	.identifier = NE_PLUGIN_ID,
	.apiVersion = NE_PLUGIN_API,
	.name = "CairoUI",
	.copyright = "(c) 2023 Alexandru Naiman. All rights reserved.",
	.version = { 0, 1, 0, 5 },
	.loadOrder = NEP_LOAD_PRE_UI
};

EXPORT struct NePluginInterface PluginInterfaces[] =
{
//	{ "NeCairoUI", &crui },
	{ NEIF_CONSOLE_OUTPUT, &CRUI_consoleOutput },
	{ NULL, NULL }
};

static bool CreateCairo(void);
static void DestroyCairo(void);
static void ScreenResized(void *user, void *args);

EXPORT bool
InitPlugin(void)
{
	if (!CreateCairo())
		return false;

	UI_pluginLoaded = true;
	f_resizeHandler = E_RegisterHandler(EVT_SCREEN_RESIZED, ScreenResized, NULL);

	E_SetCVarStr("Render_UIPass", "NeCairoUI");
	return true;
}

EXPORT void
TermPlugin(void)
{
	E_UnregisterHandler(f_resizeHandler);
	DestroyCairo();
}

static bool
CreateCairo(void)
{
	CRUI_bufferSize = *E_screenWidth * *E_screenHeight * 4;

	struct NeBufferCreateInfo bci =
	{
		.desc.size = CRUI_bufferSize * RE_NUM_FRAMES,
		.desc.usage = BU_TRANSFER_SRC,
		.desc.memoryType = MT_CPU_COHERENT,
		.desc.name = "Cairo UI buffer"
	};
	if (!Re_CreateBuffer(&bci, &CRUI_imageBuffer))
		goto error;

	if (!(CRUI_imageData = Re_MapBuffer(CRUI_imageBuffer)))
		goto error;

	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		if (!(CRUI_surface[i] = cairo_image_surface_create_for_data(CRUI_imageData + CRUI_bufferSize * i,
											CAIRO_FORMAT_ARGB32, *E_screenWidth, *E_screenHeight, 4 * *E_screenWidth)))
			goto error;

		if (!(CRUI_cairo[i] = cairo_create(CRUI_surface[i])))
			goto error;
	}

	return true;
error:
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		if (CRUI_cairo[i])
			cairo_destroy(CRUI_cairo[i]);

		if (CRUI_surface[i])
			cairo_surface_destroy(CRUI_surface[i]);
	}

	if (CRUI_imageData)
		Re_UnmapBuffer(CRUI_imageBuffer);

	if (CRUI_imageBuffer != (uint16_t)-1)
		Re_Destroy(CRUI_imageBuffer);

	return false;
}

static void
DestroyCairo(void)
{
	for (uint32_t i = 0; i < RE_NUM_FRAMES; ++i) {
		cairo_destroy(CRUI_cairo[i]);
		cairo_surface_destroy(CRUI_surface[i]);
	}

	Re_UnmapBuffer(CRUI_imageBuffer);
	Re_Destroy(CRUI_imageBuffer);
}

static void
ScreenResized(void *user, void *args)
{
	DestroyCairo();
	CreateCairo();

	const struct NeArray *comp = E_GetAllComponents(NE_CAIRO_CONTEXT_ID);
	struct NeCairoContext *ctx;
	Rt_ArrayForEach(ctx, comp) {
		cairo_destroy(ctx->cairo);
		cairo_surface_destroy(ctx->surface);

		ctx->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, *E_screenWidth, *E_screenHeight);
		ctx->cairo = cairo_create(ctx->surface);
	}
}

/* NekoEngine Cairo UI Plugin
 *
 * plugin.c
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
