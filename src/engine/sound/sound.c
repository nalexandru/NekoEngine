/* NekoEngine
 *
 * sound.c
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

#include <system/log.h>

#include <sound/sound.h>
#include <sound/sound_clip.h>
#include <sound/sound_defs.h>
#include <engine/resource.h>

#define AUDIO_MODULE	"Audio"

static ALCdevice *_device;
static ALCcontext *_context;

static ALfloat _init_orientation[] =
{ 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };

ne_status
snd_init(void)
{
	if ((_device = alcOpenDevice(NULL)) == NULL) {
		log_entry(AUDIO_MODULE, LOG_CRITICAL,
				"Failed to open audio device.");
		return NE_SND_DEV_OPEN_FAIL;
	}

	if ((_context = alcCreateContext(_device, NULL)) == NULL) {
		snd_release();
		return NE_SND_CTX_CREATE_FAIL;
	}

	if (!alcMakeContextCurrent(_context)) {
		snd_release();
		return NE_SND_CTX_CREATE_FAIL;
	}

	alListener3f(AL_POSITION, 0.f, 0.f, 0.f);
	alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f);
	alListenerfv(AL_ORIENTATION, _init_orientation);

	alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);

	res_register_type(RES_SOUND, (res_load_proc)snd_clip_load,
			(res_unload_proc)snd_clip_destroy);

	return NE_OK;
}

void
snd_set_listener_position(
	float x,
	float y,
	float z)
{
	alListener3f(AL_POSITION, x, y, z);
}

void
snd_set_listener_velocity(
	float x,
	float y,
	float z)
{
	alListener3f(AL_VELOCITY, x, y, z);
}

void
snd_set_listener_orientation(
	float f_x,
	float f_y,
	float f_z,
	float u_x,
	float u_y,
	float u_z)
{
	float orientation[6] = { f_x, f_y, f_z, u_x, u_y, u_z };
	alListenerfv(AL_ORIENTATION, orientation);
}

void
snd_release(void)
{
	alcMakeContextCurrent(NULL);

	if (_context)
		alcDestroyContext(_context);
	_context = NULL;

	if (_device)
		alcCloseDevice(_device);
	_device = NULL;
}

