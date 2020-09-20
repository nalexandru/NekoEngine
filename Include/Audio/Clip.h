#ifndef _AU_CLIP_H_
#define _AU_CLIP_H_

#include <Engine/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RES_AUDIO_CLIP	"AudioClip"

struct AudioClip
{
	uint16_t *data;
	uint32_t byteSize;
	uint8_t soundSystemData;
};

struct AudioClipCreateInfo
{
	uint16_t *samples;
	uint32_t sampleCount;
};

extern size_t Au_AudioClipDataSize;

// Shared resource handling
bool Au_CreateClip(const char *name, const struct AudioClipCreateInfo *ci, struct AudioClip *data, Handle h);
bool Au_LoadClip(struct ResourceLoadInfo *li, const char *args, struct AudioClip *model, Handle h);
void Au_UnloadClip(struct AudioClip *model, Handle h);

// Implemented in the sound library
bool Au_InitClip(struct AudioClip *clip);
void Au_TermClip(struct AudioClip *clip);

#ifdef __cplusplus
}
#endif

#endif /* _AU_CLIP_H_ */
