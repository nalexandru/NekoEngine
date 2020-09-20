#ifndef _AU_AUDIO_H_
#define _AU_AUDIO_H_

#include <stdbool.h>

#include <Audio/Clip.h>
#include <Audio/Source.h>

#ifdef __cplusplus
extern "C" {
#endif

bool Au_Init(void);
void Au_Term(void);

// Implemented by the sound library
bool Au_InitLib(void);
void Au_TermLib(void);

#ifdef __cplusplus
}
#endif

#endif /* _AU_AUDIO_H_ */