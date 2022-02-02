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

#include <Audio/Source.h>
#include <Engine/Resource.h>

bool
Au_InitSource(struct NeAudioSource *src)
{
	return false;
}

void
Au_TermSource(struct NeAudioSource *src)
{
}

void Au_SetClip(struct NeAudioSource *src, NeHandle clip)
{
}

void
Au_Play(struct NeAudioSource *src)
{
}

void
Au_Gain(struct NeAudioSource *src, float gain)
{
}

bool
Au_InitSourceComponent(struct NeAudioSource *src, const void **args)
{
/*	const char *clip = NULL;

	for (; *args; ++args) {

		if (!strncmp(fuckCpp, "clip", len))
			clip = (const char *)*(++args);
	}

	if (!Au_InitSource(src))
		return false;

	if (clip)
		Au_SetClip(src, E_LoadResource(clip, "NeAudioClip"));*/

	return true;
}

void
Au_TermSourceComponent(struct NeAudioSource *src)
{
	//Au_TermSource(src);
}

#ifdef __APPLE__
#	pragma clang diagnostic pop
#endif
