#ifndef _AV_AVSOUND_H_
#define _AV_AVSOUND_H_

#define Handle __EngineHandle

#include <Engine/Types.h>
#include <Engine/Resource.h>
#include <Engine/Component.h>

#include <Audio/Clip.h>
#include <Audio/Audio.h>
#include <Audio/Source.h>

#undef Handle

#import <AVFoundation/AVFoundation.h>

struct AudioSource
{
	COMPONENT_BASE;

	AVAudioPlayerNode *src;
	struct AudioClip *clip;
};

struct AudioClipData
{
	AVAudioPCMBuffer *buffer;
};

extern AVAudioEngine *_avEngine;

bool Au_InitSourceComponent(struct AudioSource *, const void **);
void Au_TermSourceComponent(struct AudioSource *);

#endif /* _AV_AVSOUND_H_ */
