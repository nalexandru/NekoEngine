#ifndef NE_PLUGIN_TTS_INTERNAL_H
#define NE_PLUGIN_TTS_INTERNAL_H

#include <stdbool.h>
#include <System/PlatformDetect.h>

#include "TTS.h"

#ifdef SYS_PLATFORM_WINDOWS
#	define SAPI_AVAILABLE

	bool SAPI_init(void);
	void SAPI_speak(const char *text);
	void SAPI_wait(void);
	bool SAPI_speaking(void);
	bool SAPI_selectVoice(const char *name);
	bool SAPI_listVoices(struct NeVoiceInfo **info, uint32_t *count);
	void SAPI_term(void);
#elif defined(SYS_PLATFORM_APPLE)
#	define NSSS_AVAILABLE

	bool NSSS_init(void);
	void NSSS_speak(const char *text);
	void NSSS_wait(void);
	bool NSSS_speaking(void);
	bool NSSS_selectVoice(const char *name);
	bool NSSS_listVoices(struct NeVoiceInfo **info, uint32_t *count);
	void NSSS_term(void);
#endif

#endif /* NE_PLUGIN_TTS_INTERNAL_H */

/* NekoEngine TTS Plugin
*
* TTSInternal.h
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
