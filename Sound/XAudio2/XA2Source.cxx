#ifdef _XBOX
#	include <xtl.h>
#else
#	include <Windows.h>
#endif

#include <x3daudio.h>

#include <Audio/Source.h>
#include <Engine/Resource.h>

#include "XA2Sound.h"

size_t Au_SourceSize = sizeof(struct AudioSource);

// https://docs.microsoft.com/en-us/previous-versions/dd757713(v=vs.85)?redirectedfrom=MSDN
static WAVEFORMATEX _auWaveFormat = 
{
	WAVE_FORMAT_PCM,
	2,
	44100,
	176400,
	4,
	16,
	0
};

void
Au_SetClip(struct AudioSource *src, Handle clip)
{
	struct AudioClip *ac = (struct AudioClip *)E_ResourcePtr(clip);
	if (ac)
		src->sourceVoice->SubmitSourceBuffer((XAUDIO2_BUFFER *)&ac->soundSystemData);
}

void
Au_Play(struct AudioSource *src)
{
	src->sourceVoice->Start();
}

void
Au_Gain(struct AudioSource *src, float gain)
{
	//
}

bool
Au_InitSource(struct AudioSource *src)
{
	HRESULT hr = _auAudio->CreateSourceVoice(&src->sourceVoice, &_auWaveFormat);

	if (FAILED(hr))
		return false;


	src->emitter.ChannelCount = 2;
	src->emitter.CurveDistanceScaler = FLT_MIN;

	return true;
}

void
Au_TermSource(struct AudioSource *src)
{
	src->sourceVoice->Stop();
	src->sourceVoice->DestroyVoice();
}

