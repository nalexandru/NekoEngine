#include "AVSound.h"

size_t Au_SourceSize = sizeof(AVAudioPCMBuffer *);

void
Au_SetClip(struct AudioSource *src, __EngineHandle clip)
{
	struct AudioClip *ac = (struct AudioClip *)E_ResourcePtr(clip);
	
	if (!ac)
		return;
	
	struct AudioClipData *acd = (struct AudioClipData *)&ac->soundSystemData;
	[src->src scheduleBuffer:acd->buffer completionHandler: nil];
}

void
Au_Play(struct AudioSource *src)
{
	[src->src play];
}

void
Au_Gain(struct AudioSource *src, float gain)
{
	//alSourcef(src->src, AL_GAIN, 1.f);
}

bool
Au_InitSource(struct AudioSource *src)
{
	/*alSourcef(src->src, AL_PITCH, 1.f);
	alSourcef(src->src, AL_GAIN, 1.f);
	alSource3f(src->src, AL_POSITION, 0.f, 0.f, 0.f);
	alSource3f(src->src, AL_VELOCITY, 0.f, 0.f, 0.f);*/

	src->src = [[AVAudioPlayerNode alloc] init];
	if (!src->src)
		return false;
	
	[_avEngine attachNode: src->src];
	
	return true;
}

void
Au_TermSource(struct AudioSource *src)
{
	[src->src stop];
	[_avEngine detachNode: src->src];
	[src->src release];
}

