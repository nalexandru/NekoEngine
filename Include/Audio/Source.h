#ifndef _NE_AUDIO_SOURCE_H_
#define _NE_AUDIO_SOURCE_H_

#include <Math/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

bool Au_InitSource(struct NeAudioSource *src);
void Au_SetPosition(struct NeAudioSource *src, const struct NeVec3 *v);
void Au_SetPositionF(struct NeAudioSource *src, float x, float y, float z);
void Au_SetVelocity(struct NeAudioSource *src, const struct NeVec3 *v);
void Au_SetVelocityF(struct NeAudioSource *src, float x, float y, float z);
void Au_SetDirection(struct NeAudioSource *src, const struct NeVec3 *v);
void Au_SetDirectionF(struct NeAudioSource *src, float x, float y, float z);
void Au_Play(struct NeAudioSource *src);
void Au_Pause(struct NeAudioSource *src);
void Au_Stop(struct NeAudioSource *src);
void Au_Rewind(struct NeAudioSource *src);
bool Au_IsPlaying(struct NeAudioSource *src);
void Au_Gain(struct NeAudioSource *src, float gain);
void Au_Pitch(struct NeAudioSource *src, float pitch);
void Au_Cone(struct NeAudioSource *src, float innerAngle, float outerAngle, float outerGain);
void Au_MaxDistance(struct NeAudioSource *src, float d);
void Au_RefDistance(struct NeAudioSource *src, float d);
void Au_SetClip(struct NeAudioSource *src, NeHandle clip);
void Au_TermSource(struct NeAudioSource *src);

#ifdef __cplusplus
}
#endif

#endif /* _NE_AUDIO_SOURCE_H_ */

/* NekoEngine
 *
 * Source.h
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
