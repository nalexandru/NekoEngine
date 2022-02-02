#ifdef __APPLE__
#	include <OpenAL/al.h>
#	include <OpenAL/alc.h>
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#	include <AL/al.h>
#	include <AL/alc.h>
#endif

#include <System/Log.h>
#include <System/System.h>
#include <System/Memory.h>
#include <Audio/Audio.h>
#include <Audio/Driver.h>
#include <Audio/Device.h>
#include <Engine/Config.h>
#include <Engine/Resource.h>
#include <Engine/Component.h>
#include <Scene/Components.h>

#define AU_MOD L"Audio"
#define CHK_FAIL(x, y) if (!x) { Sys_LogEntry(AU_MOD, LOG_CRITICAL, y); return false; }

ALCdevice *Au_device = NULL;
ALCcontext *Au_context = NULL;

struct NeAudioDeviceInfo Au_deviceInfo = { 0 };

bool
Au_Init(void)
{
/*	Au_device = alcOpenDevice(NULL);
	if (!Au_device)
		goto error;

	Au_context = alcCreateContext(Au_device, NULL);
	if (!Au_context)
		goto error;

	if (!alcMakeContextCurrent(Au_context))
		goto error;*/

	return true;

/*	E_RegisterResourceType(RES_AUDIO_CLIP, sizeof(struct NeAudioClip) - sizeof(uint8_t) + Au_audioClipDataSize,
		(NeResourceCreateProc)Au_CreateClip, (NeResourceLoadProc)Au_LoadClip, (NeResourceUnloadProc)Au_UnloadClip);

	return E_RegisterComponent(AUDIO_SOURCE_COMP, Au_sourceSize, 1,
		(NeCompInitProc)Au_InitSourceComponent, (NeCompTermProc)Au_TermSourceComponent);*/
/*error:
	if (Au_context)
		alcDestroyContext(Au_context);

	if (Au_device)
		alcCloseDevice(Au_device);

	return false;*/
}

void
Au_Update(void)
{
//	X3DAudioCalculate(_au3DAudio, &_auListener, )
}

void
Au_Term(void)
{
/*	alcMakeContextCurrent(NULL);

	if (Au_context)
		alcDestroyContext(Au_context);

	if (Au_device)
		alcCloseDevice(Au_device);*/
}

#ifdef __APPLE__
#	pragma clang diagnostic pop
#endif
