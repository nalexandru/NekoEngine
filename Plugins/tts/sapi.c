#define COBJMACROS
#include <sapi.h>
#include <Windows.h>

#include <System/Log.h>
#include <System/Memory.h>
#include <Engine/Config.h>
#include <Runtime/Runtime.h>

#include "TTSInternal.h"

#define SAPI_TTS_MOD	"SAPITTS"

static ISpVoice *f_voice;
static LPWSTR f_buff;
static int32_t f_bufferSize;

bool
SAPI_init(void)
{
	f_bufferSize = E_GetCVarI32("TTS.SAPI_BufferSize", 8192)->i32;
	f_buff = Sys_Alloc(sizeof(*f_buff), f_bufferSize, MH_System);
	if (!f_buff)
		return false;

	// COM is initialized by the Engine
	HRESULT hr = CoCreateInstance(&CLSID_SpVoice, NULL, CLSCTX_ALL, &IID_ISpVoice, (void **)&f_voice);
	if (FAILED(hr)) {
		Sys_Free(f_buff);
		return false;
	}

	// the first call to Speak will block the program for ~1 second
	ISpVoice_Speak(f_voice, L"", SVSFDefault | SVSFlagsAsync, NULL);

	return true;
}

void
SAPI_speak(const char *text)
{
	int32_t len = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
	if (len > f_bufferSize) {
		Sys_LogEntry(SAPI_TTS_MOD, LOG_WARNING, "Speak failed: text size (%d) is greater than buffer size (%d)", len, f_bufferSize);
		return;
	}

	ZeroMemory(f_buff, f_bufferSize * sizeof(*f_buff));
	MultiByteToWideChar(CP_UTF8, 0, text, -1, f_buff, len);
	ISpVoice_Speak(f_voice, f_buff, SVSFDefault | SVSFlagsAsync, NULL);
}

void
SAPI_wait(void)
{
	ISpVoice_WaitUntilDone(f_voice, INFINITE);
}

bool
SAPI_speaking(void)
{
	SPVOICESTATUS status;
	ISpVoice_GetStatus(f_voice, &status, NULL);
	return status.dwRunningState == SPRS_IS_SPEAKING;
}

bool
SAPI_selectVoice(const char *name)
{
	bool rc = false;
	WCHAR wname[256] = { 0 };
	size_t name_len = 0;
	ISpObjectToken *tok = NULL;
	IEnumSpObjectTokens *tokens = NULL;
	ISpObjectTokenCategory *cat = NULL;

	MultiByteToWideChar(CP_UTF8, 0, name, -1, wname, sizeof(wname) / sizeof(wname[0]));
	name_len = wcsnlen(wname, sizeof(wname) / sizeof(wname[0]));

	if (FAILED(CoCreateInstance(&CLSID_SpObjectTokenCategory, NULL, CLSCTX_ALL, &IID_ISpObjectTokenCategory, (LPVOID *)&cat)))
		goto exit;

	if (FAILED(ISpObjectTokenCategory_SetId(cat, SPCAT_VOICES, FALSE)))
		goto exit;

	if (FAILED(ISpObjectTokenCategory_EnumTokens(cat, NULL, NULL, &tokens)))
		goto exit;

	while (SUCCEEDED(IEnumSpObjectTokens_Next(tokens, 1, &tok, NULL))) {
		bool found;
		WCHAR *val;
		ISpDataKey *key;

		if (!tok)
			break;

		if (FAILED(ISpObjectToken_OpenKey(tok, L"Attributes", &key))) {
			ISpObjectToken_Release(tok);
			continue;
		}

		ISpDataKey_GetStringValue(key, L"Name", &val);
		found = !wcsncmp(val, wname, name_len);

		CoTaskMemFree(val);

		ISpDataKey_Release(key);

		if (found) {
			ISpVoice_SetVoice(f_voice, tok);
			ISpObjectToken_Release(tok);

			rc = true;
			break;
		} else {
			ISpObjectToken_Release(tok);
		}
	}

exit:
	if (tokens)
		IEnumSpObjectTokens_Release(tokens);

	if (cat)
		ISpObjectTokenCategory_Release(cat);

	return rc;
}

bool
SAPI_listVoices(struct NeVoiceInfo **info, uint32_t *count)
{
	bool rc = false;
	ISpObjectToken *tok = NULL;
	IEnumSpObjectTokens *tokens = NULL;
	ISpObjectTokenCategory *cat = NULL;
	struct NeArray array = { NULL };

	if (!info || !count)
		goto exit;

	if (FAILED(CoCreateInstance(&CLSID_SpObjectTokenCategory, NULL, CLSCTX_ALL, &IID_ISpObjectTokenCategory, (LPVOID *)&cat)))
		goto exit;

	if (FAILED(ISpObjectTokenCategory_SetId(cat, SPCAT_VOICES, FALSE)))
		goto exit;

	if (FAILED(ISpObjectTokenCategory_EnumTokens(cat, NULL, NULL, &tokens)))
		goto exit;

	if (!Rt_InitArray(&array, 2, sizeof(struct NeVoiceInfo), MH_Plugin))
		goto exit;

	while (SUCCEEDED(IEnumSpObjectTokens_Next(tokens, 1, &tok, NULL))) {
		WCHAR *val;
		DWORD lang_id;
		ISpDataKey *key;
		struct NeVoiceInfo vi = { 0 };

		if (!tok)
			break;

		if (FAILED(ISpObjectToken_OpenKey(tok, L"Attributes", &key))) {
			ISpObjectToken_Release(tok);
			continue;
		}

		ISpDataKey_GetStringValue(key, L"Name", &val);
		WideCharToMultiByte(CP_UTF8, 0, val, -1, vi.name, sizeof(vi.name), NULL, NULL);
		CoTaskMemFree(val);

		ISpDataKey_GetStringValue(key, L"Gender", &val);
		vi.gender = !wcsncmp(val, L"Male", 4);
		CoTaskMemFree(val);

		ISpDataKey_GetStringValue(key, L"Language", &val);
		lang_id = wcstol(val, NULL, 16);
		CoTaskMemFree(val);

		GetLocaleInfoA((LCID)lang_id, LOCALE_SLANGUAGE, vi.language, sizeof(vi.language));
		//GetLocaleInfoA((LCID)lang_id, LOCALE_SISO639LANGNAME, vi.language, 3);
		//GetLocaleInfoA((LCID)lang_id, LOCALE_SISO3166CTRYNAME, vi.language + 3, 3);
		//vi.language[2] = '_';

		//uint32_t samplingRate : 31; ?

		ISpDataKey_Release(key);
		ISpObjectToken_Release(tok);

		Rt_ArrayAdd(&array, &vi);
	}

	*info = (struct NeVoiceInfo *)array.data;
	*count = (uint32_t)array.count;

	rc = true;
exit:
	if (tokens)
		IEnumSpObjectTokens_Release(tokens);

	if (cat)
		ISpObjectTokenCategory_Release(cat);

	return rc;
}

void
SAPI_term(void)
{
	if (f_voice)
		ISpVoice_Release(f_voice);
	Sys_Free(f_buff);
}

/* NekoEngine TTS Plugin
 *
 * sapi.c
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
