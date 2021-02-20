#define Handle __EngineHandle

#include <Render/Device.h>
#include <Render/AccelerationStructure.h>

#undef Handle

#include "MTLDriver.h"

struct AccelerationStructure *
MTL_CreateAccelerationStructure(id<MTLDevice> dev, const struct AccelerationStructureCreateInfo *asci)
{
	return NULL;
}

void
MTL_DestroyAccelerationStructure(id<MTLDevice> dev, struct AccelerationStructure *buff)
{
	//
}
