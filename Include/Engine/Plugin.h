#ifndef NE_ENGINE_PLUGIN_H
#define NE_ENGINE_PLUGIN_H

#include <Engine/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NE_PLUGIN_ID	0xB16B00B5
#define NE_PLUGIN_API	1

/*
 * Plugins must export:
 *	NePlugin struct named PluginInfo
 *	NePluginInitProc function named InitPlugin
 *	NePluginTermProc function named TermPlugin
 * Optionally, a plugin can expose interfaces through an null-terminated array
 * of NePluginInterface named PluginInterfaces.
 */

enum NePluginLoadOrder
{
	NEP_LOAD_EARLY_INIT,
	NEP_LOAD_PRE_IO,
	NEP_LOAD_PRE_SCRIPTING,
	NEP_LOAD_PRE_ECS,
	NEP_LOAD_PRE_RESOURCES,
	NEP_LOAD_PRE_RENDER,
	NEP_LOAD_PRE_AUDIO,
	NEP_LOAD_PRE_INPUT,
	NEP_LOAD_PRE_UI,
	NEP_LOAD_PRE_NETWORK,
	NEP_LOAD_POST_INIT,
	NEP_LOAD_POST_APP_INIT,

	NEP_FORCE_UINT32 = (uint32_t)-1
};

struct NePlugin
{
	uint32_t identifier;
	uint32_t apiVersion;

	const char *name;
	const char *copyright;
	struct NeVersion version;
	enum NePluginLoadOrder loadOrder;
};

struct NePluginInterface
{
	const char *name;
	void *ptr;
};

typedef bool (*NePluginInitProc)(void);
typedef void (*NePluginTermProc)(void);

bool E_PluginLoaded(const char *name);
void *E_GetInterface(const char *name);

#ifdef _ENGINE_INTERNAL_

void E_RegisterInterface(const char *name, void *ptr);

bool E_InitPluginList(void);
bool E_LoadPlugins(enum NePluginLoadOrder order);
void E_UnloadPlugins(enum NePluginLoadOrder order);
void E_TermPluginList(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /* NE_ENGINE_PLUGIN_H */

/* NekoEngine
 *
 * Plugin.h
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
