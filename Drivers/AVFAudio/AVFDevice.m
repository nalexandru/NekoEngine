#include <stdlib.h>

#include "AVFDriver.h"

struct AudioDevice *
AVF_CreateDevice(struct AudioDeviceInfo *info, struct AudioDeviceProcs *devProcs, struct AudioSourceProcs *srcProcs)
{
	struct AudioDevice *ad = calloc(1, sizeof(*ad));

	return ad;
}

void
AVF_DestroyDevice(struct AudioDevice *dev)
{
	free(dev);
}
