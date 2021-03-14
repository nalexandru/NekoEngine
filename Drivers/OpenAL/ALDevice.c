#include <stdlib.h>

#include "OpenALDriver.h"

struct AudioDevice *
AL_CreateDevice(struct AudioDeviceInfo *info, struct AudioDeviceProcs *devProcs, struct AudioSourceProcs *srcProcs)
{
	struct AudioDevice *ad = calloc(1, sizeof(*ad));

	return ad;
}

void
AL_DestroyDevice(struct AudioDevice *dev)
{
	free(dev);
}
