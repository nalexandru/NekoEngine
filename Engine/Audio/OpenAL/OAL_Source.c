#include <stdio.h>
#include <string.h>

#include <Audio/Clip.h>
#include <Audio/Source.h>
#include <Engine/Resource.h>
#include <Engine/Component.h>
#include <Scene/Components.h>

#include "Internal.h"

static bool InitSource(struct NeAudioSource *src, const void **args);
static void TermSource(struct NeAudioSource *src);

NE_REGISTER_COMPONENT(NE_AUDIO_SOURCE, struct NeAudioSource, 16, InitSource, NULL, TermSource)

bool
Au_InitSource(struct NeAudioSource *src)
{
	alGenSources(1, &src->id);
	if (alGetError() != AL_NO_ERROR)
		return false;

	return true;
}

void
Au_Position(struct NeAudioSource *src, const struct NeVec3 *v)
{
	alSource3f(src->id, AL_POSITION, v->x, v->y, v->z);
}

void
Au_PositionF(struct NeAudioSource *src, float x, float y, float z)
{
	alSource3f(src->id, AL_POSITION, x, y, z);
}

void
Au_Velocity(struct NeAudioSource *src, const struct NeVec3 *v)
{
	alSource3f(src->id, AL_VELOCITY, v->x, v->y, v->z);
}

void
Au_VelocityF(struct NeAudioSource *src, float x, float y, float z)
{
	alSource3f(src->id, AL_VELOCITY, x, y, z);
}

void
Au_Direction(struct NeAudioSource *src, const struct NeVec3 *v)
{
	alSource3f(src->id, AL_DIRECTION, v->x, v->y, v->z);
}

void
Au_DirectionF(struct NeAudioSource *src, float x, float y, float z)
{
	alSource3f(src->id, AL_DIRECTION, x, y, z);
}

void
Au_Play(struct NeAudioSource *src)
{
	alSourcePlay(src->id);
}

void
Au_Pause(struct NeAudioSource *src)
{
	alSourcePause(src->id);
}

void
Au_Stop(struct NeAudioSource *src)
{
	alSourceStop(src->id);
}

void
Au_Rewind(struct NeAudioSource *src)
{
	alSourceRewind(src->id);
}

bool
Au_IsPlaying(struct NeAudioSource *src)
{
	ALenum state;
	alGetSourcei(src->id, AL_SOURCE_STATE, &state);
	return state == AL_PLAYING;
}

void
Au_Gain(struct NeAudioSource *src, float gain)
{
	alSourcef(src->id, AL_GAIN, gain);
}

void
Au_Pitch(struct NeAudioSource *src, float pitch)
{
	alSourcef(src->id, AL_PITCH, pitch);
}

void
Au_Cone(struct NeAudioSource *src, float innerAngle, float outerAngle, float outerGain)
{
	alSourcef(src->id, AL_CONE_INNER_ANGLE, innerAngle);
	alSourcef(src->id, AL_CONE_OUTER_ANGLE, outerAngle);
	alSourcef(src->id, AL_CONE_OUTER_GAIN, outerGain);
}

void
Au_MaxDistance(struct NeAudioSource *src, float d)
{
	alSourcef(src->id, AL_MAX_DISTANCE, d);
}

void
Au_RefDistance(struct NeAudioSource *src, float d)
{
	alSourcef(src->id, AL_REFERENCE_DISTANCE, d);
}

void
Au_SetClip(struct NeAudioSource *src, NeHandle clip)
{
	alSourceStop(src->id);

	if (clip == NE_INVALID_HANDLE) {
		alSourcei(src->id, AL_BUFFER, 0);
	} else {
		const struct NeAudioClip *ac = E_ResourcePtr(clip);
		alSourcei(src->id, AL_BUFFER, ac->handle);
	}

	if (src->clip != NE_INVALID_HANDLE)
		E_UnloadResource(src->clip);
	src->clip = clip;
}

void
Au_TermSource(struct NeAudioSource *src)
{
	Au_SetClip(src, NE_INVALID_HANDLE);
	alDeleteSources(1, &src->id);
}

static bool
InitSource(struct NeAudioSource *src, const void **args)
{
	const char *clip = NULL;
	bool play = false;

	if (!Au_InitSource(src))
		return false;

	Au_Pitch(src, 1.f);
	Au_Gain(src, 1.f);
	Au_PositionF(src, 0.f, 0.f, 0.f);
	Au_VelocityF(src, 0.f, 0.f, 0.f);
	Au_DirectionF(src, 0.f, 0.f, 0.f);

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "Clip", len)) {
			clip = (const char *) *(++args);
		} else if (!strncmp(arg, "Pitch", len)) {
			Au_Pitch(src, strtof(*(++args), NULL));
		} else if (!strncmp(arg, "Gain", len)) {
			Au_Gain(src, strtof(*(++args), NULL));
		} else if (!strncmp(arg, "Cone", len)) {
			char *ptr = (char *)*(++args);
			const float innerAngle = strtof(ptr, &ptr);
			const float outerAngle = strtof(ptr + 2, &ptr);
			const float outerGain = strtof(ptr + 2, &ptr);
			Au_Cone(src, innerAngle, outerAngle, outerGain);
		} else if (!strncmp(arg, "MaxDistance", len)) {
			Au_MaxDistance(src, strtof(*(++args), NULL));
		} else if (!strncmp(arg, "RefDistance", len)) {
			Au_RefDistance(src, strtof(*(++args), NULL));
		} else if (!strncmp(arg, "PlayOnLoad", len)) {
			play = strstr(*(++args), "true") != NULL;
		}
	}

	if (clip) {
		Au_SetClip(src, E_LoadResource(clip, RES_AUDIO_CLIP));
		if (play)
			alSourcePlay(src->id);
	}

	return true;
}

static void
TermSource(struct NeAudioSource *src)
{
	alSourceStop(src->id);
	alSourcei(src->id, AL_BUFFER, 0);

	if (src->clip != NE_INVALID_HANDLE)
		E_UnloadResource(src->clip);

	Au_TermSource(src);
}

#ifdef __APPLE__
#	pragma clang diagnostic pop
#endif

/* NekoEngine
 *
 * OAL_Source.c
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
