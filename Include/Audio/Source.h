#ifndef _AU_SOURCE_H_
#define _AU_SOURCE_H_

#include <Engine/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct AudioSource;

extern size_t Au_SourceSize;

// Shared component handling
bool Au_InitSourceComponent(struct AudioSource *src, const void **);
void Au_TermSourceComponent(struct AudioSource *src);

// Implemented by the sound library
bool Au_InitSource(struct AudioSource *src);
void Au_TermSource(struct AudioSource *src);

void Au_SetClip(struct AudioSource *src, Handle clip);
void Au_Play(struct AudioSource *src);
void Au_Gain(struct AudioSource *src, float gain);

#ifdef __cplusplus
}
#endif

#endif /* _AU_SOURCE_H_ */