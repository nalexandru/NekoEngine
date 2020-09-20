#ifndef _OAL_OALSOUND_H_
#define _OAL_OALSOUND_H_

#include <AL/al.h>

#include <Engine/Types.h>
#include <Engine/Component.h>

#include <Audio/Clip.h>
#include <Audio/Source.h>

struct AudioSource
{
	COMPONENT_BASE;

	ALuint src;
	struct AudioClip *clip;
};

bool Au_InitSourceComponent(struct AudioSource *, const void **);
void Au_TermSourceComponent(struct AudioSource *);

#endif /* _OAL_OALSOUND_H_ */
