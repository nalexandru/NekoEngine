#ifndef _AU_CLIP_H_
#define _AU_CLIP_H_

#include <Engine/Types.h>

#define RES_AUDIO_CLIP	"AudioClip"

enum AudioFormat
{
	AF_INT_16,
	AF_INT_32,
	AF_FLOAT_32,
	AF_FLOAT_64
};

struct AudioClip
{
	uint16_t *data;
	uint32_t byteSize;
	uint32_t bitsPerSample;
	uint32_t sampleRate;
	uint32_t channels;
	enum AudioFormat format;
	uint8_t soundSystemData;
};

struct AudioClipCreateInfo
{
	uint16_t *samples;
	uint32_t sampleCount;
};

// Shared resource handling
bool Au_CreateClip(const char *name, const struct AudioClipCreateInfo *ci, struct AudioClip *data, Handle h);
bool Au_LoadClip(struct ResourceLoadInfo *li, const char *args, struct AudioClip *model, Handle h);
void Au_UnloadClip(struct AudioClip *model, Handle h);

// Implemented in the sound library
bool Au_InitClip(struct AudioClip *clip);
void Au_TermClip(struct AudioClip *clip);

#endif /* _AU_CLIP_H_ */
