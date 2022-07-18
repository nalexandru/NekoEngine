#ifndef _NE_AUDIO_AUDIO_H_
#define _NE_AUDIO_AUDIO_H_

#include <stdbool.h>

#include <Audio/Clip.h>
#include <Audio/Source.h>

enum NeDistanceModel
{
	DM_INVERSE_DISTANCE,
	DM_INVERSE_DISTANCE_CLAMPED,
	DM_LINEAR_DISTANCE,
	DM_LINEAR_DISTANCE_CLAMPED,
	DM_EXPONENT_DISTANCE,
	DM_EXPONENT_DISTANCE_CLAMPED
};

bool Au_Init(void);

void Au_DistanceModel(enum NeDistanceModel model);

void Au_ListenerPosition(struct NeVec3 *v);
void Au_ListenerVelocity(struct NeVec3 *v);
void Au_ListenerOrientation(float *orientation);

void Au_Term(void);

#endif /* _NE_AUDIO_AUDIO_H_ */
