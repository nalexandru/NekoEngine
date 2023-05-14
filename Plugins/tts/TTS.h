#ifndef NE_PLUGIN_TTS
#define NE_PLUGIN_TTS

#include <stdint.h>

enum NeViseme
{
	NE_VIS_SILENT		= 0x0000,
	NE_VIS_AE_AX_AH		= 0x4145,
	NE_VIS_AA			= 0x4141,
	NE_VIS_AO			= 0x414F,
	NE_VIS_EY_EH_UH		= 0x4559,
	NE_VIS_ER			= 0x4552,
	NE_VIS_Y_IY_IH_IX	= 0x5900,
	NE_VIS_W_UW			= 0x5700,
	NE_VIS_OW			= 0x4F57,
	NE_VIS_AW			= 0x4157,
	NE_VIS_OY			= 0x4F59,
	NE_VIS_AY			= 0x4159,
	NE_VIS_H			= 0x4800,
	NE_VIS_R			= 0x5200,
	NE_VIS_L			= 0x4C00,
	NE_VIS_S_Z			= 0x5300,
	NE_VIS_SH_CH_JH_ZH	= 0x5348,
	NE_VIS_TH_DH		= 0x5448,
	NE_VIS_F_V			= 0x4600,
	NE_VIS_D_T_N		= 0x4400,
	NE_VIS_K_G_NG		= 0x4B00,
	NE_VIS_P_B_M		= 0x5000
};

typedef void (*NeTTSVisemeCallback)(enum NeViseme viseme);

struct NeVoiceInfo
{
	char name[256];
	char language[90];
	uint32_t gender : 1;			// 0 - Female, 1 - Male
	uint32_t samplingRate : 31;
};

struct NeTTS
{
	void (*speak)(const char *text);
	void (*wait)(void);
	bool (*speaking)(void);
	void (*setVisemeCallback)(NeTTSVisemeCallback cb);
	bool (*selectVoice)(const char *name);
	bool (*listVoices)(struct NeVoiceInfo **info, uint32_t *count);
};

#endif /* NE_PLUGIN_TTS */

/* NekoEngine TTS Plugin
 *
 * TTS.h
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
