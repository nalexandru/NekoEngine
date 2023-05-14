#include <System/Log.h>
#include <System/System.h>
#include <Engine/Config.h>
#include <Engine/Plugin.h>
#include <Runtime/Runtime.h>

#define PLUGIN_MOD	"Plugin"

struct PluginInterface
{
	uint64_t hash;
	void *ptr;
};

struct NePluginInfo
{
	uint64_t name;
	struct NePlugin *plugin;
	NePluginInitProc init;
	NePluginTermProc term;
	struct NePluginInterface *ifaces;
	void *handle;
};

static struct NeArray f_pluginList;
static struct NeArray f_interfaces;

bool
E_InitPluginList(void)
{
	bool rc = false;

	if (!Rt_InitArray(&f_pluginList, 10, sizeof(struct NePluginInfo), MH_System))
		return false;

	if (!Rt_InitArray(&f_interfaces, 10, sizeof(struct PluginInterface), MH_System))
		return false;

	char dir[4096], path[4096];
	Sys_ExecutableLocation(dir, sizeof(dir));
	strlcat(dir, "/Plugins", sizeof(dir));

	Sys_LogEntry(PLUGIN_MOD, LOG_DEBUG, "Plugin path: %s", dir);

	const char *plugins = CVAR_STRING("Engine_Plugins");
	if (!plugins || !strlen(plugins))
		return true;

	char *p = Rt_StrDup(plugins, MH_Transient);
	char *e = strchr(p, ';');
	if (e) *e++ = 0x0;

	while (p) {
		struct NePluginInfo pi = { 0 };

		bool valid = false;

#ifdef SYS_PLATFORM_UNIX
		snprintf(path, sizeof(path), "%s/lib%s", dir, p);
#else
		snprintf(path, sizeof(path), "%s/%s.dll", dir, p);
#endif

		pi.handle = Sys_LoadLibrary(path);
		if (pi.handle) {
			pi.plugin = Sys_GetProcAddress(pi.handle, "PluginInfo");
			pi.init = Sys_GetProcAddress(pi.handle, "InitPlugin");
			pi.term = Sys_GetProcAddress(pi.handle, "TermPlugin");
			pi.ifaces = Sys_GetProcAddress(pi.handle, "PluginInterfaces");

			valid = pi.plugin && pi.init && pi.term &&
				pi.plugin->identifier == NE_PLUGIN_ID &&
				pi.plugin->apiVersion == NE_PLUGIN_API;
		}

		if (!valid) {
			Sys_LogEntry(PLUGIN_MOD, LOG_CRITICAL, "Failed to load library for plugin %s", p);
			goto exit;
		}

		pi.name = Rt_HashString(pi.plugin->name);
		Rt_ArrayAdd(&f_pluginList, &pi);

		p = e;
		if (!p) break;

		e = strchr(p, ';');
		if (e) *e++ = 0x0;
	}

	rc = true;

exit:
	return rc;
}

bool
E_LoadPlugins(enum NePluginLoadOrder order)
{
	struct NePluginInfo *p;
	Rt_ArrayForEach(p, &f_pluginList) {
		if (p->plugin->loadOrder != order)
			continue;

		if (!p->init()) {
			Sys_LogEntry(PLUGIN_MOD, LOG_CRITICAL, "Plugin %s failed initialization", p->plugin->name);
			return false;
		}

		if (!p->ifaces)
			continue;

		// TODO: use GUIDs instead of strings
		struct NePluginInterface *iface = p->ifaces;
		while (iface->name) {
			struct PluginInterface pi =
			{
				.hash = Rt_HashString(iface->name),
				.ptr = iface->ptr
			};
			Rt_ArrayInsert(&f_interfaces, &pi, 0);

			iface++;
		}
	}
	return true;
}

void
E_UnloadPlugins(enum NePluginLoadOrder order)
{
	struct NePluginInfo *p;
	Rt_ArrayForEach(p, &f_pluginList) {
		if (p->plugin->loadOrder != order)
			continue;

		struct NePluginInterface *iface = p->ifaces;
		while (iface->name) {
			uint64_t hash = Rt_HashString(iface->name);
			size_t i;
			for (i = 0; i < f_interfaces.count; ++i) {
				struct PluginInterface *pi = Rt_ArrayGet(&f_interfaces, i);
				if (pi->hash == hash)
					break;
			}
			Rt_ArrayRemove(&f_interfaces, i);

			iface++;
		}

		p->term();
	}
}

bool
E_PluginLoaded(const char *name)
{
	uint64_t hash = Rt_HashString(name);
	struct NePluginInfo *p;
	Rt_ArrayForEach(p, &f_pluginList)
		if (p->name == hash)
			return true;
	return false;
}

void *
E_GetInterface(const char *name)
{
	uint64_t hash = Rt_HashString(name);
	struct PluginInterface *pi;
	Rt_ArrayForEach(pi, &f_interfaces)
		if (pi->hash == hash)
			return pi->ptr;
	return NULL;
}

void
E_TermPluginList(void)
{
	Rt_TermArray(&f_interfaces);
	Rt_TermArray(&f_pluginList);
}

void
E_RegisterInterface(const char *name, void *ptr)
{
	struct PluginInterface pi = { .hash = Rt_HashString(name), .ptr = ptr };
	Rt_ArrayAdd(&f_interfaces, &pi);
}

/* NekoEngine
 *
 * Plugin.c
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
