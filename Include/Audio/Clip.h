#ifndef _NE_AUDIO_CLIP_H_
#define _NE_AUDIO_CLIP_H_

#include <Engine/Types.h>
#include <Engine/Component.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RES_AUDIO_CLIP	"AudioClip"

struct NeAudioClip
{
	NE_COMPONENT_BASE;

	uint32_t handle;
	uint16_t *data;
	uint32_t byteSize;
	uint32_t bitsPerSample;
	uint32_t sampleRate;
	uint32_t channels;
};

struct NeAudioClipCreateInfo
{
	uint16_t *samples;
	uint32_t sampleCount;
};

bool Au_CreateClip(const char *name, const struct NeAudioClipCreateInfo *ci, struct NeAudioClip *data, NeHandle h);
bool Au_LoadClip(struct NeResourceLoadInfo *li, const char *args, struct NeAudioClip *model, NeHandle h);
void Au_UnloadClip(struct NeAudioClip *model, NeHandle h);

#ifdef __cplusplus
}
#endif

#endif /* _NE_AUDIO_CLIP_H_ */

/* NekoEngine
 *
 * Clip.h
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
