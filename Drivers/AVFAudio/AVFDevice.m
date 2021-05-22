#include <stdlib.h>

#include <System/Memory.h>

#include "AVFDriver.h"

struct AudioDevice *
AVF_CreateDevice(struct AudioDeviceInfo *info, struct AudioDeviceProcs *devProcs, struct AudioSourceProcs *srcProcs)
{
	struct AudioDevice *ad = Sys_Alloc(sizeof(*ad), 1, MH_AudioDriver);

	return ad;
}

void
AVF_DestroyDevice(struct AudioDevice *dev)
{
	Sys_Free(dev);
}
