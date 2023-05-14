#include <Engine/Plugin.h>
#include <Script/Script.h>
#include <Script/Interface.h>

#include "TTSInternal.h"

#ifdef _WIN32
#	define EXPORT __declspec(dllexport)
#else
#	define EXPORT
#endif

struct NeTTS tts;
NeTTSVisemeCallback TTS_visemeCallback = NULL;

static void SetVisemeCallback(NeTTSVisemeCallback cb);

EXPORT struct NePlugin PluginInfo =
{
	.identifier = NE_PLUGIN_ID,
	.apiVersion = NE_PLUGIN_API,
	.name = "NekoEngine Text-to-Speech",
	.copyright = "(c) 2022 Alexandru Naiman",
	.version = { 0, 2, 0, 2 },
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
		tts.selectVoice = SAPI_selectVoice;
		tts.listVoices = SAPI_listVoices;

		rc = true;
	}
#endif

#ifdef NSSS_AVAILABLE
	if (NSSS_init()) {
		tts.speak = NSSS_speak;
		tts.wait = NSSS_wait;
		tts.speaking = NSSS_speaking;
		tts.selectVoice = NSSS_selectVoice;
		tts.listVoices = NSSS_listVoices;

		rc = true;
	}
#endif

	tts.setVisemeCallback = SetVisemeCallback;

	if (!rc)
		return false;

	return true;
}

static void
SetVisemeCallback(NeTTSVisemeCallback cb)
{
	TTS_visemeCallback = cb;
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
S_Speak(lua_State *vm)
{
	tts.speak(luaL_checkstring(vm, 1));
	return 0;
}

static int
S_Wait(lua_State *vm)
{
	tts.wait();
	return 0;
}

static int
S_Speaking(lua_State *vm)
{
	lua_pushboolean(vm, tts.speaking());
	return 1;
}

static int
S_SelectVoice(lua_State *vm)
{
	lua_pushboolean(vm, tts.selectVoice(luaL_checkstring(vm, 0)));
	return 1;
}

static int
S_ListVoices(lua_State *vm)
{
	lua_newtable(vm);

	uint32_t count;
	struct NeVoiceInfo *info;
	if (!tts.listVoices(&info, &count))
		return 1;

	for (uint32_t i = 0; i < count; ++i) {
		lua_newtable(vm);
		{
			lua_pushstring(vm, info[i].name);
			lua_setfield(vm, -2, "name");
			lua_pushstring(vm, info[i].language);
			lua_setfield(vm, -2, "language");
			lua_pushinteger(vm, info[i].gender);
			lua_setfield(vm, -2, "gender");
			lua_pushinteger(vm, info[i].samplingRate);
			lua_setfield(vm, -2, "samplingRate");
		}
		lua_rawseti(vm, -2, i + 1);
	}

	Sys_Free(info);

	return 1;
}

NE_SCRIPT_INTEFACE(NeTTS)
{
	luaL_Reg reg[] =
	{
		{ "Speak", S_Speak },
		{ "Wait", S_Wait },
		{ "Speaking", S_Speaking },
		{ "SelectVoice", S_SelectVoice },
		{ "ListVoices", S_ListVoices },
		{ NULL, NULL }
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "TTS");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, 0);
		lua_setfield(vm, -2, "Female");

		lua_pushinteger(vm, 1);
		lua_setfield(vm, -2, "Male");
	}
	lua_setglobal(vm, "VoiceGender");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, NE_VIS_SILENT);
		lua_setfield(vm, -2, "Silent");
		lua_pushinteger(vm, NE_VIS_AE_AX_AH);
		lua_setfield(vm, -2, "AE_AX_AH");
		lua_pushinteger(vm, NE_VIS_AA);
		lua_setfield(vm, -2, "AA");
		lua_pushinteger(vm, NE_VIS_AO);
		lua_setfield(vm, -2, "AO");
		lua_pushinteger(vm, NE_VIS_EY_EH_UH);
		lua_setfield(vm, -2, "EY_EH_UH");
		lua_pushinteger(vm, NE_VIS_ER);
		lua_setfield(vm, -2, "ER");
		lua_pushinteger(vm, NE_VIS_Y_IY_IH_IX);
		lua_setfield(vm, -2, "Y_IY_IH_IX");
		lua_pushinteger(vm, NE_VIS_W_UW);
		lua_setfield(vm, -2, "W_UW");
		lua_pushinteger(vm, NE_VIS_OW);
		lua_setfield(vm, -2, "OW");
		lua_pushinteger(vm, NE_VIS_AW);
		lua_setfield(vm, -2, "AW");
		lua_pushinteger(vm, NE_VIS_OY);
		lua_setfield(vm, -2, "OY");
		lua_pushinteger(vm, NE_VIS_AY);
		lua_setfield(vm, -2, "AY");
		lua_pushinteger(vm, NE_VIS_H);
		lua_setfield(vm, -2, "H");
		lua_pushinteger(vm, NE_VIS_R);
		lua_setfield(vm, -2, "R");
		lua_pushinteger(vm, NE_VIS_L);
		lua_setfield(vm, -2, "L");
		lua_pushinteger(vm, NE_VIS_S_Z);
		lua_setfield(vm, -2, "S_Z");
		lua_pushinteger(vm, NE_VIS_SH_CH_JH_ZH);
		lua_setfield(vm, -2, "SH_CH_JH_ZH");
		lua_pushinteger(vm, NE_VIS_TH_DH);
		lua_setfield(vm, -2, "TH_DH");
		lua_pushinteger(vm, NE_VIS_F_V);
		lua_setfield(vm, -2, "F_V");
		lua_pushinteger(vm, NE_VIS_D_T_N);
		lua_setfield(vm, -2, "D_T_N");
		lua_pushinteger(vm, NE_VIS_K_G_NG);
		lua_setfield(vm, -2, "K_G_NG");
		lua_pushinteger(vm, NE_VIS_P_B_M);
		lua_setfield(vm, -2, "P_B_M");
	}
	lua_setglobal(vm, "Viseme");

	return 1;
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
