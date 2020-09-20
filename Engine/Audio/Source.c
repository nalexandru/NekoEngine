#include <stdio.h>
#include <string.h>

#include <Audio/Source.h>
#include <Engine/Resource.h>

bool
Au_InitSourceComponent(struct AudioSource *src, const void **args)
{
	const char *clip = NULL;

	for (args; *args; ++args) {
		const char *fuckCpp = (const char *)*args; // in sane languages this cast is NOT NEEDED
		size_t len = strlen(fuckCpp);

		if (!strncmp(fuckCpp, "clip", len))
			clip = (const char *)*(++args);
	}

	if (!Au_InitSource(src))
		return false;

	if (clip)
		Au_SetClip(src, E_LoadResource(clip, "AudioClip"));

	return true;
}

void
Au_TermSourceComponent(struct AudioSource *src)
{
	Au_TermSource(src);
}
