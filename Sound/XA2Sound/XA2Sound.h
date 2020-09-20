#ifndef _XA2_XA2SOUND_H_
#define _XA2_XA2SOUND_H_

#include <xaudio2.h>
#include <x3daudio.h>
#include <Engine/Types.h>
#include <Engine/Component.h>

#include <Audio/Clip.h>
#include <Audio/Source.h>

struct AudioSource
{
	COMPONENT_BASE;

	IXAudio2SourceVoice *sourceVoice;
	X3DAUDIO_EMITTER emitter;
	struct AudioClip *clip;
};

extern IXAudio2 *_auAudio;
extern X3DAUDIO_HANDLE _au3DAudio;
extern X3DAUDIO_LISTENER _auListener;

bool Au_InitSourceComponent(AudioSource, const void **);
void Au_TermSourceComponent(AudioSource);

bool Au_CreateClip(const char *name, const struct AudioClipCreateInfo *ci, struct AudioClip *data, Handle h);
bool Au_LoadClip(const struct ResourceLoadInfo *li, const char *args, struct AudioClip *model, Handle h);
void Au_UnloadClip(struct AudioClip *model, Handle h);

#endif /* _AU_AUDIOIMPL_H_ */
