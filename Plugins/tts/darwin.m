#import <Foundation/Foundation.h>
#import <AppKit/NSSpeechSynthesizer.h>

#include <System/Log.h>
#include <System/Thread.h>
#include <System/Memory.h>
#include <Engine/Config.h>

#include "TTSInternal.h"

#define NSSS_TTS_MOD	"NSSpeechSynthetizerTTS"

static NSSpeechSynthesizer *_synth = nil;

bool
NSSS_init(void)
{
	if ((_synth = [[NSSpeechSynthesizer alloc] init]) == nil)
		return false;
	
	_synth.usesFeedbackWindow = false;
	
	/*_bufferSize = E_GetCVarI32("TTS.SAPI_BufferSize", 8192)->i32;
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
	ISpVoice_Speak(_voice, L"", SVSFDefault | SVSFlagsAsync, NULL);*/

	return true;
}

void
NSSS_speak(const char *text)
{
	[_synth startSpeakingString:[NSString stringWithUTF8String:text]];
}

void
NSSS_wait(void)
{
	while ([_synth isSpeaking])
		Sys_Yield();
}

bool
NSSS_speaking(void)
{
	return [_synth isSpeaking];
}

void
NSSS_term(void)
{
}

/* NekoEngine TTS Plugin
 *
 * darwin.m
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2022, Alexandru Naiman
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
