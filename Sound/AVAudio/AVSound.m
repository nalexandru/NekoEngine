#include "AVSound.h"

AVAudioEngine *_avEngine = NULL;

bool
Au_InitLib(void)
{
	_avEngine = [[AVAudioEngine alloc] init];
/*	if (!(_device = alcOpenDevice(NULL)))
		return false;
	
	if (!(_context = alcCreateContext(_device, NULL)))
		return false;
	
	if (!alcMakeContextCurrent(_context))
		return false;
	
	alListener3f(AL_POSITION, 0.f, 0.f, 0.f);
	alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f);
	alListenerfv(AL_ORIENTATION, _initOrientation);
	
	alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);*/
	
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
	/*alcMakeContextCurrent(NULL);
	
	if (_context)
		alcDestroyContext(_context);
	_context = NULL;
	
	if (_device)
		alcCloseDevice(_device);
	_device = NULL;*/
}
