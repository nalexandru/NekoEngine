#ifndef NE_AUDIO_AUDIO_H
#define NE_AUDIO_AUDIO_H

#include <stdbool.h>

#include <Audio/Clip.h>
#include <Audio/Source.h>

#ifdef __cplusplus
extern "C" {
#endif

enum NeDistanceModel
{
	DM_InverseDistance,
	DM_InverseDistanceClamped,
	DM_LinearDistance,
	DM_LinearDistanceClamped,
	DM_ExponentDistance,
	DM_ExponentDistanceClamped
};

bool Au_Init(void);

void Au_DistanceModel(enum NeDistanceModel model);

void Au_ListenerPosition(const struct NeVec3 *v);
void Au_ListenerPositionF(float x, float y, float z);
void Au_ListenerVelocity(const struct NeVec3 *v);
void Au_ListenerVelocityF(float x, float y, float z);
void Au_ListenerOrientation(const float *orientation);

void Au_Term(void);

#ifdef __cplusplus
}
#endif

#endif /* NE_AUDIO_AUDIO_H */

/* NekoEngine
 *
 * Audio.h
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
