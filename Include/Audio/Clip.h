#ifndef _NE_AUDIO_CLIP_H_
#define _NE_AUDIO_CLIP_H_

#include <Engine/Types.h>

#define RES_AUDIO_CLIP	"AudioClip"

enum NeAudioFormat
{
	AF_INT_16,
	AF_INT_32,
	AF_FLOAT_32,
	AF_FLOAT_64
};

struct NeAudioClip
{
	uint16_t *data;
	uint32_t byteSize;
	uint32_t bitsPerSample;
	uint32_t sampleRate;
	uint32_t channels;
	enum NeAudioFormat format;
	uint8_t soundSystemData;
};

struct NeAudioClipCreateInfo
{
	uint16_t *samples;
	uint32_t sampleCount;
};

bool Au_CreateClip(const char *name, const struct NeAudioClipCreateInfo *ci, struct NeAudioClip *data, NeHandle h);
bool Au_LoadClip(struct NeResourceLoadInfo *li, const char *args, struct NeAudioClip *model, NeHandle h);
void Au_UnloadClip(struct NeAudioClip *model, NeHandle h);

bool Au_InitClip(struct NeAudioClip *clip);
void Au_TermClip(struct NeAudioClip *clip);

#endif /* _NE_AUDIO_CLIP_H_ */
