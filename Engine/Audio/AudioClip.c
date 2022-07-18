#include <stdlib.h>

#ifdef __APPLE__
#	include <OpenAL/al.h>
#	include <OpenAL/alc.h>
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#	include <AL/al.h>
#	include <AL/alc.h>
#endif

#include <Audio/Clip.h>
#include <Engine/Asset.h>
#include <Engine/Types.h>
#include <System/Memory.h>
#include <Engine/Resource.h>

static inline bool _InitClip(struct NeAudioClip *ac);

bool
Au_CreateClip(const char *name, const struct NeAudioClipCreateInfo *ci, struct NeAudioClip *ac, NeHandle h)
{
	ac->data = ci->samples;
	ac->byteSize = ci->sampleCount * 2;

	return _InitClip(ac);
}

bool
Au_LoadClip(struct NeResourceLoadInfo *li, const char *args, struct NeAudioClip *ac, NeHandle h)
{
	if (!E_LoadWaveAsset(&li->stm, ac))
		return false;

	return _InitClip(ac);
}

void
Au_UnloadClip(struct NeAudioClip *ac, NeHandle h)
{
	alDeleteBuffers(1, &ac->handle);
	Sys_Free((void *)ac->data);
}

static inline bool
_InitClip(struct NeAudioClip *ac)
{
	ALenum format;

	alGenBuffers(1, &ac->handle);
	if (alGetError() != AL_NO_ERROR)
		return false;

	if (ac->bitsPerSample == 8)
		format = ac->channels == 1 ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;
	else if (ac->bitsPerSample == 16)
		format = ac->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
	else
		goto error;

	alBufferData(ac->handle, format, ac->data, ac->byteSize, ac->sampleRate);
	if (alGetError() != AL_NO_ERROR)
		goto error;

	return true;

error:
	alDeleteBuffers(1, &ac->handle);
	return false;
}

#ifdef __APPLE__
#	pragma clang diagnostic pop
#endif
