#ifndef _AU_AUDIO_H_
#define _AU_AUDIO_H_

#include <stdbool.h>

#include <Audio/Clip.h>
#include <Audio/Source.h>

bool Au_Init(void);
void Au_Term(void);

// Implemented by the sound library
bool Au_InitLib(void);
void Au_TermLib(void);

#endif /* _AU_AUDIO_H_ */
