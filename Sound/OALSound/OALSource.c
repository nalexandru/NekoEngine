#include <Audio/Source.h>
#include <Engine/Resource.h>

#include "OALSound.h"

size_t Au_SourceSize = sizeof(struct AudioSource);

void
Au_SetClip(struct AudioSource *src, Handle clip)
{
	//struct AudioClip *ac = (struct AudioClip *)E_ResourcePtr(clip);
	//if (ac)
	//	alSourcei(src->src, AL_BUFFER, ac->buffer);
}

void
Au_Play(struct AudioSource *src)
{
	alSourcePlay(src->src);
}

void
Au_Gain(struct AudioSource *src, float gain)
{
	alSourcef(src->src, AL_GAIN, 1.f);
}

bool
Au_InitSource(struct AudioSource *src)
{
	alGenSources(1, &src->src);
	if (!src->src)
		return false;

	alSourcef(src->src, AL_PITCH, 1.f);
	alSourcef(src->src, AL_GAIN, 1.f);
	alSource3f(src->src, AL_POSITION, 0.f, 0.f, 0.f);
	alSource3f(src->src, AL_VELOCITY, 0.f, 0.f, 0.f);

	return true;
}

void
Au_TermSource(struct AudioSource *src)
{
	alSourceStop(src->src);
	alSourcei(src->src, AL_BUFFER, AL_NONE);
	alDeleteSources(1, &src->src);
}

