#ifndef _NE_AUDIO_SOURCE_H_
#define _NE_AUDIO_SOURCE_H_

#include <Engine/Types.h>

bool Au_InitSource(struct AudioSource *src);
void Au_TermSource(struct AudioSource *src);

void Au_SetClip(struct AudioSource *src, Handle clip);
void Au_Play(struct AudioSource *src);
void Au_Gain(struct AudioSource *src, float gain);

bool Au_InitSourceComponent(struct AudioSource *src, const void **);
void Au_TermSourceComponent(struct AudioSource *src);

#endif /* _NE_AUDIO_SOURCE_H_ */
