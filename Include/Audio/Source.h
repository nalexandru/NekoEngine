#ifndef _AU_SOURCE_H_
#define _AU_SOURCE_H_

#include <Engine/Types.h>

struct AudioSourceProcs
{
	void *a;
};

extern struct AudioSourceProcs Au_sourceProcs;

// Shared component handling
bool Au_InitSourceComponent(struct AudioSource *src, const void **);
void Au_TermSourceComponent(struct AudioSource *src);

// Implemented by the sound library
bool Au_InitSource(struct AudioSource *src);
void Au_TermSource(struct AudioSource *src);

void Au_SetClip(struct AudioSource *src, Handle clip);
void Au_Play(struct AudioSource *src);
void Au_Gain(struct AudioSource *src, float gain);

#endif /* _AU_SOURCE_H_ */
