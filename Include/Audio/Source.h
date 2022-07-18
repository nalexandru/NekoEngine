#ifndef _NE_AUDIO_SOURCE_H_
#define _NE_AUDIO_SOURCE_H_

#include <Math/Math.h>
#include <Engine/Types.h>

bool Au_InitSource(struct NeAudioSource *src);
void Au_SetPosition(struct NeAudioSource *src, struct NeVec3 *v);
void Au_SetVelocity(struct NeAudioSource *src, struct NeVec3 *v);
void Au_SetDirection(struct NeAudioSource *src, struct NeVec3 *v);
void Au_Play(struct NeAudioSource *src);
void Au_Pause(struct NeAudioSource *src);
void Au_Stop(struct NeAudioSource *src);
void Au_Rewind(struct NeAudioSource *src);
bool Au_IsPlaying(struct NeAudioSource *src);
void Au_Gain(struct NeAudioSource *src, float gain);
void Au_Pitch(struct NeAudioSource *src, float pitch);
void Au_Cone(struct NeAudioSource *src, float innerAngle, float outerAngle, float outerGain);
void Au_MaxDistance(struct NeAudioSource *src, float d);
void Au_RefDistance(struct NeAudioSource *src, float d);
void Au_SetClip(struct NeAudioSource *src, NeHandle clip);
void Au_TermSource(struct NeAudioSource *src);

#endif /* _NE_AUDIO_SOURCE_H_ */
