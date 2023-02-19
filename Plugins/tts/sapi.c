#define COBJMACROS
#include <sapi.h>
#include <Windows.h>

#include <System/Log.h>
#include <System/Memory.h>
#include <Engine/Config.h>

#include "TTSInternal.h"

#define SAPI_TTS_MOD	"SAPITTS"

static ISpVoice *_voice;
static LPWSTR _buff;
static int32_t _bufferSize;

bool
SAPI_init(void)
{
	_bufferSize = E_GetCVarI32("TTS.SAPI_BufferSize", 8192)->i32;
	_buff = Sys_Alloc(sizeof(*_buff), _bufferSize, MH_System);
	if (!_buff)
		return false;

	// COM is initialized by the Engine
	HRESULT hr = CoCreateInstance(&CLSID_SpVoice, NULL, CLSCTX_ALL, &IID_ISpVoice, (void **)&_voice);
	if (FAILED(hr)) {
		Sys_Free(_buff);
		return false;
	}

	// the first call to Speak will block the program for ~1 second
	ISpVoice_Speak(_voice, L"", SVSFDefault | SVSFlagsAsync, NULL);

	return true;
}

void
SAPI_speak(const char *text)
{
	int32_t len = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
	if (len > _bufferSize) {
		Sys_LogEntry(SAPI_TTS_MOD, LOG_WARNING, "Speak failed: text size (%d) is greater than buffer size (%d)", len, _bufferSize);
		return;
	}

	ZeroMemory(_buff, _bufferSize * sizeof(*_buff));
	MultiByteToWideChar(CP_UTF8, 0, text, -1, _buff, len);
	ISpVoice_Speak(_voice, _buff, SVSFDefault | SVSFlagsAsync, NULL);
}

void
SAPI_wait(void)
{
	ISpVoice_WaitUntilDone(_voice, INFINITE);
}

bool
SAPI_speaking(void)
{
	SPVOICESTATUS status;
	ISpVoice_GetStatus(_voice, &status, NULL);
	return status.dwRunningState == SPRS_IS_SPEAKING;
}

void
SAPI_term(void)
{
	if (_voice)
		ISpVoice_Release(_voice);
	Sys_Free(_buff);
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
