#include "AVSound.h"

size_t Au_AudioClipDataSize = sizeof(struct AudioClipData);

bool
Au_InitClip(struct AudioClip *ac)
{
	if (!ac)
		return false;
	
	NSUInteger pcmFormat = AVAudioOtherFormat;
	struct AudioClipData *acd = (struct AudioClipData *)&ac->soundSystemData;
	
	switch (ac->format) {
	case AF_INT_16: pcmFormat = AVAudioPCMFormatInt16;
	case AF_INT_32: pcmFormat = AVAudioPCMFormatInt32;
	case AF_FLOAT_32: pcmFormat = AVAudioPCMFormatFloat32;
	case AF_FLOAT_64: pcmFormat = AVAudioPCMFormatFloat64;
	}
		
	AVAudioFormat *fmt = [[AVAudioFormat alloc] initWithCommonFormat: pcmFormat
														  sampleRate: ac->sampleRate
															channels: ac->channels
														 interleaved: false];
	if (!fmt)
		return false;
	
	acd->buffer = [[AVAudioPCMBuffer alloc] initWithPCMFormat:fmt
												frameCapacity: ac->byteSize / fmt.streamDescription->mBytesPerFrame];
	if (!acd->buffer)
		return false;
	
	//https://developer.apple.com/forums/thread/24124
	//memcpy(acd->buffer.floatChannelData, ac->data, ac->byteSize);
	
	switch (ac->format) {
	case AF_INT_16: pcmFormat = AVAudioPCMFormatInt16;
	case AF_INT_32: pcmFormat = AVAudioPCMFormatInt32;
	case AF_FLOAT_32: pcmFormat = AVAudioPCMFormatFloat32;
	case AF_FLOAT_64: pcmFormat = AVAudioPCMFormatFloat64;
	}
	
	//ALuint *buff = (ALuint *)&ac->soundSystemData;

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
