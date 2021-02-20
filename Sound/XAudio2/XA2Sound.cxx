#include <xaudio2.h>
#include <x3daudio.h>

#include "XA2Sound.h"

IXAudio2 *_auAudio;
X3DAUDIO_HANDLE _au3DAudio;
X3DAUDIO_LISTENER _auListener;

static X3DAUDIO_DSP_SETTINGS _auDSPSettings;
static IXAudio2MasteringVoice *_masterVoice;

bool
Au_InitLib(void)
{
	HRESULT hr;

	hr = XAudio2Create(&_auAudio);
	if (FAILED(hr))
		return false;

	hr = _auAudio->CreateMasteringVoice(&_masterVoice);
	if (FAILED(hr))
		return false;

/*	DWORD channelMask;
	_masterVoice->GetChannelMask(&channelMask);

	X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, _au3DAudio);

	ZeroMemory(&_auListener, sizeof(_auListener));
	ZeroMemory(&_auDSPSettings, sizeof(_auDSPSettings));

	XAUDIO2_VOICE_DETAILS voiceDetails;
	_masterVoice->GetVoiceDetails(&voiceDetails);

	_auDSPSettings.SrcChannelCount = 1;
	_auDSPSettings.DstChannelCount = voiceDetails.InputChannels;
	_auDSPSettings.pMatrixCoefficients = (FLOAT32 *)calloc(voiceDetails.InputChannels, sizeof(*_auDSPSettings.pMatrixCoefficients));*/

	return true;
}

/*void
Au_Update(void)
{
//	X3DAudioCalculate(_au3DAudio, &_auListener, )
}*/

void
Au_TermLib(void)
{
	free(_auDSPSettings.pMatrixCoefficients);

	if (_masterVoice)
		_masterVoice->DestroyVoice();

	if (_auAudio) {
		_auAudio->StopEngine();
		_auAudio->Release();
	}
}
