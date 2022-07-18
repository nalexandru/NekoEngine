#include <stdio.h>
#include <string.h>

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
#include <Audio/Clip.h>
#include <Audio/Source.h>
#include <Engine/Resource.h>
#include <Engine/Component.h>
#include <Scene/Components.h>

struct NeAudioSource
{
	ALuint id;
	NeHandle clip;
};

static bool _InitSource(struct NeAudioSource *src, const void **args);
static void _TermSource(struct NeAudioSource *src);

E_REGISTER_COMPONENT(AUDIO_SOURCE_COMP, struct NeAudioSource, 16, _InitSource, _TermSource)

bool
Au_InitSource(struct NeAudioSource *src)
{
	alGenSources(1, &src->id);
	if (alGetError() != AL_NO_ERROR)
		return false;

	return false;
}

void
Au_SetPosition(struct NeAudioSource *src, struct NeVec3 *v)
{
	alSource3f(src->id, AL_POSITION, v->x, v->y, v->z);
}

void
Au_SetVelocity(struct NeAudioSource *src, struct NeVec3 *v)
{
	alSource3f(src->id, AL_VELOCITY, v->x, v->y, v->z);
}

void
Au_SetDirection(struct NeAudioSource *src, struct NeVec3 *v)
{
	alSource3f(src->id, AL_DIRECTION, v->x, v->y, v->z);
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

	if (clip == E_INVALID_HANDLE) {
		alSourcei(src->id, AL_BUFFER, 0);
	} else {
		const struct NeAudioClip *ac = E_ResourcePtr(clip);
		alSourcei(src->id, AL_BUFFER, ac->handle);
	}

	if (src->clip != E_INVALID_HANDLE)
		E_UnloadResource(src->clip);
	src->clip = clip;
}

void
Au_TermSource(struct NeAudioSource *src)
{
	Au_SetClip(src, E_INVALID_HANDLE);
	alDeleteSources(1, &src->id);
}

static bool
_InitSource(struct NeAudioSource *src, const void **args)
{
	const char *clip = NULL;

	for (; args && *args; ++args) {
		const char *arg = *args;
		size_t len = strlen(arg);

		if (!strncmp(arg, "clip", len))
			clip = (const char *)*(++args);
	}

	if (!Au_InitSource(src))
		return false;

	if (clip)
		Au_SetClip(src, E_LoadResource(clip, "NeAudioClip"));

	return true;
}

static void
_TermSource(struct NeAudioSource *src)
{
	Au_TermSource(src);
}

#ifdef __APPLE__
#	pragma clang diagnostic pop
#endif
