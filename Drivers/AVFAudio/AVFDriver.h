#ifndef _AVF_AUDIO_DRIVER_H_
#define _AVF_AUDIO_DRIVER_H_

#include <Audio/Audio.h>
#include <Audio/Driver.h>
#include <Audio/Device.h>
#include <Audio/Source.h>

struct AudioDevice
{
	void *a;
};

struct AudioDevice *AVF_CreateDevice(struct AudioDeviceInfo *info, struct AudioDeviceProcs *devProcs, struct AudioSourceProcs *srcProcs);
void AVF_DestroyDevice(struct AudioDevice *dev);

#endif /* _AVF_AUDIO_DRIVER_H_ */
