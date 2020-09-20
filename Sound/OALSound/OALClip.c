#include <Audio/Clip.h>

#include "OALSound.h"

size_t Au_AudioClipDataSize = sizeof(ALuint);

bool
Au_InitClip(struct AudioClip *ac)
{
	ALuint *buff = (ALuint *)&ac->soundSystemData;

	/*alGetBuffers(1, buff);
	alBufferData(*buff, 
	
	buff->pAudioData = (BYTE *)ac->data;
	buff->AudioBytes = ac->byteSize;*/

	return true;
}

void
Au_TermClip(struct AudioClip *ac)
{
	//
}
