#include "XA2Sound.h"

size_t Au_audioClipDataSize = sizeof(XAUDIO2_BUFFER);

bool
Au_InitClip(struct AudioClip *ac)
{
	XAUDIO2_BUFFER *buff = (XAUDIO2_BUFFER *)&ac->soundSystemData;

	buff->pAudioData = (BYTE *)ac->data;
	buff->AudioBytes = ac->byteSize;

	return true;
}

void
Au_TermClip(struct AudioClip *ac)
{
	//
}
