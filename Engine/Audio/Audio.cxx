#ifdef __APPLE__
#	include <OpenAL/al.h>
#	include <OpenAL/alc.h>
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#	include <AL/al.h>
#	include <AL/alc.h>
#endif

#include <Math/Math.h>
#include <System/Log.h>
#include <System/System.h>
#include <System/Memory.h>
#include <Audio/Audio.h>
#include <Audio/Device.h>
#include <Engine/Config.h>
#include <Engine/ECSystem.h>
#include <Engine/Resource.h>
#include <Engine/Component.h>
#include <Scene/Components.h>
#include <Scene/Transform.h>
#include <Scene/Camera.h>

#define AU_MOD				"Audio"
#define AU_UPDATE_LISTENER	"Au_UpdateListener"
#define CHK_FAIL(x, y) if (!x) { Sys_LogEntry(AU_MOD, LOG_CRITICAL, y); return false; }

struct NeAudioListener
{
	NE_COMPONENT_BASE;

	bool enabled;
	struct NeVec3 lastPosition;
};

static bool _InitListener(struct NeAudioListener *src, const char **args);
static void _TermListener(struct NeAudioListener *src);

E_REGISTER_COMPONENT(AUDIO_LISTENER_COMP, struct NeAudioListener, 1, _InitListener, _TermListener)

ALCdevice *Au_device = NULL;
ALCcontext *Au_context = NULL;

struct NeAudioDeviceInfo Au_deviceInfo = { 0 };

static ALenum NeToALDistanceModel[] =
{
	AL_INVERSE_DISTANCE,
	AL_INVERSE_DISTANCE_CLAMPED,
	AL_LINEAR_DISTANCE,
	AL_LINEAR_DISTANCE_CLAMPED,
	AL_EXPONENT_DISTANCE,
	AL_EXPONENT_DISTANCE_CLAMPED
};

bool
Au_Init(void)
{
	if (CVAR_BOOL("Audio_Disable"))
		return true;

	Au_device = alcOpenDevice(NULL);
	if (!Au_device)
		goto error;

	Au_context = alcCreateContext(Au_device, NULL);
	if (!Au_context)
		goto error;

	if (!alcMakeContextCurrent(Au_context))
		goto error;

	alDistanceModel(AL_LINEAR_DISTANCE);

	alGetError();

	E_RegisterResourceType(RES_AUDIO_CLIP, sizeof(struct NeAudioClip),
		(NeResourceCreateProc)Au_CreateClip, (NeResourceLoadProc)Au_LoadClip, (NeResourceUnloadProc)Au_UnloadClip);

	return true;

error:
	if (Au_context)
		alcDestroyContext(Au_context);

	if (Au_device)
		alcCloseDevice(Au_device);

	return false;
}

void
Au_DistanceModel(enum NeDistanceModel model)
{
	alDistanceModel(NeToALDistanceModel[model]);
}

void
Au_ListenerPosition(const struct NeVec3 *v)
{
	alListener3f(AL_POSITION, v->x, v->y, v->z);
}

void
Au_ListenerPositionF(float x, float y, float z)
{
	alListener3f(AL_POSITION, x, y, z);
}

void
Au_ListenerVelocity(const struct NeVec3 *v)
{
	alListener3f(AL_VELOCITY, v->x, v->y, v->z);
}

void
Au_ListenerVelocityF(float x, float y, float z)
{
	alListener3f(AL_VELOCITY, x, y, z);
}

void
Au_ListenerOrientation(const float *orientation)
{
	alListenerfv(AL_ORIENTATION, orientation);
}

void
Au_Term(void)
{
	alcMakeContextCurrent(NULL);

	if (Au_context)
		alcDestroyContext(Au_context);

	if (Au_device)
		alcCloseDevice(Au_device);
}

static bool
_InitListener(struct NeAudioListener *l, const char **args)
{
	l->enabled = true;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "enabled", len))
			l->enabled = !strncmp(*(++args), "true", 4);
	}

	return true;
}

static void
_TermListener(struct NeAudioListener *l)
{
	l->enabled = false;
}

E_SYSTEM(AU_UPDATE_LISTENER, ECSYS_GROUP_POST_LOGIC, 0, true, void, 2, TRANSFORM_COMP, AUDIO_LISTENER_COMP)
{
	float orientation[6];
	struct NeVec3 velocity{ 0.f, 0.f, 0.f };
	const struct NeTransform *xform = (struct NeTransform *)comp[0];
	struct NeAudioListener *l = (struct NeAudioListener *)comp[1];

	if (l->enabled) {
		alListener3f(AL_POSITION, xform->position.x, xform->position.y, xform->position.z);

		M_Store(&velocity, XMVectorSubtract(M_Load(&xform->position), M_Load(&l->lastPosition)));
		alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);

		memcpy(orientation, &xform->right.x, sizeof(float) * 3);
		memcpy(&orientation[3], &xform->up.x, sizeof(float) * 3);
		alListenerfv(AL_ORIENTATION, orientation);

		memcpy(&l->lastPosition, &xform->position, sizeof(l->lastPosition));
	}
}

#ifdef __APPLE__
#	pragma clang diagnostic pop
#endif

/* NekoEngine
 *
 * Audio.c
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
