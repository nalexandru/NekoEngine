#ifndef _NE_AUDIO_SOURCE_H_
#define _NE_AUDIO_SOURCE_H_

#include <Engine/Types.h>

bool Au_InitSource(struct NeAudioSource *src);
void Au_TermSource(struct NeAudioSource *src);

void Au_SetClip(struct NeAudioSource *src, NeHandle clip);
void Au_Play(struct NeAudioSource *src);
void Au_Gain(struct NeAudioSource *src, float gain);

bool Au_InitSourceComponent(struct NeAudioSource *src, const void **);
void Au_TermSourceComponent(struct NeAudioSource *src);

#endif /* _NE_AUDIO_SOURCE_H_ */
