#include <stdlib.h>

#include <Audio/Clip.h>
#include <Engine/Asset.h>
#include <Engine/Types.h>
#include <System/Memory.h>
#include <Engine/Resource.h>

bool
Au_CreateClip(const char *name, const struct AudioClipCreateInfo *ci, struct AudioClip *ac, Handle h)
{
	ac->data = ci->samples;
	ac->byteSize = ci->sampleCount * 2;

	return Au_InitClip(ac);
}

bool
Au_LoadClip(struct ResourceLoadInfo *li, const char *args, struct AudioClip *ac, Handle h)
{
	if (!E_LoadWaveAsset(&li->stm, ac))
		return false;

	return Au_InitClip(ac);
}

void
Au_UnloadClip(struct AudioClip *ac, Handle h)
{
	Au_TermClip(ac);

	Sys_Free((void *)ac->data);
}
