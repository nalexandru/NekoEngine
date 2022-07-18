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

static bool _InitListener(struct NeAudioListener *src, const void **args);
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
Au_ListenerPosition(struct NeVec3 *v)
{
	alListener3f(AL_POSITION, v->x, v->y, v->z);
}

void
Au_ListenerVelocity(struct NeVec3 *v)
{
	alListener3f(AL_VELOCITY, v->x, v->y, v->z);
}

void
Au_ListenerOrientation(float *orientation)
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
_InitListener(struct NeAudioListener *l, const void **args)
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
	struct NeTransform *xform = comp[0];
	struct NeAudioListener *l = comp[1];
	float orientation[6];
	struct NeVec3 velocity;

	if (l->enabled) {
		alListener3f(AL_POSITION, xform->position.x, xform->position.y, xform->position.z);

		M_SubVec3(&velocity, &xform->position, &l->lastPosition);
		alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);

		memcpy(orientation, &xform->right.x, sizeof(float) * 3);
		memcpy(&orientation[3], &xform->up.x, sizeof(float) * 3);
		alListenerfv(AL_ORIENTATION, orientation);

		M_CopyVec3(&l->lastPosition, &xform->position);
	}
}

#ifdef __APPLE__
#	pragma clang diagnostic pop
#endif
