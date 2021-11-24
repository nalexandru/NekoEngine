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
Au_InitSource(struct AudioSource *src)
{
	return false;
}

void
Au_TermSource(struct AudioSource *src)
{
}

void Au_SetClip(struct AudioSource *src, Handle clip)
{
}

void
Au_Play(struct AudioSource *src)
{
}

void
Au_Gain(struct AudioSource *src, float gain)
{
}

bool
Au_InitSourceComponent(struct AudioSource *src, const void **args)
{
/*	const char *clip = NULL;

	for (; *args; ++args) {
		const char *fuckCpp = (const char *)*args; // in sane languages this cast is NOT NEEDED
		size_t len = strlen(fuckCpp);

		if (!strncmp(fuckCpp, "clip", len))
			clip = (const char *)*(++args);
	}

	if (!Au_InitSource(src))
		return false;

	if (clip)
		Au_SetClip(src, E_LoadResource(clip, "AudioClip"));*/

	return true;
}

void
Au_TermSourceComponent(struct AudioSource *src)
{
	//Au_TermSource(src);
}

#ifdef __APPLE__
#	pragma clang diagnostic pop
#endif
