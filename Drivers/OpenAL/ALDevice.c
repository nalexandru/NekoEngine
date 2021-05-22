#include <stdlib.h>

#include <System/Memory.h>

#include "OpenALDriver.h"

struct AudioDevice *
AL_CreateDevice(struct AudioDeviceInfo *info, struct AudioDeviceProcs *devProcs, struct AudioSourceProcs *srcProcs)
{
	struct AudioDevice *ad = Sys_Alloc(1, sizeof(*ad), MH_Audio);

	return ad;
}

void
AL_DestroyDevice(struct AudioDevice *dev)
{
	Sys_Free(dev);
}
