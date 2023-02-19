#include <Engine/Plugin.h>
#include <Script/Script.h>

#include "TTSInternal.h"

#ifdef _WIN32
#	define EXPORT __declspec(dllexport)
#else
#	define EXPORT
#endif

struct NeTTS tts;

static void _ScriptIface(lua_State *vm);

EXPORT struct NePlugin PluginInfo =
{
	.identifier = NE_PLUGIN_ID,
	.apiVersion = NE_PLUGIN_API,
	.name = "NekoEngine Text-to-Speech",
	.copyright = "(c) 2022 Alexandru Naiman",
	.version = { 0, 1, 0, 1 },
	.loadOrder = NEP_LOAD_PRE_ECS
};

EXPORT struct NePluginInterface PluginInterfaces[] =
{
	{ "NeTTS", &tts },
	{ NULL, NULL }
};

EXPORT bool
InitPlugin(void)
{
	bool rc = false;
#ifdef SAPI_AVAILABLE
	if (SAPI_init()) {
		tts.speak = SAPI_speak;
		tts.wait = SAPI_wait;
		tts.speaking = SAPI_speaking;

		rc = true;
	}
#endif
	
#ifdef NSSS_AVAILABLE
	if (NSSS_init()) {
		tts.speak = NSSS_speak;
		tts.wait = NSSS_wait;
		tts.speaking = NSSS_speaking;

		rc = true;
	}
#endif

	if (!rc)
		return false;

	Sc_RegisterInterface(_ScriptIface);

	return true;
}

EXPORT void
TermPlugin(void)
{
#ifdef SAPI_AVAILABLE
	if (tts.speak == SAPI_speak)
		SAPI_term();
#endif
	
#ifdef NSSS_AVAILABLE
	if (tts.speak == NSSS_speak)
		NSSS_term();
#endif
}

// Script interface

static int
_S_Speak(lua_State *vm)
{
	tts.speak(luaL_checkstring(vm, 1));
	return 0;
}

static int
_S_Wait(lua_State *vm)
{
	tts.wait();
	return 0;
}

static int
_S_Speaking(lua_State *vm)
{
	lua_pushboolean(vm, tts.speaking());
	return 1;
}

void
_ScriptIface(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		{ "Speak", _S_Speak },
		{ "Wait", _S_Wait },
		{ "Speaking", _S_Speaking },
		{ NULL, NULL }
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "TTS");
}


/* NekoEngine TTS Plugin
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
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
