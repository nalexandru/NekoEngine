/* NekoEngine
 *
 * sound_clip.h
 * Author: Alexandru Naiman
 *
 * NekoEngine Sound Subsystem
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2019, Alexandru Naiman
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
 */

#include <stdlib.h>

#include <engine/resource.h>
#include <sound/sound_src.h>
#include <sound/sound_defs.h>

struct ne_sound_src
{
	ne_sound_clip *clip;
	ALuint src;
};

ne_sound_src *
snd_src_init(void)
{
	ne_sound_src *src = calloc(1, sizeof(*src));
	if (!src)
		return NULL;

	alGenSources(1, &src->src);
	alSourcef(src->src, AL_PITCH, 1.f);
	alSourcef(src->src, AL_GAIN, 1.f);
	alSource3f(src->src, AL_POSITION, 0.f, 0.f, 0.f);
	alSource3f(src->src, AL_VELOCITY, 0.f, 0.f, 0.f);

	return src;
}

void
snd_src_pitch(
	ne_sound_src *src,
	float f)
{
	alSourcef(src->src, AL_PITCH, f);
}

void
snd_src_gain(
	ne_sound_src *src,
	float f)
{
	alSourcef(src->src, AL_GAIN, f);
}

void
snd_src_cone_inner_angle(
	ne_sound_src *src,
	float f)
{
	alSourcef(src->src, AL_CONE_INNER_ANGLE, f);
}

void
snd_src_cone_outer_angle(
	ne_sound_src *src,
	float f)
{
	alSourcef(src->src, AL_CONE_OUTER_ANGLE, f);
}

void
snd_src_cone_outer_gain(
	ne_sound_src *src,
	float f)
{
	alSourcef(src->src, AL_CONE_OUTER_GAIN, f);
}

void
snd_src_direction(
	ne_sound_src *src,
	kmVec3 *v)
{
	alSource3f(src->src, AL_DIRECTION, v->x, v->y, v->z);
}

void
snd_src_position(
	ne_sound_src *src,
	kmVec3 *v)
{
	alSource3f(src->src, AL_POSITION, v->x, v->y, v->z);
}

void
snd_src_velocity(
	ne_sound_src *src,
	kmVec3 *v)
{
	alSource3f(src->src, AL_VELOCITY, v->x, v->y, v->z);
}

void
snd_src_loop(
	ne_sound_src *src,
	bool loop)
{
	alSourcei(src->src, AL_LOOPING, loop);
}

void
snd_src_max_distance(
	ne_sound_src *src,
	float d)
{
	alSourcef(src->src, AL_MAX_DISTANCE, d);
}

void
snd_src_ref_distance(
	ne_sound_src *src,
	float d)
{
	alSourcef(src->src, AL_REFERENCE_DISTANCE, d);
}

void
snd_src_play(ne_sound_src *src)
{
	alSourcePlay(src->src);
}

void
snd_src_pause(ne_sound_src *src)
{
	alSourcePause(src->src);
}

void
snd_src_stop(ne_sound_src *src)
{
	alSourceStop(src->src);
}

void
snd_src_rewind(ne_sound_src *src)
{
	alSourceRewind(src->src);
}

bool
snd_src_playing(ne_sound_src *src)
{
	ALenum state;
	alGetSourcei(src->src, AL_SOURCE_STATE, &state);
	return state == AL_PLAYING;
}

ne_sound_clip *
snd_src_set_clip(
	ne_sound_src *src,
	ne_sound_clip *clip)
{
	ne_sound_clip *old = src->clip;

	src->clip = clip;
	alSourcei(src->src, AL_BUFFER, clip->buffer);

	return old;
}

void
snd_src_destroy(ne_sound_src *src)
{
	alSourceStop(src->src);
	alSourcei(src->src, AL_BUFFER, AL_NONE);
	alDeleteSources(1, &src->src);

	if (src->clip)
		res_unload(src->clip, RES_SOUND);

	free(src);
}

ne_status
snd_src_comp_create(
	void *src,
	const void **args)
{
	struct ne_sound_src_comp *comp = src;

	comp->src = snd_src_init();

	return comp->src ? NE_OK : NE_FAIL;
}

void
snd_src_comp_destroy(void *src)
{
	struct ne_sound_src_comp *comp = src;

	snd_src_destroy(comp->src);
}

