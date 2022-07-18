#ifndef _NE_AUDIO_CLIP_H_
#define _NE_AUDIO_CLIP_H_

#include <Engine/Types.h>
#include <Engine/Component.h>

#define RES_AUDIO_CLIP	"AudioClip"

struct NeAudioClip
{
	NE_COMPONENT_BASE;

	uint32_t handle;
	uint16_t *data;
	uint32_t byteSize;
	uint32_t bitsPerSample;
	uint32_t sampleRate;
	uint32_t channels;
};

struct NeAudioClipCreateInfo
{
	uint16_t *samples;
	uint32_t sampleCount;
};

bool Au_CreateClip(const char *name, const struct NeAudioClipCreateInfo *ci, struct NeAudioClip *data, NeHandle h);
bool Au_LoadClip(struct NeResourceLoadInfo *li, const char *args, struct NeAudioClip *model, NeHandle h);
void Au_UnloadClip(struct NeAudioClip *model, NeHandle h);

#endif /* _NE_AUDIO_CLIP_H_ */
