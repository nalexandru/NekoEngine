#include <xaudio2.h>
#include <x3daudio.h>

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

#define AU_MOD				"XAudio2"
#define AU_UPDATE_LISTENER	"Au_UpdateListener"
#define CHK_FAIL(x, y) if (!x) { Sys_LogEntry(AU_MOD, LOG_CRITICAL, y); return false; }

struct NeAudioListener
{
	NE_COMPONENT_BASE;

	bool enabled;
	struct NeVec3 lastPosition;
};

static bool InitListener(struct NeAudioListener *l, const char **args);
static void TermListener(struct NeAudioListener *l);

E_REGISTER_COMPONENT(AUDIO_LISTENER_COMP, struct NeAudioListener, 1, InitListener, TermListener)

IXAudio2 *Au_xAudio2 = nullptr;
IXAudio2MasteringVoice *Au_masterVoice = nullptr;
X3DAUDIO_HANDLE Au_x3dAudio{};
X3DAUDIO_LISTENER Au_listener{};

struct NeAudioDeviceInfo Au_deviceInfo = { 0 };

/*static ALenum NeToALDistanceModel[] =
{
	AL_INVERSE_DISTANCE,
	AL_INVERSE_DISTANCE_CLAMPED,
	AL_LINEAR_DISTANCE,
	AL_LINEAR_DISTANCE_CLAMPED,
	AL_EXPONENT_DISTANCE,
	AL_EXPONENT_DISTANCE_CLAMPED
};*/

bool
Au_Init(void)
{
	DWORD channelMask;

	if (CVAR_BOOL("Audio_Disable"))
		return true;

	if (FAILED(XAudio2Create(&Au_xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
		goto error;

	if (FAILED(Au_xAudio2->CreateMasteringVoice(&Au_masterVoice)))
		goto error;

	Au_masterVoice->GetChannelMask(&channelMask);
	if (FAILED(X3DAudioInitialize(channelMask, 343.f, Au_x3dAudio)))
		goto error;

	E_RegisterResourceType(RES_AUDIO_CLIP, sizeof(struct NeAudioClip),
		(NeResourceCreateProc)Au_CreateClip, (NeResourceLoadProc)Au_LoadClip, (NeResourceUnloadProc)Au_UnloadClip);

	return true;

error:
	if (Au_masterVoice)
		Au_masterVoice->DestroyVoice();

	if (Au_xAudio2)
		Au_xAudio2->Release();

	return false;
}

void
Au_DistanceModel(enum NeDistanceModel model)
{
	//alDistanceModel(NeToALDistanceModel[model]);
}

void
Au_ListenerPosition(const struct NeVec3 *v)
{
	memcpy(&Au_listener.Position, v, sizeof(float) * 3);
}

void
Au_ListenerPositionF(float x, float y, float z)
{
	Au_listener.Position.x = x;
	Au_listener.Position.y = y;
	Au_listener.Position.z = z;
}

void
Au_ListenerVelocity(const struct NeVec3 *v)
{
	memcpy(&Au_listener.Velocity, v, sizeof(float) * 3);
}

void
Au_ListenerVelocityF(float x, float y, float z)
{
	Au_listener.Velocity.x = x;
	Au_listener.Velocity.y = y;
	Au_listener.Velocity.z = z;
}

void
Au_ListenerOrientation(const float *orientation)
{
	//alListenerfv(AL_ORIENTATION, orientation);
}

void
Au_Term(void)
{
	if (Au_masterVoice)
		Au_masterVoice->DestroyVoice();

	if (Au_xAudio2)
		Au_xAudio2->Release();
}

static bool
InitListener(struct NeAudioListener *l, const char **args)
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
TermListener(struct NeAudioListener *l)
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
		Au_listener.Position.x = xform->position.x;
		Au_listener.Position.y = xform->position.y;
		Au_listener.Position.z = xform->position.z;

		M_Store(&velocity, XMVectorSubtract(M_Load(&xform->position), M_Load(&l->lastPosition)));
		Au_listener.Velocity.x = velocity.x;
		Au_listener.Velocity.y = velocity.y;
		Au_listener.Velocity.z = velocity.z;

		memcpy(orientation, &xform->right.x, sizeof(float) * 3);
		memcpy(&orientation[3], &xform->up.x, sizeof(float) * 3);
		//alListenerfv(AL_ORIENTATION, orientation);

		//Listener.OrientFront = ListenerOrientFront;
		//Listener.OrientTop = ListenerOrientTop;

		memcpy(&l->lastPosition, &xform->position, sizeof(l->lastPosition));
	}
}

/* NekoEngine
 *
 * Audio.cxx
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
