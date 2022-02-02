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

bool
Au_CreateClip(const char *name, const struct NeAudioClipCreateInfo *ci, struct NeAudioClip *ac, NeHandle h)
{
/*	ac->data = ci->samples;
	ac->byteSize = ci->sampleCount * 2;

	return Au_InitClip(ac);*/
	return false;
}

bool
Au_LoadClip(struct NeResourceLoadInfo *li, const char *args, struct NeAudioClip *ac, NeHandle h)
{
/*	if (!E_LoadWaveAsset(&li->stm, ac))
		return false;

	return Au_InitClip(ac);*/
	return false;
}

void
Au_UnloadClip(struct NeAudioClip *ac, NeHandle h)
{
//	Au_TermClip(ac);

//	Sys_Free((void *)ac->data);
}

bool
Au_InitClip(struct NeAudioClip *clip)
{
	return false;
}

void
Au_TermClip(struct NeAudioClip *clip)
{
}

#ifdef __APPLE__
#	pragma clang diagnostic pop
#endif
