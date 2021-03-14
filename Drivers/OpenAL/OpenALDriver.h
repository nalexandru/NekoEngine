#ifndef _OPEN_AL_DRIVER_H_
#define _OPEN_AL_DRIVER_H_

#include <Audio/Audio.h>
#include <Audio/Driver.h>
#include <Audio/Device.h>
#include <Audio/Source.h>

struct AudioDevice
{
	void *a;
};

struct AudioDevice *AL_CreateDevice(struct AudioDeviceInfo *info, struct AudioDeviceProcs *devProcs, struct AudioSourceProcs *srcProcs);
void AL_DestroyDevice(struct AudioDevice *dev);

#endif /* _OPEN_AL_DRIVER_H_ */